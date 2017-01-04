use Test::More tests => 4;
BEGIN { use PS::MOPS::MPC ':all' };

#########################

use warnings;
use Astro::Time;

our @chkdegs = (
    360, 180, 62, 62.5, 10, 14.33, 7, 3.141592654, 0, .1, .0003
);

our @chkhms = (
'23 48 23.30',
'23 48 22.69',
'23 47 13.14',
'23 47 12.62',
'00 02 13.61',
'00 02 12.96',
'23 59 52.32',
'23 59 51.57',
'23 57 12.03',
'23 57 11.19',
'23 54 15.11',
'23 54 14.19',
'23 44 32.76',
'23 44 32.23',
'23 42 36.58',
'23 42 35.97',
'23 40 26.17',
'23 40 25.49',
'00 17 55.03',
'00 17 54.42',
'00 23 42.73',
'00 23 42.30',
'00 22  2.15',
'00 22  1.62',
'23 47 18.35',
);

our @chkdms = (
'-05 18 25.9',
'-05 18 30.3',
'-03 54 24.1',
'-03 54 33.2',
'-01 39 49.5',
'-01 39 49.3',
'-01 40  2.2',
'-01 40  2.5',
'-01 41 40.2',
'-01 41 40.9',
'-01 44 32.3',
'-01 44 33.3',
'+00 49 29.0',
'+00 49 22.1',
'+00 26 30.9',
'+00 26 23.4',
'+00 01 27.8',
'+00 01 19.6',
'+01 22  6.1',
'+01 22  2.1',
'-02 44 12.6',
'-02 44 18.0',
'-03 02 26.7',
'-03 02 32.7',
'-01 28 59.0',
);

# Test HMS/deg and DMS/deg conversions; test against Astro::Time.
sub _close {
    my ($a, $b) = @_;
    return sprintf("%10.5f", $a) == sprintf("%10.5f", $b);
}


sub deg2hms {
    my $deg;

    foreach $deg (@chkdegs) {
        my $ours = _deg2hms($deg);
        my $theirs = deg2str($deg, "H", 3, " ");
        if ($ours ne $theirs) {
            print "$ours != $theirs\n";
            return 0;
        }
    }
    return 1;
}

sub deg2dms {
    my $deg;

    foreach $deg (@chkdegs) {
        my $ours = _deg2dms($deg);
        my $theirs = deg2str($deg, "D", 3, " ");
        if ($ours ne $theirs) {
            print "$ours != $theirs\n";
            return 0;
        }
    }
    return 1;
}

sub hms2deg {
    my $str;

    foreach $str (@chkhms) {
        my $ours = _hms2deg($str);
        my $theirs = str2deg($str, "H");
        if (!_close($ours, $theirs)) {
            print "$ours != $theirs\n";
            return 0;
        }
    }
    return 1;
}

sub dms2deg {
    my $str;

    foreach $str (@chkdms) {
        my $ours = _dms2deg($str);
        my $theirs = str2deg($str, "D");
        if (!_close($ours, $theirs)) {
            print "$ours != $theirs\n";
            return 0;
        }
    }
    return 1;
}

ok(deg2hms(), 'degree_to_HMS');
ok(deg2dms(), 'degree_to_DMS');
ok(hms2deg(), 'HMS_to_degree');
ok(dms2deg(), 'DMS_to_degree');
