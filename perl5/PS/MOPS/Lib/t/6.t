use strict;
use warnings;
use Benchmark;
use Test::More tests => 13;
use PS::MOPS::Lib qw(:all);


sub test_toB62 {
    ok(mopslib_toB62(0, '00000') eq '00000', 'basic');
    ok(mopslib_toB62(1, '00000') eq '00001', 'basic nonzero');
    ok(mopslib_toB62(1, 'LARRY00000') eq 'LARRY00001', 'prefix');
    ok(mopslib_toB62(10, 'LARRY00000') eq 'LARRY0000a', 'prefix10');
    ok(mopslib_toB62(10 + 26, 'LARRY00000') eq 'LARRY0000A', 'prefix36');
    ok(mopslib_toB62(10 + 26 + 26, 'LARRY00000') eq 'LARRY00010', 'prefix62');
}


sub test_fromB62 {
    ok(mopslib_fromB62('00000') == 0, 'fromB62 zero');
    ok(mopslib_fromB62('00001') == 1, 'fromB62 nonzero');
    ok(mopslib_fromB62(mopslib_toB62(42, '000000000')) == 42, 'fromB62 42');
    ok(mopslib_fromB62(mopslib_toB62(4242, '000000000')) == 4242, 'fromB62 4242');
    ok(mopslib_fromB62(mopslib_toB62(424242, '000000000')) == 424242, 'fromB62 424242');
    ok(mopslib_fromB62(mopslib_toB62(4243425, '000000000')) == 4243425, 'fromB62 4242425');
    ok(mopslib_fromB62(mopslib_toB62(999999, '000000000')) == 999999, 'fromB62 999999');
}


my $test_module_name = 'PS-MOPS-Lib-6';
my $g_start = new Benchmark;
test_toB62;
test_fromB62;
my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";
