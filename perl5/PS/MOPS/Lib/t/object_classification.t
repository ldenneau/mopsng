use strict;
use warnings;
use Benchmark;

use Test::More tests => 9;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);
use PS::MOPS::DC::Orbit;

my $test_module_name = 'PS-MOPS-Lib-3';
my $g_start = new Benchmark;

ok(mopslib_classifyObject({
    q => 1.0, e => .8, i => 3, moid_1 => .01,
}) eq 'PHO', 'PHO');

ok(mopslib_classifyObject({
    q => 1.0, e => .8, i => 3, moid_2 => .01,
}) eq 'PHO', 'PHO');

ok(mopslib_classifyObject({
    q => 1.0, e => .8, i => 3 
}) eq 'NEO', 'NEO');

ok(mopslib_classifyObject({
    q => 1.4, e => .2, i => 3 
}) eq 'MC', 'MC');

ok(mopslib_classifyObject({
    q => 3.0, e => .2, i => 3 
}) eq 'MB', 'MB');

ok(mopslib_classifyObject({
    q => 5.2, e => .05, i => 3 
}) eq 'TRO', 'TRO');

ok(mopslib_classifyObject({
    q => 20.0, e => .05, i => 3 
}) eq 'CEN', 'CEN');

ok(mopslib_classifyObject({
    q => 40.0, e => .05, i => 3 
}) eq 'TNO', 'TNO');

ok(mopslib_classifyObject({
    q => 70.0, e => .05, i => 3 
}) eq 'SDO', 'SDO');

my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";
