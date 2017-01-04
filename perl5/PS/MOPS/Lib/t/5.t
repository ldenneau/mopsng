# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use Benchmark;
use Test::More tests => 13;
use PS::MOPS::Lib qw(:all);

#########################


sub test_inSquareField1 {
    # Nominal.
    ok(mopslib_inSquareField(1, 1, 5, 1, 1), 'nominal');
    ok(mopslib_inSquareField(1, 1, 5, 1, -1), 'neg dec');
    ok(mopslib_inSquareField(1, 1, 5, 359, 1), 'zero crossing');
    ok(mopslib_inSquareField(1, 1, 5, 359, -1), 'zero crossing+neg dec');
    ok(!mopslib_inSquareField(1, 1, 5, 5, 5), 'not in');
}


sub test_inSquareField2 {
    # High declination.  RA zone should expand.
    ok(mopslib_inSquareField(1, 60, 5, 1, 60), 'high dec 1');
    ok(mopslib_inSquareField(1, 60, 5, 5, 60), 'high dec 2');
    ok(!mopslib_inSquareField(1, 60, 5, 20, 60), 'high dec 2');
}



sub test_inField1 {
    # Nominal.
    ok(mopslib_inField(0, 0, 3, 1.49, 0), 'plus RA');
    ok(mopslib_inField(0, 0, 3, -1.49, 0), 'minus RA');
    ok(mopslib_inField(0.1, 0.1, 3, 359.9, 0.1), 'zero crossing');
    ok(mopslib_inField(0.1, 0.1, 3, 359.9, -0.1), 'zero crossing+neg dec');
    ok(!mopslib_inField(1, 1, 3, 5, 5), 'outside');
}


my $test_module_name = 'PS-MOPS-Lib-5';
my $g_start = new Benchmark;
test_inSquareField1;
test_inSquareField2;
test_inField1;
my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";
