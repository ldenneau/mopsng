# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl PS-MOPS-Config.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More tests => 11;
BEGIN { use_ok('PS::MOPS::Config') };

#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.

my $text = join '', <DATA>;
$parsed = PS::MOPS::Config->new_from_string($text);

ok($parsed->{ssm}, 'hash');
ok($parsed->{ssm}->{exposure_time_s}, 'hash item');
ok($parsed->{list_items}, 'list');
ok($parsed->{list_items}->[0] eq 'foo', 'list item 1');
ok($parsed->{list_items}->[1] eq 'bar', 'list item 2');
ok($parsed->{site}->{s2n_config}, 'nested hash');
ok($parsed->{site}->{s2n_config}->{g}, 'nested hash item 1');
ok($parsed->{site}->{s2n_config}->{g}->{M1}, 'nested hash item 2');
ok($parsed->{test}->{zib} eq 'foo[bar]', 'hash item containing single-quoted [');
ok($parsed->{test}->{zab} eq 'bar[foo]', 'hash item containing double-quoted [');



__DATA__
# Test config file.  Put everything we might see in here.

# Solar system model, used for synthetic data generation.
ssm {
    exposure_time_s = 30.0                  # exposure time, seconds

    add_astrometric_error = 1               # 1 => add astrometric error, 0 => no
    add_false_detections = 0                # 1 => include false detections, 0 => no
    add_shape_mag = 0                       # 1 => compute shape magnitudes for objects
}

list_items [
    foo
    bar
    'zib{1}'
    'zorb{2}'
]

findtracklets {
    maxv_degperday = 2.0                    # findtracklets max degree/day
    minobs = 2                              # mininum number detections/tracklet; usually 2; 'auto' allowed for tuplewise stacks
    extended = 1                            # use elongated tracklets
    maxt = 0.050                            # The maximum time range for a tracklet (in fractional days).
    tti_min = 30.0                          # 30-minute nominal TTI
    tuplewise = 1                           # set to 1 for tuplewise processing
}

dtctl {
    attribution_resid_threshold_arcsec = 0.30;      # attributed orbits must have resid < this
    attribution_proximity_threshold_arcsec = 600;   # 10 arcsec
    attribution_use_iod = 1;
}

lodctl {
    iod_threshold_arcsec = 3600             # IOD residual threshold, arcseconds
    diffcor_threshold_arcsec = 0.40         # differential correction residual threshold, arcseconds
    fallback_iod = 30                       # if set, use low-resid IOD if diffcor failed

    max_link_days = 15                      # size of window for moving objects
    min_nights = 3                          # minimum number of nights to attempt OD
    slow_minv_degperday = 0.0               # minv for "slow movers"
    slow_maxv_degperday = 0.5               # maxv for "slow movers"
    slow_grouped_radius_deg = 4             # radius for grouping link batch jobs

    fast_minv_degperday = 0.4               # minv for "fast movers"
    fast_maxv_degperday = 7.0               # maxv for "fast movers"
    fast_grouped_radius_deg = 8             # radius for grouping link batch jobs

    allow_multiple_attributions = 1         # allow multiple tracklet attributions
    iod_program = 'milani_iod'
    diffcor_program = 'mopsdiffcor'         # default standard MOPS diffcor
}

linktracklets {
    slow_link_opts = 'vtree_thresh 0.0004 pred_thresh 0.0008'
    fast_link_opts = 'vtree_thresh 0.0030 pred_thresh 0.0050'
}

debug {
    force_empty_attribution_list = 0        # for testing provisional attributions; should be 0
    disable_attributions = 0                # enable attributions when fields processed
    disable_precovery = 0                   # enable precovery jobs after linking night completed
    disable_orbit_identification = 0        # enable orbit id after linking
    disable_orbit_determination = 0         # enable orbit determination; otherwise just link
}

site {
# Leave blank; force manual set
    obscode = 566                           # Haleakala
    gmt_offset_hours = -10                  # 10 hours behind GMT; used for night delination
    field_size_deg2 = 7.0

# A/P
    limiting_mag = 24
    fwhm_arcseconds = 0.7                   # PSF FWHM; calc false/deg2 from this (see insertFalseDetections)
    astrometric_error_arcseconds = 0.01     # arcseconds RMS
    s2n = 5.0                               # SSM high-confidence S/N
#    low_confidence_s2n = 3.0                # PS low-confidence S/N

## deprecated
##    false_detection_rate_per_deg2 = 200     # 200 false dets/deg2
##    false_detection_mag = 23.5              # mag for false detections
#    false_detection_s2n = 5.0              # false detection signal-to-noise (5 or 3 usually); default use ssm->{s2n}

    # Comprehensive A/P config to control synthetic detection generation in different filters.
    s2n_config {
        g {
            M1 = 24.90
            MU = 21.90
            M1plusMU = 46.8
            exposure_time_sec = 60
            limiting_mag = 23.2
        }
        r {
            M1 = 25.15
            MU = 20.86
            M1plusMU = 46.01
            exposure_time_sec = 38
            limiting_mag = 22.7
        }
        i {
            M1 = 25.00
            MU = 20.15
            M1plusMU = 45.15
            exposure_time_sec = 30
            limiting_mag = 22.6
        }
        z {
            M1 = 24.63
            MU = 19.26
            M1plusMU = 43.89
            exposure_time_sec = 30
            limiting_mag = 21.6
        }
        y {
            M1 = 23.02
            MU = 17.98
            M1plusMU = 41.00
            exposure_time_sec = 30
            limiting_mag = 20.1
        }
    }
}

test {
    zib = 'foo[bar]'
    zab = "bar[foo]"
}
