//#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>

#include "fitsio.h"
#include "slalib.h"


#define PI 3.14159265358979323846

// LD
#define RADPERDEG (PI / 180)
#define DEGPERRAD (180 / PI)

/* Function declarations. */
int printerror(int status);
int false_detection_mag(double hc_s2n, double hc_limiting_mag, double false_s2n, double *mag_ptr, double *s2n_ptr);
double mopslib_photoS2N(double M, double exptime_s);
double astro_error(double M, double S2N);
int in_field(double fc_ra_rad, double fc_dec_rad, double fov_rad, double pt_ra_rad, double pt_dec_rad);
int hsn2flux(double mag, double s2n, double C0, double *flux_ptr, double *flux_sig_ptr);



/* Other globals. */
double LOG10 = 2.30258509299405;    ///< natural log of 10
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


void
usage(const char *msg)
{
    if (msg) {
        /* Display error message before usage, if specified. */
        fprintf(stderr, "%s\n\n", msg);
    }

    fprintf(stderr, "\
Usage: createFalse [options] OUTFILE\n\
\n\
  --ra_deg RA : right ascension in degrees\n\
  --dec_deg DEC : declination in degrees\n\
  --epoch_mjd EPOCH : epoch in MJD\n\
  --filter FILTER : g/r/i/z/y filter\n\
  --limiting_mag MAG : limiting magnitude\n\
  --num_dets NUM : number of detections to attempt\n\
  --field_size_deg2 SIZE : area of field in square degrees\n\
  --field_shape circe|square : shape of field\n\
  --astro_error_baseline_arcsec ERROR_ARCSEC : baseline astrometric error\n\
  --hc_s2n S2N : high-confidence S/N (usually 5.0)\n\
  --false_s2n S2N : low-confidence S/N (between 3.0 and 5.0)\n\
  OUTFILE : output file\n\n\
");
};


