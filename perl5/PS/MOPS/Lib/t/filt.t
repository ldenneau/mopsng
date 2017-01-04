use strict;
use warnings;
use Benchmark;

use Test::More tests => 16;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);
use PS::MOPS::DC::Orbit;


my $test_module_name = 'PS-MOPS-Lib-3';
my $g_start = new Benchmark;

my $mag = 22;

ok(mopslib_V2filt($mag, 'V') == 22 + 0.0, 'V2filt(V)');
ok(mopslib_V2filt($mag, ' ') == 22 + 0.0, 'V2filt( )');
ok(mopslib_V2filt($mag, 'w') == 22 + 0.5, 'V2filt(w)');
ok(mopslib_V2filt($mag, 'g') == 22 + 0.5, 'V2filt(g)');
ok(mopslib_V2filt($mag, 'r') == 22 + 0.1, 'V2filt(r)');
ok(mopslib_V2filt($mag, 'i') == 22 - 0.3, 'V2filt(i)');
ok(mopslib_V2filt($mag, 'z') == 22 + 0.1, 'V2filt(z)');
ok(mopslib_V2filt($mag, 'y') == 22 - 0.0, 'V2filt(z)');

ok(mopslib_filt2V($mag, 'V') == 22 + 0.0, 'filt2V(V)');
ok(mopslib_filt2V($mag, ' ') == 22 + 0.0, 'filt2V( )');
ok(mopslib_filt2V($mag, 'w') == 22 - 0.5, 'filt2V(w)');
ok(mopslib_filt2V($mag, 'g') == 22 - 0.5, 'filt2V(g)');
ok(mopslib_filt2V($mag, 'r') == 22 - 0.1, 'filt2V(r)');
ok(mopslib_filt2V($mag, 'i') == 22 + 0.3, 'filt2V(i)');
ok(mopslib_filt2V($mag, 'z') == 22 - 0.1, 'filt2V(z)');
ok(mopslib_filt2V($mag, 'y') == 22 + 0.0, 'filt2V(z)');

my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";
