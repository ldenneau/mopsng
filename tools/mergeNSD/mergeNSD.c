/* mergeNSD
 * $Id$
 *
 * Given a raw packed nonsythetic detection (NSD) file, sort its contents 
 * by RA and write a new packed file and index.
 * */

//#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <byteswap.h>

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>

#include "fitsio.h"
#include "slalib.h"


// require packing for maximum storage efficiency
#pragma pack(push,4)
typedef struct {
    int field_id;
    int det_num;
    double epoch_mjd, ra_deg, dec_deg;
    float mag, s2n, ra_sig, dec_sig, mag_sig;
} LSD_T;
#pragma pack(pop)


typedef struct {
    long alloc_size;        // size of buffer
    long occupancy;         // num dets in buffer
    double dec_min_deg;     // dec min for bin
    double dec_max_deg;     // dec max for bin
    LSD_T *data;            // ptr to buffer
} BIN_T;


// LD
#define RADPERDEG (M_PI / 180)
#define DEGPERRAD (180 / M_PI)

/* Function declarations. */
int printerror(int status);
void fail_if(int cond, char *msg, int exit_code);
LSD_T *read_lsds(const char *filename, long *num_lsds);
BIN_T *create_bins(int num_bins, int initial_bin_size);
void add_to_bin(BIN_T *bins, LSD_T *lsd_ptr, int num_slices);
int cmp_ra(const void *lsd1, const void *lsd2);
void sort_bins(BIN_T *bin, int num_bins);
void write_archive(const char *filename, BIN_T *bins, int num_bins);
void write_preindex(const char *filename, long field_id, double field_ra_deg, double field_dec_deg, BIN_T *bins, int num_bins, size_t index_file_offset);



/* Other globals. */
static const double LN10 = 2.30258509299404568401;
int verbose = 0;                    ///< verbose runtime output


static void
usage(const char *msg)
{
    if (msg) {
        /* Display error message before usage, if specified. */
        fprintf(stderr, "%s\n\n", msg);
    }

    fprintf(stderr, "\
Usage: mergeNSD [options] INFILE ARCHIVEFILE PREINDEXFILE\n\
\n\
  --num_slices NUM : number of RA slices\n\
  --field_id ID : MOPS field ID\n\
  --field_ra_deg RA : field RA in degrees\n\
  --field_dec_deg DEC : field declination in degrees\n\
  --index_file_offset OFFSET : offset in bytes of this field in single-file index\n\
  INFILE : input packed binary NSD filename\n\
  ARCHIVEFILE : output NSD archive filename\n\
  PREINDEXFILE : output NSD pre-index filename\n\n\
");
};


static int 
get_options(
    int argc,
    char **argv,
    int *num_slices_ptr,
    long *field_id_ptr,
    double *field_ra_deg_ptr,
    double *field_dec_deg_ptr,
    long *index_file_offset_ptr,
    char **infilename_ptr,
    char **archivefilename_ptr,
    char **preindexfilename_ptr
    ) 
{
    int c;
    int option_index = 0;

    *num_slices_ptr = -1;
    *field_id_ptr = -1;
    *field_ra_deg_ptr = -9999;
    *field_dec_deg_ptr = -9999;
    *index_file_offset_ptr = -1;
    *infilename_ptr = NULL;
    *archivefilename_ptr = NULL;
    *preindexfilename_ptr = NULL;

    while (1) {
        static struct option long_opts[] = {
            {"num_slices", required_argument, NULL, 0},
            {"field_id", required_argument, NULL, 0},
            {"field_ra_deg", required_argument, NULL, 0},
            {"field_dec_deg", required_argument, NULL, 0},
            {"index_file_offset", required_argument, NULL, 0},
            { 0, 0, 0, 0},
        };
        c = getopt_long(argc, argv, "", long_opts, &option_index);
        if (c == -1)
            break;
    
        // Set some sentinel values so we can check that we got everything.
        switch (c) {
            case 0:
                if (strcmp(long_opts[option_index].name, "num_slices") == 0) {
                    *num_slices_ptr = atoi(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "field_id") == 0) {
                    *field_id_ptr = atol(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "field_ra_deg") == 0) {
                    *field_ra_deg_ptr = atof(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "field_dec_deg") == 0) {
                    *field_dec_deg_ptr = atof(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "index_file_offset") == 0) {
                    *index_file_offset_ptr = atol(optarg);
                }
                break;

            default:
                usage(NULL);
                exit(100);
        }
    }

    if (optind < argc) { *infilename_ptr = argv[optind++]; }
    if (optind < argc) { *archivefilename_ptr = argv[optind++]; }
    if (optind < argc) { *preindexfilename_ptr = argv[optind++]; }

    /** Check options. */
    if (!*infilename_ptr) { usage("Input file was not specified."); exit(1); }
    if (!*archivefilename_ptr) { usage("Archive file was not specified."); exit(1); }
    if (!*preindexfilename_ptr) { usage("Pre-index file was not specified."); exit(1); }
    if (*num_slices_ptr <= 0) { usage("--num_slices was not specified."); exit(1); }
    if (*field_id_ptr < 0) { usage("--field_id was not specified."); exit(1); }
    if (*field_ra_deg_ptr <= -1000) { usage("--field_ra_deg was not specified."); exit(1); }
    if (*field_dec_deg_ptr <= -1000) { usage("--field_dec_deg was not specified."); exit(1); }
    if (*index_file_offset_ptr < 0) { usage("--index_file_offset was not specified."); exit(1); }

    return 0;
}



