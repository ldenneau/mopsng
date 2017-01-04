use strict;
use warnings;
use Benchmark;
use Test::More tests => 5;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);


my @DATA = <DATA>;  # slurp into global

sub test_jd2ocnum {
    foreach (@DATA) {
        my ($jd, $want) = split /\s+/, $_, 2;
        return unless mopslib_jd2ocnum($jd) == $want;
    }
    1;  # all OK
}

sub test_mjd2ocnum {
    foreach (@DATA) {
        my ($jd, $want) = split /\s+/, $_, 2;
        return unless mopslib_mjd2ocnum($jd - $MJD2JD_OFFSET) == $want;
    }
    1;  # all OK
}

sub test_ocnum2jd {
    return 
        mopslib_ocnum2jd(0) == 2451564 and
        mopslib_ocnum2mjd(0) == 51563.5 and
        mopslib_ocnum2mjd(63) == 51563.5 and

    1;  # all OK
}

sub test_mjd2localtimestr {
    return 
        mopslib_mjd2localtimestr(54106, -10) eq '2007-01-05T14:00:00.0' 
        and mopslib_mjd2localtimestr(54106, -10, ' ') eq '2007-01-05 14:00:00.0';
}

sub test_mjd2utctimestr {
    return 
        mopslib_mjd2utctimestr(54106) eq '2007-01-06T00:00:00.0Z' 
        and mopslib_mjd2utctimestr(54106, ' ') eq '2007-01-06 00:00:00.0Z'
        and mopslib_mjd2utctimestr(54106.5) eq '2007-01-06T12:00:00.0Z' 
        and mopslib_mjd2utctimestr(54106.5, ' ') eq '2007-01-06 12:00:00.0Z';
}


my $test_module_name = 'PS-MOPS-Lib-7';
my $g_start = new Benchmark;
ok(test_jd2ocnum,"jd2ocnum");
ok(test_mjd2ocnum,"mjd2ocnum");
ok(test_ocnum2jd,"ocnum2jd");
ok(test_mjd2localtimestr,"mjd2localtimestr");
ok(test_mjd2utctimestr,"utc2localtimestr");
my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";

__DATA__
2451563.4   -1
2451563.6   -1
2451564     -1
2451564.4   -1
2451564.49  -1
2451564.5   0
2451564.51  0
2451564.6   0
2451564.69729 0
2451564.7   0
2451593.4   0
2451593.6   0
2451594.4   0
2451594.49  0
2451594.5   1
2451594.51  1
2451594.6   1
2451623.4   1
2451623.6   1
2451624.4   1
2451624.6   2
2451652.49  2
2451652.51  2
2451653.49  2
2451653.51  3
2451682.49  3
2451682.51  3
2451683.49  3
2451683.51  4
2451919.35  11
2451919.39  11
2451919.50  12
2453453.49  63
2453453.51  63
2453454.49  63
2453454.51  64
2455343.49  127
2455343.51  127
2455344.49  127
2455344.51  128
