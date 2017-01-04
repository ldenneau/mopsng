/* ingestIPP
 * $Id$
 *
 * Quickly read an IPP FITS detection file and split the contents into
 * two backed binary files: a low-confidence (LC) archive file and 
 * high-confidence (HC) file.
 *
 * The low-confidence file will be written directly to the LC archive; 
 * the HC file will be written to TMP and immediately ingested into the
 * MOPS DB.
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


typedef struct {
    double ra_deg, ra_sig;
    double dec_deg, dec_sig;
    double mag, mag_sig;
    double s2n;
    double ang_deg, ang_sig;
    double len_deg, len_sig;
    long det_num;
    long proc_id;
} HC_ROW_T;

#pragma pack(push,4)
typedef struct {
    int field_id;
    int det_num;
    double epoch_mjd, ra_deg, dec_deg;
    float mag, s2n, ra_sig, dec_sig, mag_sig;
} LC_ROW_T;


typedef struct {
    double ra_deg;
    double dec_deg;
    double ra_sig;
    double dec_sig;
    float mag;
    float mag_sig;
    float starpsf;
    float ang_deg;
    float ang_sig;
    float len_deg;
    float len_sig;
    unsigned int flags;
    long proc_id;
} HOTWIRED_FITS_ROW_T;
#pragma pack(pop)


#define PI 3.14159265358979323846

// LD
#define RADPERDEG (PI / 180)
#define DEGPERRAD (180 / PI)

/* Function declarations. */
int printerror(int status);
void flux2hsn(double flux, double flux_sig, double C0, double *mag, double *mag_sig, double *s2n);
void fail_if(int cond, char *msg, int exit_code);
int get_file_info(char *infilename, HOTWIRED_FITS_ROW_T **everything, long *data_size, long *row_size, long *nrows);



/* Other globals. */
static const double LN10 = 2.30258509299404568401;
int verbose = 0;                    ///< verbose runtime output

// PS1 constants for photometric/astrometric error.  For more information 'perldoc PS::MOPS::Lib'.
static const double M1 = 0;
static const double MU = 46.2;
static const double PS = 2;
static const double C = 1.00017;
static FWHM_as = 0.6;                   /* nominal PSF full-width half-max */

enum FIELD_TYPES {
    FIELD_SHAPE_CIRCLE = 0, 
    FIELD_SHAPE_SQUARE
};


static void
usage(const char *msg)
{
    if (msg) {
        /* Display error message before usage, if specified. */
        fprintf(stderr, "%s\n\n", msg);
    }

    fprintf(stderr, "\
Usage: splitIPPP [options] INFILE\n\
\n\
  --hc_filename FILE : high-confidence output filename\n\
  --lc_filename FILE : low-confidence output filename\n\
  --hc_cutoff S2N : high-confidence S/N cutoff (usually 5)\n\
  --lc_cutoff S2N : low-confidence S/N cutoff (usually 3)\n\
  --epoch_mjd EPOCH : epoch in MJD\n\
  --filter FILTER : g/r/i/z/y/w filter\n\
  INFILE : input IPP FITS file\n\n\
");
};


static int 
get_options(
    int argc,
    char **argv,
    long *field_id_ptr,
    char **hc_filename_ptr,
    char **lc_filename_ptr,
    double *hc_cutoff_ptr,
    double *lc_cutoff_ptr,
    double *epoch_mjd_ptr,
    char **filter_ptr,
    double *C0_ptr,
    char **infilename_ptr
    ) 
{
    int c;
    int option_index = 0;

    *field_id_ptr = -1;
    *hc_filename_ptr = NULL;
    *lc_filename_ptr = NULL;
    *hc_cutoff_ptr = 0;
    *lc_cutoff_ptr = 0;
    *epoch_mjd_ptr = 0;
    *filter_ptr = 0;
    *infilename_ptr = NULL;

    while (1) {
        static struct option long_opts[] = {
            {"field_id", required_argument, NULL, 0},
            {"hc_filename", required_argument, NULL, 0},
            {"lc_filename", required_argument, NULL, 0},
            {"hc_cutoff", required_argument, NULL, 0},
            {"lc_cutoff", required_argument, NULL, 0},
            {"epoch_mjd", required_argument, NULL, 0},
            {"filter", required_argument, NULL, 0},
            { 0, 0, 0, 0},
        };
        c = getopt_long(argc, argv, "", long_opts, &option_index);
        if (c == -1)
            break;
    
        // Set some sentinel values so we can check that we got everything.
        switch (c) {
            case 0:
                if (strcmp(long_opts[option_index].name, "field_id") == 0) {
                    *field_id_ptr = atol(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "hc_filename") == 0) {
                    *hc_filename_ptr = optarg;
                }
                else if (strcmp(long_opts[option_index].name, "lc_filename") == 0) {
                    *lc_filename_ptr = optarg;
                }
                else if (strcmp(long_opts[option_index].name, "hc_cutoff") == 0) {
                    *hc_cutoff_ptr = atof(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "lc_cutoff") == 0) {
                    *lc_cutoff_ptr = atof(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "epoch_mjd") == 0) {
                    *epoch_mjd_ptr = atof(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "filter") == 0) {
                    *filter_ptr = optarg;
                }
                break;

            default:
                usage(NULL);
                exit(100);
        }
    }

    if (optind < argc) {
        *infilename_ptr = argv[optind++];
    }

    /** Check options. */
    if (!*infilename_ptr) {
        usage(NULL);
        exit(1);
    }

    if (!*hc_filename_ptr || !*lc_filename_ptr || *hc_cutoff_ptr <= 0 || *lc_cutoff_ptr <= 0) {
        usage(NULL);
        exit(1);
    }

    if (*field_id_ptr < 0) {
        usage("--field_id ID must be specified");
        exit(1);
    }

    if (!*filter_ptr || (!strchr(*filter_ptr, 'g') && !strchr(*filter_ptr, 'r') && !strchr(*filter_ptr, 'i') && !strchr(*filter_ptr, 'z') 
        && !strchr(*filter_ptr, 'y') && !strchr(*filter_ptr, 'w'))) {
        usage("--filter FILT must be one of g/r/i/z/y/w.");
        exit(1);
    }
    char f = (*filter_ptr)[0];
    if (f == 'g') {
        *C0_ptr = 24.9;
    }
    else if (f == 'r') {
        *C0_ptr = 25.1;
    }
    else if (f == 'i') {
        *C0_ptr = 25.0;
    }
    else if (f == 'z') {
        *C0_ptr = 24.6;
    }
    else if (f == 'y') {
        *C0_ptr = 23.6;
    }

    return 0;
}