static int 
get_options(
    int argc,
    char **argv,
    double *ra_deg_ptr,
    double *dec_deg_ptr,
    double *epoch_mjd_ptr,
    char **filter_ptr,
    double *C0_ptr,
    double *limiting_mag_ptr,
    int *num_dets_ptr,
    double *field_size_deg2_ptr,
    char **field_shape_ptr,
    int *field_shape_en_ptr,
    double *astro_error_baseline_arcsec_ptr,
    double *hc_s2n_ptr,
    double *false_s2n_ptr,
    char **outfilename_ptr
    ) 
{
    int c;
    int option_index = 0;

    *astro_error_baseline_arcsec_ptr = -1;
    *hc_s2n_ptr = -1;
    *false_s2n_ptr = -1;
    *outfilename_ptr = NULL;

    while (1) {
        static struct option long_opts[] = {
            {"ra_deg", required_argument, NULL, 0},
            {"dec_deg", required_argument, NULL, 0},
            {"epoch_mjd", required_argument, NULL, 0},
            {"filter", required_argument, NULL, 0},
            {"limiting_mag", required_argument, NULL, 0},
            {"num_dets", required_argument, NULL, 0},
            {"field_size_deg2", required_argument, NULL, 0},
            {"field_shape", required_argument, NULL, 0},
            {"astro_error_baseline_arcsec", required_argument, NULL, 0},
            {"hc_s2n", required_argument, NULL, 0},
            {"false_s2n", required_argument, NULL, 0},
            { 0, 0, 0, 0},
        };
        c = getopt_long(argc, argv, "", long_opts, &option_index);
        if (c == -1)
            break;
    
        // Set some sentinel values so we can check that we got everything.
        switch (c) {
            case 0:
                if (strcmp(long_opts[option_index].name, "ra_deg") == 0) {
                    *ra_deg_ptr = atof(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "dec_deg") == 0) {
                    *dec_deg_ptr = atof(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "epoch_mjd") == 0) {
                    *epoch_mjd_ptr = atof(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "filter") == 0) {
                    *filter_ptr = optarg;
                }
                else if (strcmp(long_opts[option_index].name, "limiting_mag") == 0) {
                    *limiting_mag_ptr = atof(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "num_dets") == 0) {
                    *num_dets_ptr = atoi(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "field_size_deg2") == 0) {
                    *field_size_deg2_ptr = atof(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "field_shape") == 0) {
                    *field_shape_ptr = optarg;
                }
                else if (strcmp(long_opts[option_index].name, "astro_error_baseline_arcsec") == 0) {
                    *astro_error_baseline_arcsec_ptr = atof(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "hc_s2n") == 0) {
                    *hc_s2n_ptr = atof(optarg);
                }
                else if (strcmp(long_opts[option_index].name, "false_s2n") == 0) {
                    *false_s2n_ptr = atof(optarg);
                }
                break;

            default:
                usage(NULL);
                exit(100);
        }
    }

    if (optind < argc) {
        *outfilename_ptr = argv[optind++];
    }

    /** Check options. */
    if (!*outfilename_ptr) {
        usage(NULL);
        exit(1);
    }

    if (*ra_deg_ptr < 0 || *ra_deg_ptr > 360) {
        usage("--ra_deg RA must be between 0 and 360.");
        exit(1);
    }

    if (*dec_deg_ptr < -90 || *dec_deg_ptr > 90) {
        usage("--dec_deg DEC must be between -90 and +90.");
        exit(1);
    }

    if (!*filter_ptr || (!strchr(*filter_ptr, 'g') && !strchr(*filter_ptr, 'r') && !strchr(*filter_ptr, 'i') && !strchr(*filter_ptr, 'z') && !strchr(*filter_ptr, 'y'))) {
        usage("--filter FILT must be one of g/r/i/z/y.");
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

    if (*limiting_mag_ptr < 0 || *limiting_mag_ptr > 30) {
        usage("--limiting_mag MAG must be between 0 and 30.");
        exit(1);
    }

    if (*num_dets_ptr < 0 || *num_dets_ptr > 10000000) {  /* ten million */
        usage("--num_dets NUM must be between 0 and 10000000 (ten million).");
        exit(1);
    }

    if (*field_size_deg2_ptr < 0 || *field_size_deg2_ptr > 20) {
        usage("--field_size_deg2 SIZE must be between 0 and 20.");
        exit(1);
    }

    if (!*field_shape_ptr || (!strcmp(*field_shape_ptr, "circle") && !strcmp(*field_shape_ptr, "square"))) {
        usage("--field_shape SHAPE must be either 'circle' or 'square'.");
        exit(1);
    }
    if (0 == strcmp(*field_shape_ptr, "circle")) {
        *field_shape_en_ptr = FIELD_SHAPE_CIRCLE;
    }
    else if (0 == strcmp(*field_shape_ptr, "square")) {
        *field_shape_en_ptr = FIELD_SHAPE_SQUARE;
    }

    if (*astro_error_baseline_arcsec_ptr < 0) {
        usage("--astro_error_baseline_arcsec ERROR_ARCSEC must be specified.");
        exit(1);
    }

    if (*hc_s2n_ptr < 0) {
        usage("--hc_s2n S2N must be specified.");
        exit(1);
    }

    if (*false_s2n_ptr < 0) {
        usage("--false_s2n S2N must be specified.");
        exit(1);
    }

    return 0;
}



int
main(int argc, char **argv)
{
    ///< Options
    double ra_deg;
    double dec_deg;
    double epoch_mjd;
    char *filter = NULL;
    double C0;
    double limiting_mag;
    int num_dets;
    double field_size_deg2;
    char *field_shape = NULL;
    double astro_error_baseline_arcsec;
    int field_shape_en;
    char *outfilename = NULL;
    double hc_s2n;
    double false_s2n;

    double t0 = time(NULL);

    if (get_options(    
            argc, argv, 
            &ra_deg,
            &dec_deg,
            &epoch_mjd,
            &filter,
            &C0,
            &limiting_mag,
            &num_dets,
            &field_size_deg2,
            &field_shape,
            &field_shape_en,
            &astro_error_baseline_arcsec,
            &hc_s2n,
            &false_s2n,
            &outfilename
        )) {
        usage(NULL);
        exit(1);
    }


    /** Do interesting stuff. */
    fitsfile *fptr;
    int fits_status, hdutype;
    long firstrow, firstelem;

    char *ttype[] = { "RA_DEG", "DEC_DEG", "RA_SIG", "DEC_SIG", "MAG", "MAG_SIG", "FLAGS", "STARPSF", "ANG", "ANG_SIG", "LEN", "LEN_SIG", "PROC_ID"};
    char *tform[] = { "1D1", "1D1", "1D1", "1D1", "1D1", "1D1", "1J", "1D1", "1D1", "1D1", "1D1", "1D1", "1K1"};
    char *tunit[] = { "deg", "deg", "deg", "deg", "\0", "\0", "\0", "\0", "deg", "deg", "deg", "deg", "\0"};

    fits_status = 0;
    if (fits_open_file(&fptr, outfilename, READWRITE, &fits_status)) {
        printerror(fits_status);
    }

    if (fits_movabs_hdu(fptr, 2, &hdutype, &fits_status)) {
        printerror(fits_status);
    }

    /* Allocate columns. */
    double *ra_deg_col = (double *) malloc(num_dets * sizeof(double));
    double *ra_deg_ptr = ra_deg_col;

    double *ra_sig_col = (double *) malloc(num_dets * sizeof(double));
    double *ra_sig_ptr = ra_sig_col;

    double *dec_deg_col = (double *) malloc(num_dets * sizeof(double));
    double *dec_deg_ptr = dec_deg_col;

    double *dec_sig_col = (double *) malloc(num_dets * sizeof(double));
    double *dec_sig_ptr = dec_sig_col;

    double *mag_col = (double *) malloc(num_dets * sizeof(double));
    double *mag_ptr = mag_col;

    double *mag_sig_col = (double *) malloc(num_dets * sizeof(double));
    double *mag_sig_ptr = mag_sig_col;

    unsigned int *flags_col = (unsigned int *) malloc(num_dets * sizeof(unsigned int));
    unsigned int *flags_ptr = flags_col;

    double *starpsf_col = (double *) malloc(num_dets * sizeof(double));
    double *starpsf_ptr = starpsf_col;

    double *ang_col = (double *) malloc(num_dets * sizeof(double));
    double *ang_ptr = ang_col;

    double *ang_sig_col = (double *) malloc(num_dets * sizeof(double));
    double *ang_sig_ptr = ang_sig_col;

    double *len_col = (double *) malloc(num_dets * sizeof(double));
    double *len_ptr = len_col;

    double *len_sig_col = (double *) malloc(num_dets * sizeof(double));
    double *len_sig_ptr = len_sig_col;

    long *dummy_proc_id_col = (long *) calloc(num_dets, sizeof(long));


    /* Create detections. */
    srand(time(NULL) + getpid());
    int i;
    int num_rows = 0;
    double field_ra_rad, field_dec_rad;
    double det_ra_deg, det_ra_sig;
    double det_dec_deg, det_dec_sig;
    double det_mag, det_mag_sig;
    double det_ang, det_ang_sig;
    double det_len, det_len_sig;

    double tp_size_rad;
    double tp_x_rad, tp_y_rad;
    double sph_ra_rad, sph_dec_rad;
    double s2n;

    if (FIELD_SHAPE_CIRCLE == field_shape_en) {
        tp_size_rad = 2 * sqrt(field_size_deg2 / PI) * RADPERDEG;
    }
    else {
        tp_size_rad = sqrt(field_size_deg2) * RADPERDEG;
    }
    field_ra_rad = ra_deg * RADPERDEG;
    field_dec_rad = dec_deg * RADPERDEG;

    for (i = 0; i < num_dets; i++) {
        tp_x_rad = ((rand() / (RAND_MAX + 1.0)) - 0.5) * tp_size_rad;
        tp_y_rad = ((rand() / (RAND_MAX + 1.0)) - 0.5) * tp_size_rad;
        slaDtp2s(
            tp_x_rad, tp_y_rad,           // random tangent-plane positions
            field_ra_rad, field_dec_rad,  // field center
            &sph_ra_rad, &sph_dec_rad     // resultant field RA/DEC of detection
        );

        if (in_field(field_ra_rad, field_dec_rad, tp_size_rad, sph_ra_rad, sph_dec_rad)) {        
            det_ra_deg = sph_ra_rad * DEGPERRAD;
            det_dec_deg = sph_dec_rad * DEGPERRAD;
            false_detection_mag(hc_s2n, limiting_mag, false_s2n, &det_mag, &s2n);
            det_ra_sig = (astro_error_baseline_arcsec + astro_error(det_mag, s2n)) / 3600;
            det_dec_sig = det_ra_sig;


            // Store the detection.
            *ra_deg_ptr++ = det_ra_deg;
            *ra_sig_ptr++ = det_ra_sig;
            *dec_deg_ptr++ = det_dec_deg;
            *dec_sig_ptr++ = det_dec_sig;
//            hsn2flux(mag, s2n, C0, &det_flux, &det_flux_sig);
            *mag_ptr++ = det_mag;
            *mag_sig_ptr++ = 1.0 / s2n;
            *flags_ptr++ = 0;
            *starpsf_ptr++ = 0;
            *ang_ptr++ = 0;
            *ang_sig_ptr++ = 0;
            *len_ptr++ = 0;
            *len_sig_ptr++ = 0;

            num_rows++;
        }
    }
    //fprintf(stderr, "done (%f sec).\nWriting %d detections to FITS file %s...", num_rows, time(NULL) - t0, outfilename);

    if (fits_write_col(fptr, TDOUBLE, 1 /*col*/, 1 /*row*/, 1 /*elem*/, num_rows, ra_deg_col, &fits_status)) {
        printerror(fits_status);
    }

    if (fits_write_col(fptr, TDOUBLE, 2 /*col*/, 1 /*row*/, 1 /*elem*/, num_rows, ra_sig_col, &fits_status)) {
        printerror(fits_status);
    }

    if (fits_write_col(fptr, TDOUBLE, 3 /*col*/, 1 /*row*/, 1 /*elem*/, num_rows, dec_deg_col, &fits_status)) {
        printerror(fits_status);
    }

    if (fits_write_col(fptr, TDOUBLE, 4 /*col*/, 1 /*row*/, 1 /*elem*/, num_rows, dec_sig_col, &fits_status)) {
        printerror(fits_status);
    }

    if (fits_write_col(fptr, TDOUBLE, 5 /*col*/, 1 /*row*/, 1 /*elem*/, num_rows, mag_col, &fits_status)) {
        printerror(fits_status);
    }

    if (fits_write_col(fptr, TDOUBLE, 6 /*col*/, 1 /*row*/, 1 /*elem*/, num_rows, mag_sig_col, &fits_status)) {
        printerror(fits_status);
    }

    if (fits_write_col(fptr, TUINT, 7 /*col*/, 1 /*row*/, 1 /*elem*/, num_rows, flags_col, &fits_status)) {
        printerror(fits_status);
    }

    if (fits_write_col(fptr, TDOUBLE, 8 /*col*/, 1 /*row*/, 1 /*elem*/, num_rows, starpsf_col, &fits_status)) {
        printerror(fits_status);
    }

    if (fits_write_col(fptr, TDOUBLE, 9 /*col*/, 1 /*row*/, 1 /*elem*/, num_rows, ang_col, &fits_status)) {
        printerror(fits_status);
    }

    if (fits_write_col(fptr, TDOUBLE, 10 /*col*/, 1 /*row*/, 1 /*elem*/, num_rows, ang_sig_col, &fits_status)) {
        printerror(fits_status);
    }

    if (fits_write_col(fptr, TDOUBLE, 11 /*col*/, 1 /*row*/, 1 /*elem*/, num_rows, len_col, &fits_status)) {
        printerror(fits_status);
    }

    if (fits_write_col(fptr, TDOUBLE, 12 /*col*/, 1 /*row*/, 1 /*elem*/, num_rows, len_sig_col, &fits_status)) {
        printerror(fits_status);
    }

    if (fits_write_col(fptr, TLONGLONG, 13 /*col*/, 1 /*row*/, 1 /*elem*/, num_rows, dummy_proc_id_col, &fits_status)) {
        printerror(fits_status);
    }

    if (fits_close_file(fptr, &fits_status)) {
        printerror(fits_status);
    }
    //fprintf(stderr, "done (%f sec).\n", time(NULL) - t0);

    return (0);
}


int printerror(int status) {
    if (status) {
        fits_report_error(stderr, status);
        exit(status);
    }
    return;
}


int false_detection_mag(double hc_s2n, double hc_limiting_mag, double false_s2n, double *mag_ptr, double *s2n_ptr) {
    double p, SN, V;

    p = rand() / (RAND_MAX + 1.0);
    SN = sqrt(false_s2n * false_s2n - 2 * log(1 - p));
    if (SN <= 0) return 99.1;
    *mag_ptr = hc_limiting_mag - 2.5 * log(SN / hc_s2n) / LOG10;
    *s2n_ptr = SN;
    return 0;
}


double mopslib_photoS2N(double M, double exptime_s) {
/* Returns photometric signal-to-noise as a function of apparent magnitude
and exposure time.*/
    double S2N = PS * C * pow(10, -0.2 * (2 * M - MU - M1))
        * sqrt(exptime_s / PI / (FWHM_as * FWHM_as));
    return S2N;
}

double astro_error(double M, double S2N) {
    /* Returns astrometric error in arcseconds as a function of apparent magnitude. */
    if (S2N <= 1.5) {
        return 1.5;
    }

    double astroError = 0.070 * (FWHM_as / 0.6) * (5.0 / S2N);
    return astroError;
}


int in_field(double fc_ra_rad, double fc_dec_rad, double fov_rad, double pt_ra_rad, double pt_dec_rad) {
    // fc == 'field center'
    // fov = 'field of view' (diameter of field)
    // pt = point to test
    return slaDsep(fc_ra_rad, fc_dec_rad, pt_ra_rad, pt_dec_rad) < fov_rad / 2;
}


int hsn2flux(double mag, double s2n, double C0, double *flux_ptr, double *flux_sig_ptr) {
    *flux_ptr = pow(10, ((mag - C0) / -2.5));
    *flux_sig_ptr = *flux_ptr / s2n;
    return 0;
}

