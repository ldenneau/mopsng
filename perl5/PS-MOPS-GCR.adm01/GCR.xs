#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <gcf/gcf.h>

#include "const-c.inc"

MODULE = PS::MOPS::GCR		PACKAGE = PS::MOPS::GCR		

INCLUDE: const-xs.inc

double
compute_gcr(dets_aref)
    SV *dets_aref
    INIT:
        I32 numdets = 0;
        double gcr_arcsec = 42;
        int i;
        HV *det_hv;
        SV *det_href;
        SV **mjd_svp;
        SV **ra_svp;
        SV **dec_svp;
        double mjd, ra, dec;

        if ((!SvROK(dets_aref))
            || (SvTYPE(SvRV(dets_aref)) != SVt_PVAV)
            || ((numdets = av_len((AV *) SvRV(dets_aref))) < 0)) {
            XSRETURN_UNDEF;
        }
        double t[numdets];
        double c[numdets][2];
        double nt[numdets];
        double rs[numdets][2];

    CODE:
        /* Walk list of detections, stuff params, calc GCR. */
        numdets++;  // is highest index, cvt to num elements
        for (i = 0; i < numdets; i++) {
            det_href = (SV *) (*av_fetch((AV *) SvRV(dets_aref), i, 0));

            /* Check that we got an HV. */
            det_hv = (HV *) SvRV(det_href);

            /* Got an HV for the det, now look up key-vals. */
            mjd_svp = hv_fetch(det_hv, "epoch", sizeof("epoch") - 1, 0);
            ra_svp = hv_fetch(det_hv, "ra", sizeof("ra") - 1, 0);
            dec_svp = hv_fetch(det_hv, "dec", sizeof("dec") - 1, 0);
            if (mjd_svp == NULL || ra_svp == NULL || dec_svp == NULL)
                XSRETURN_UNDEF;

            /* Look up our values. */
            mjd = SvNV(*mjd_svp);
            ra = SvNV(*ra_svp);
            dec = SvNV(*dec_svp);

            t[i] = mjd;
            c[i][0] = ra * M_PI / 180;
            c[i][1] = dec * M_PI / 180;
        }

        gcfparam gcf;
        gcf.nObs = numdets;
        gcf.ntime = nt;
        gcf.rs = rs;
        gcFit(&gcf, t, c);
        RETVAL = gcRms(&gcf);
    OUTPUT:
        RETVAL

double
compute_vel(dets_aref)
    SV *dets_aref
    INIT:
        I32 numdets = 0;
        double gcr_vel = 42;
        int i;
        HV *det_hv;
        SV *det_href;
        SV **mjd_svp;
        SV **ra_svp;
        SV **dec_svp;
        double mjd, ra, dec;

        if ((!SvROK(dets_aref))
            || (SvTYPE(SvRV(dets_aref)) != SVt_PVAV)
            || ((numdets = av_len((AV *) SvRV(dets_aref))) < 0)) {
            XSRETURN_UNDEF;
        }
        double t[numdets];
        double c[numdets][2];
        double nt[numdets];
        double rs[numdets][2];

    CODE:
        /* Walk list of detections, stuff params, calc GCR. */
        numdets++;  // is highest index, cvt to num elements
        for (i = 0; i < numdets; i++) {
            det_href = (SV *) (*av_fetch((AV *) SvRV(dets_aref), i, 0));

            /* Check that we got an HV. */
            det_hv = (HV *) SvRV(det_href);

            /* Got an HV for the det, now look up key-vals. */
            mjd_svp = hv_fetch(det_hv, "epoch", sizeof("epoch") - 1, 0);
            ra_svp = hv_fetch(det_hv, "ra", sizeof("ra") - 1, 0);
            dec_svp = hv_fetch(det_hv, "dec", sizeof("dec") - 1, 0);
            if (mjd_svp == NULL || ra_svp == NULL || dec_svp == NULL)
                XSRETURN_UNDEF;

            /* Look up our values. */
            mjd = SvNV(*mjd_svp);
            ra = SvNV(*ra_svp);
            dec = SvNV(*dec_svp);

            t[i] = mjd;
            c[i][0] = ra * M_PI / 180;
            c[i][1] = dec * M_PI / 180;
        }

        gcfparam gcf;
        gcf.nObs = numdets;
        gcf.ntime = nt;
        gcf.rs = rs;
        gcFit(&gcf, t, c);
        RETVAL = gcVel(&gcf);
    OUTPUT:
        RETVAL