int
main(int argc, char **argv)
{
    ///< Options
    char *hc_filename = NULL;
    char *lc_filename = NULL;
    double hc_cutoff;
    double lc_cutoff;
    char *infilename = NULL;
    double C0;
    double epoch_mjd;
    char *filter;
    long field_id;

    double t0 = time(NULL);

    if (get_options(    
            argc, argv, 
            &field_id,
            &hc_filename,
            &lc_filename,
            &hc_cutoff,
            &lc_cutoff,
            &epoch_mjd,
            &filter,
            &C0,
            &infilename
        )) {
        usage(NULL);
        exit(1);
    }


    /* Allocate columns.  This is all hard-wired FITSIO circumvention, with
     * extreme chumminess regarding the IPP format, which we presume to be
     * RA_DEG 1D (8 bytes)
     * DEC_DEG 1D (8 bytes)
     * RA_SIG 1D (8 bytes)
     * DEC_SIG 1D (8 bytes)
     * MAG 1D (8 bytes)
     * MAG_SIG 1D (8 bytes)
     * STARPSF 1D (8 bytes)
     * ANG 1D (8 bytes)
     * ANG_SIG 1D (8 bytes)
     * LEN 1D (8 bytes)
     * LEN_SIG 1D (8 bytes)
     * FLAGS 1J (4 bytes)
     * PROC_ID 1K (8 bytes) */

    HOTWIRED_FITS_ROW_T *everything;            // pointer to read-in data
    long data_size;
    long row_size;
    long nrows;

    get_file_info(infilename, &everything, &data_size, &row_size, &nrows);


    /* Open files. */
    int hc_file = open(hc_filename, O_WRONLY | O_CREAT | O_TRUNC,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    int lc_file = open(lc_filename, O_WRONLY | O_CREAT | O_TRUNC,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    /* Write HSD and LSD files.  Note file data is FITS format, so network ordered, thus
     * we have to byte-swap. */
    double s2n;
    int det_num = 1;

    HOTWIRED_FITS_ROW_T *p = (HOTWIRED_FITS_ROW_T *) everything;
    LC_ROW_T *lc_row_ptr = (LC_ROW_T *) everything; // in-place pointer for re-written LC data
    int num_hc = 0;

    long foo;
    int i = 0;
    int num_lc = 0;                                 // number passing basic S/N threshold
    unsigned long b64;
    for (i = 0; i < nrows; i++) {
        // Byte-swap each element of the input data.
        b64 = bswap_64(*((unsigned long *) &p->ra_deg)); p->ra_deg = *((double *) &b64);
        b64 = bswap_64(*((unsigned long *) &p->dec_deg)); p->dec_deg = *((double *) &b64);
        b64 = bswap_64(*((unsigned long *) &p->ra_sig)); p->ra_sig = *((double *) &b64);
        b64 = bswap_64(*((unsigned long *) &p->dec_sig)); p->dec_sig = *((double *) &b64);
        b64 = bswap_64(*((unsigned long *) &p->mag)); p->mag = *((double *) &b64);
        b64 = bswap_64(*((unsigned long *) &p->mag_sig)); p->mag_sig = *((double *) &b64);
        b64 = bswap_64(*((unsigned long *) &p->starpsf)); p->starpsf = *((double *) &b64);
        b64 = bswap_64(*((unsigned long *) &p->ang_deg)); p->ang_deg = *((double *) &b64);
        b64 = bswap_64(*((unsigned long *) &p->ang_sig)); p->ang_sig = *((double *) &b64);
        b64 = bswap_64(*((unsigned long *) &p->len_deg)); p->len_deg = *((double *) &b64);
        b64 = bswap_64(*((unsigned long *) &p->len_sig)); p->len_sig = *((double *) &b64);
        p->flags = bswap_32(p->flags);
        p->proc_id = bswap_64(p->proc_id);

        if (p->mag_sig > 0 && p->ra_sig > 0 && p->dec_sig > 0) {
            s2n = 1.0 / p->mag_sig;
            if (s2n > lc_cutoff) {
                LC_ROW_T lc_row;
                lc_row.field_id = field_id;
                lc_row.ra_deg = p->ra_deg;
                lc_row.dec_deg = p->dec_deg;
                lc_row.epoch_mjd = epoch_mjd;
                lc_row.mag = (float) p->mag;
                lc_row.s2n = (float) s2n;
                lc_row.ra_sig = (float) p->ra_sig;
                lc_row.dec_sig = (float) p->dec_sig;
                lc_row.mag_sig = (float) p->mag_sig;
                lc_row.det_num = det_num;

                if (s2n > hc_cutoff) {
                    HC_ROW_T hc_row;
                    hc_row.ra_deg = p->ra_deg;
                    hc_row.ra_sig = p->ra_sig;
                    hc_row.dec_deg = p->dec_deg;
                    hc_row.dec_sig = p->dec_sig;
                    hc_row.mag = p->mag;
                    hc_row.mag_sig = p->mag_sig;
                    hc_row.s2n = s2n;
                    hc_row.ang_deg = p->ang_deg;
                    hc_row.ang_sig = p->ang_sig;
                    hc_row.len_deg = p->len_deg;
                    hc_row.len_sig = p->len_sig;
                    hc_row.det_num = det_num;
                    hc_row.proc_id = p->proc_id;
                    write(hc_file, &hc_row, sizeof(HC_ROW_T));
                    num_hc++;
                }

                /* Store our LC_ROW in-place. This works because LC_ROW is smaller than
                 * the original row size.
                 */
                *lc_row_ptr++ = lc_row;
                num_lc++;
            }
        } 
        p++;        // next FITS row
        det_num++;
    }

    // Write the LSDs.
    write(lc_file, everything, sizeof(LC_ROW_T) * num_lc);

    close(hc_file);
    close(lc_file);
    return (0);
}


void flux2hsn(double flux, double flux_sig, double C0, double *mag, double *mag_sig, double *s2n) {

    if (isnan(flux) || isnan(flux_sig) || flux <= 0 || flux_sig < 0) {
        *s2n = 0;
        *mag = 0;
        *mag_sig = 0;
        return;
    }
    *mag = -2.5 * log(flux) / LN10 + C0;
    *mag_sig = abs(-2.5 * flux_sig / (flux * LN10));
    *s2n = flux / flux_sig;
} 

int printerror(int status) {
    if (status) {
        fits_report_error(stderr, status);
        exit(status);
    }
    return;
}


int hsn2flux(double mag, double s2n, double C0, double *flux_ptr, double *flux_sig_ptr) {
    *flux_ptr = pow(10, ((mag - C0) / -2.5));
    *flux_sig_ptr = *flux_ptr / s2n;
    return 0;
}


void fail_if(int cond, char *msg, int exit_code) {
    if (cond) {
        fprintf(stderr, "%s\n", msg);
        fits_report_error(stderr, cond);
        exit(exit_code);
    }
}


int get_file_info(char *infilename, HOTWIRED_FITS_ROW_T **everything_in, long *data_size_in, long *row_size_in, long *nrows_in) {

    long nrows;
    long row_size = sizeof(HOTWIRED_FITS_ROW_T);

    /* Get FITS table vitals. */
    int status = 0;
    int dummy;
    fitsfile *fp;

    fits_open_file(&fp, infilename, READONLY, &status);
    fail_if(status, "can't open FITS file", 1);

    fits_movrel_hdu(fp, 1, &dummy, &status);
    fail_if(status, "can't move to HDU", 1);

    fits_get_num_rows(fp, &nrows, &status);
    fail_if(status, "can't get nrows", 1);

    HOTWIRED_FITS_ROW_T *everything = (HOTWIRED_FITS_ROW_T *) malloc(nrows * sizeof(HOTWIRED_FITS_ROW_T));
    fits_read_tblbytes(fp, 1, 1, nrows * row_size, (unsigned char *) everything, &status);
    fail_if(status, "Can't read table bytes.", 1);

    fits_close_file(fp, &status);
    fail_if(status, "Can't close FITS file.", 1);

    *everything_in = everything;
    *data_size_in = nrows * row_size;
    *row_size_in = row_size;
    *nrows_in = nrows;
}
