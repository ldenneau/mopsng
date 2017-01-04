# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use strict;
use Benchmark;
use Test::More tests => 2;
use PS::MOPS::Lib qw(:all);

#########################

sub test_jd {
    mopslib_jd2mjd(2453373.5) == 53373
    and mopslib_jd2mjd(2400000.5) == 0;
}


sub test_mjd {
    mopslib_mjd2jd(53373) == 2453373.5
    and mopslib_mjd2jd(0) == 2400000.5;
}

my $test_module_name = 'PS-MOPS-Lib-3';
my $g_start = new Benchmark;
ok(test_jd,"mopslib_jd2mjd");
ok(test_mjd,"mopslib_,jd2jd");
my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";
