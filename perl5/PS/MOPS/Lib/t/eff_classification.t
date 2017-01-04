use strict;
use warnings;
use Benchmark;

use Test::More tests => 4;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);

my $test_module_name = 'PS-MOPS-Lib-3';
my $g_start = new Benchmark;


ok(mopslib_effClassifyObjectNames(qw(
    S000    
    S000    
    S000    
)) eq $MOPS_EFF_CLEAN);

ok(mopslib_effClassifyObjectNames(qw(
    S000    
    S000    
    S001    
)) eq $MOPS_EFF_MIXED);

ok(mopslib_effClassifyObjectNames(qw(
    S000    
    S000    
    S000    
), $MOPS_NONSYNTHETIC_OBJECT_NAME
) eq $MOPS_EFF_BAD);

ok(mopslib_effClassifyObjectNames(
    $MOPS_NONSYNTHETIC_OBJECT_NAME,
    $MOPS_NONSYNTHETIC_OBJECT_NAME,
    $MOPS_NONSYNTHETIC_OBJECT_NAME,
) eq $MOPS_EFF_NONSYNTHETIC);

my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";