int
main(int argc, char **argv)
{
    ///< Options
    int num_slices;
    char *infilename = NULL;
    char *archivefilename = NULL;
    char *preindexfilename = NULL;
    long field_id;
    double field_ra_deg;
    double field_dec_deg;
    long index_file_offset;

    long num_lsds;
    double t0 = time(NULL);

    if (get_options(    
            argc, argv, 
            &num_slices,
            &field_id,
            &field_ra_deg,
            &field_dec_deg,
            &index_file_offset,
            &infilename,
            &archivefilename,
            &preindexfilename
        )) {
        usage(NULL);
        exit(1);
    }

    LSD_T *lsds = read_lsds(infilename, &num_lsds);
//    fprintf(stderr, "Read LSDs (%ld).\n", num_lsds);

    BIN_T *bins = create_bins(num_slices, 1024);

    long i = 0;
    LSD_T *lsd_ptr = lsds;
    while (i < num_lsds) {
        add_to_bin(bins, lsd_ptr, num_slices);
        i++, lsd_ptr++;
    }
//    fprintf(stderr, "Done binning.\n");

    // Sort the bins.
    sort_bins(bins, num_slices);
//    fprintf(stderr, "Done sorting.\n");

    // Write the LSD archive.
    write_archive(archivefilename, bins, num_slices);
    write_preindex(preindexfilename, field_id, field_ra_deg, field_dec_deg, bins, num_slices, index_file_offset);

//    fprintf(stderr, "Wrote %d HC detections.\n", num_hc);

//    close(hc_file);
//    close(lc_file);
    return (0);
}


int printerror(int status) {
    if (status) {
        fits_report_error(stderr, status);
        exit(status);
    }
    return;
}


void fail_if(int cond, char *msg, int exit_code) {
    if (cond) {
        fprintf(stderr, "%s\n", msg);
        exit(exit_code);
    }
}


LSD_T *read_lsds(const char *filename, long *num_lsds) {
    int rc;
    struct stat st;
    rc = stat(filename, &st);                           // get file size (and other info)
    fail_if(rc < 0, "could not stat input file", 1);

    int fd = open(filename, O_RDONLY);                  // open for reading
    fail_if(fd < 0, "could not open input file", 1);

    size_t infile_size = st.st_size;                      // how much to read
    fail_if(infile_size % sizeof(LSD_T) != 0, "Data size is not a multiple of LSD size", 1);

    LSD_T *lsds = (LSD_T *) malloc(infile_size * sizeof(LSD_T));
    fail_if(!lsds, "malloc of LSDs failed", 1);

    long num_read = read(fd, lsds, infile_size);
    fail_if(num_read != infile_size, "short read", 1);

    *num_lsds = infile_size / sizeof(LSD_T);            // save number of LSDs we read

    close(fd);
    return lsds;
}

