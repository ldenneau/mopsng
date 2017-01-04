use strict;
use warnings;
use Benchmark;

use Test::More tests => 10;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);
use PS::MOPS::DC::Orbit;

my $test_module_name = 'PS-MOPS-Lib-3';
my $g_start = new Benchmark;

my $mag = 22.3;
my $s2n = 9;
ok(mopslib_hsn2flux($mag, $s2n, 'g'), 'hsn2flux(g)');
ok(mopslib_hsn2flux($mag, $s2n, 'r'), 'hsn2flux(r)');
ok(mopslib_hsn2flux($mag, $s2n, 'i'), 'hsn2flux(i)');
ok(mopslib_hsn2flux($mag, $s2n, 'z'), 'hsn2flux(z)');
ok(mopslib_hsn2flux($mag, $s2n, 'y'), 'hsn2flux(y)');

my $flux = 20;
my $flux_sigma = 2;
ok(mopslib_flux2hsn($flux, $flux_sigma, 'g'), 'flux2hsn(g)');
ok(mopslib_flux2hsn($flux, $flux_sigma, 'r'), 'flux2hsn(r)');
ok(mopslib_flux2hsn($flux, $flux_sigma, 'i'), 'flux2hsn(i)');
ok(mopslib_flux2hsn($flux, $flux_sigma, 'z'), 'flux2hsn(z)');
ok(mopslib_flux2hsn($flux, $flux_sigma, 'y'), 'flux2hsn(y)');

my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";
