use strict;
use warnings;
use Benchmark;
use Test::More tests => 5;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);
use PS::MOPS::DC::Orbit;


my $test_module_name = 'PS-MOPS-Lib-3';
my $g_start = new Benchmark;

my %orb1 = qw(
    q 1.252068 
    e 0.382 
    i 9.3 
    node 252.1 
    argPeri 185.6 
    timePeri 53015.0711320161 
    hV 10.315 
    epoch 52860 
);

my %orb2 = qw(
    q 0.74812 
    e 0.528 
    i 10.2 
    node 201.6 
    argPeri 31.8 
    timePeri 52787.5192209622 
    hV 18.064 
    epoch 52860 
);

my %orb3 = qw(
    q 0.74812 
    e 0.528 
    i 10.2 
    node 179.9999
    argPeri 179.9999
    timePeri 52787.5192209622 
    hV 18.064 
    epoch 52860 
);

my %orb4 = qw(
    q 0.74812 
    e 0.528 
    i 10.2 
    node 180.0001
    argPeri 180.0001
    timePeri 52787.5192209622 
    hV 18.064 
    epoch 52860 
);

my $inst = undef;

my $orb1 = PS::MOPS::DC::Orbit->new($inst, %orb1);
my $orb2 = PS::MOPS::DC::Orbit->new($inst, %orb2);
my $orb3 = PS::MOPS::DC::Orbit->new($inst, %orb3);
my $orb4 = PS::MOPS::DC::Orbit->new($inst, %orb4);


sub _near {
    my ($v1, $v2) = @_;
    return 1 if (
        ($v1 == 0 && $v2 == 0) 
        or $v1 == $v2
        or abs($v1 - $v2) < .00001
    );
    return 0;
}


sub compare_same {
    my ($d3, $d4) = mopslib_calculateDCriterion($orb1, $orb1);
    return _near($d3, 0) && _near($d4, 0);
}


sub compare {
    my ($d3, $d4) = mopslib_calculateDCriterion($orb1, $orb2);
    my $ref_d3 = 0.5443853748;
    my $ref_d4 = 1.043862275;
    return _near($d3, $ref_d3) && _near($d4, $ref_d4);
}


ok(compare_same(), 'same');
ok(compare(), 'basic');    
ok(_near(mopslib_calculateDCriterion($orb3, $orb4), 0), '180-1');    

$orb3->argPeri(-0.00001);
$orb4->argPeri(+0.00001);
ok(_near(mopslib_calculateDCriterion($orb3, $orb4), 0), 'argPeri-near-zero');    

$orb3->node(-0.00001);
$orb4->node(+0.00001);
ok(_near(mopslib_calculateDCriterion($orb3, $orb4), 0), 'node-near-zero');    


my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";
