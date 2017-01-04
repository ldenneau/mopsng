#!/usr/bin/env perl

use Test::More tests => 6;
use strict;
use warnings;
use FileHandle;
use File::Temp;


# Some arithmetic.
my $EXPOSURE_TIME_SEC = 30;
my $EXPOSURE_TIME_DAY = 30 / 86400;                         # exposure time in days
my $deg_per_exp;
my ($mjd1, $mjd2);
my ($ra1, $ra2);
my ($dec1, $dec2);

# Other stuff.
my $TEST_DIR = File::Temp::tempdir('/tmp/testXXXXXX', CLEANUP => 1);
#my $FINDTRACKLETS = '../findtracklets';
my $FINDTRACKLETS = '../findTracklets';



sub run_test {
    # Given the input detections string, run findtracklets on them and return 1 if any
    # tracklets were found, zero otherwise.
    my ($inp) = @_;

    my $dets_filename = "$TEST_DIR/dets";
    my $pairs_filename = "$TEST_DIR/pairs";
    my $sum_filename = "$TEST_DIR/sum";
    my $dets_fh = new FileHandle ">$dets_filename";
    print $dets_fh $inp;
    $dets_fh->close;

    my $cmd = "$FINDTRACKLETS file $dets_filename pairfile $pairs_filename summaryfile $sum_filename maxv 2.0 minobs 2 maxt 0.050 etime 30.0 &> /dev/null";
    (system($cmd) == 0) or die "system call failed: $?";
    return (-s $pairs_filename > 0);        # check size of pairs file
}


# 0.5 degrees/day
$deg_per_exp = 0.5 * $EXPOSURE_TIME_DAY;
$mjd1 = 54111.0000000;
$mjd2 = $mjd1 + $EXPOSURE_TIME_DAY;
$ra1 = 15.0000000;
$ra2 = $ra1 + $deg_per_exp;
my $DETS_0 = <<"EOF";
1 $mjd1 $ra1 0.0 21.5 566 S130E3S
2 $mjd2 $ra2 0.0 21.5 566 S130E3S
EOF

# 1.1 degrees/day
$deg_per_exp = 1.1 * $EXPOSURE_TIME_DAY;
$mjd1 = 54111.0000000;
$mjd2 = $mjd1 + $EXPOSURE_TIME_DAY;
$ra1 = 15.0000000;
$ra2 = $ra1 + $deg_per_exp;
my $DETS_1 = <<"EOF";
1 $mjd1 $ra1 0.0 21.5 566 S130E3S
2 $mjd2 $ra2 0.0 21.5 566 S130E3S
EOF

# 3.0 degrees/day
$deg_per_exp = 3.0 * $EXPOSURE_TIME_DAY;
$mjd1 = 54111.0000000;
$mjd2 = $mjd1 + $EXPOSURE_TIME_DAY;
$ra1 = 15.0000000;
$ra2 = $ra1 + $deg_per_exp;
my $DETS_2 = <<"EOF";
1 $mjd1 $ra1 0.0 21.5 566 S130E3S
2 $mjd2 $ra2 0.0 21.5 566 S130E3S
EOF


# 0.5 degrees/day, dec
$deg_per_exp = 0.5 * $EXPOSURE_TIME_DAY;
$mjd1 = 54111.0000000;
$mjd2 = $mjd1 + $EXPOSURE_TIME_DAY;
$dec1 = 0.0;
$dec2 = $dec1 + $deg_per_exp;
my $DETS_3 = <<"EOF";
1 $mjd1 15.0 $dec1 21.5 566 S130E3S
2 $mjd2 15.0 $dec2 21.5 566 S130E3S
EOF

# 1.1 degrees/day, dec
$deg_per_exp = 1.1 * $EXPOSURE_TIME_DAY;
$mjd1 = 54111.0000000;
$mjd2 = $mjd1 + $EXPOSURE_TIME_DAY;
$dec1 = 0.0;
$dec2 = $dec1 + $deg_per_exp;
my $DETS_4 = <<"EOF";
1 $mjd1 15.0 $dec1 21.5 566 S130E3S
2 $mjd2 15.0 $dec2 21.5 566 S130E3S
EOF

# 3.0 degrees/day, dec
$deg_per_exp = 3.0 * $EXPOSURE_TIME_DAY;
$mjd1 = 54111.0000000;
$mjd2 = $mjd1 + $EXPOSURE_TIME_DAY;
$dec1 = 0.0;
$dec2 = $dec1 + $deg_per_exp;
my $DETS_5 = <<"EOF";
1 $mjd1 15.0 $dec1 21.5 566 S130E3S
2 $mjd2 15.0 $dec2 21.5 566 S130E3S
EOF

ok(run_test($DETS_0));
ok(run_test($DETS_1));
ok(!run_test($DETS_2));

ok(run_test($DETS_3));
ok(run_test($DETS_4));
ok(!run_test($DETS_5));