BIN_T *create_bins(int num_bins, int initial_bin_size) {
    BIN_T *bins = (BIN_T *) malloc(num_bins * sizeof(BIN_T));
    fail_if(!bins, "malloc of bins failed", 1);

    // Create bins.
    int i;
    for (i = 0; i < num_bins; i++) {
        bins[i].data = (LSD_T *) malloc(initial_bin_size * sizeof(LSD_T));      // alloc space for LSDs
        bins[i].alloc_size = initial_bin_size;                                  // how many we can accomodate
        bins[i].occupancy = 0;                                                  // current usage
        bins[i].dec_max_deg = -9999;
        bins[i].dec_min_deg = 9999;
    }

    return bins;
}

void add_to_bin(BIN_T *bins, LSD_T *lsd_ptr, int num_slices) {
    // Add the LSD to the bin.  If we would exceed the bin's capacity, 
    // extend first.
    int bin_num;
    BIN_T *bin;

    // Determine which bin LSD belongs in based on RA.
    double ra_deg = lsd_ptr->ra_deg;
    double dec_deg = lsd_ptr->dec_deg;

    if (ra_deg < 0 || ra_deg > 360) {
        // Normalize RA into [0, 360)
        ra_deg = DEGPERRAD * slaDranrm(ra_deg * RADPERDEG);
        lsd_ptr->ra_deg = ra_deg;
    }
    bin_num = (int) (num_slices * ra_deg / 360.0);
    if (bin_num >= num_slices) {
        bin_num = 0;
    }
    bin = &bins[bin_num];

    if (bin->occupancy == bin->alloc_size) {
        // Extend bin if it's not big enough.
        size_t new_size = bin->alloc_size * 2;
        bin->data = (LSD_T *) realloc(bin->data, new_size * sizeof(LSD_T));
        bin->alloc_size = new_size;
        fail_if(!bin->data, "could not realloc bin", 1);
    }

    // Update bin information.
    bin->data[bin->occupancy++] = *lsd_ptr;  // copy LSD
    if (dec_deg < bin->dec_min_deg) {
        bin->dec_min_deg = dec_deg;
    }
    if (dec_deg > bin->dec_max_deg) {
        bin->dec_max_deg = dec_deg;
    }
}


int cmp_ra(const void *lsd1, const void *lsd2) {
    if (((LSD_T *) lsd1)->ra_deg < ((LSD_T *) lsd2)->ra_deg) {
        return -1;
    }
    else if (((LSD_T *) lsd1)->ra_deg > ((LSD_T *) lsd2)->ra_deg) {
        return 1;
    }
    else {
        return 0;
    }
}


void sort_bins(BIN_T *bins, int num_bins) {
    // Quicksort the detections in the specified bin.
    int i;
    for (i = 0; i < num_bins; i++) {
        if (bins[i].occupancy > 0) {
            qsort(bins[i].data, bins[i].occupancy, sizeof(LSD_T), cmp_ra);
        }
    }
}


void write_archive(const char *filename, BIN_T *bins, int num_bins) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    fail_if(fd < 0, "could not create output file", 1);

    int num_written;
    int i;
    for (i = 0; i < num_bins; i++) {
        if (bins[i].occupancy > 0) {
            num_written = write(fd, bins[i].data, bins[i].occupancy * sizeof(LSD_T));
            fail_if(num_written != bins[i].occupancy * sizeof(LSD_T), "short write", 1);
        }
    }
    close(fd);
}


void write_preindex(const char *filename, long field_id, double field_ra_deg, double field_dec_deg, BIN_T *bins, int num_bins, size_t index_file_offset) {
    /* Write the ASCII index file. */
    FILE *fp = fopen(filename, "at");       // append + text mode
    fail_if(NULL == fp, "could not create index file", 1);

    double slice_ra_min_deg;
    double slice_ra_max_deg;
    size_t offset;
    int i;
    size_t slice_size;

    for (i = 0; i < num_bins; i++) {
        if (bins[i].occupancy > 0) {
            slice_ra_min_deg = (i * 360.0) / num_bins;
            slice_ra_max_deg = ((i + 1) * 360.0) / num_bins;
            slice_size = bins[i].occupancy;                         // size of slice in LSD_T units

            fprintf(fp, "%ld %f %f %f %f %f %f %ld\n",
                field_id,
                field_ra_deg, slice_ra_min_deg, slice_ra_max_deg,
                field_dec_deg, bins[i].dec_min_deg, bins[i].dec_max_deg,
                slice_size
            );

        }
    }
    fclose(fp);
}
