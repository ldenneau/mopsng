# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl PS-MOPS-XFN.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More tests => 4;
use Data::Dumper;
BEGIN { use_ok('PS::MOPS::XFN') };

#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.

use PS::MOPS::DC::SSM;
use PS::MOPS::DC::Shape;
use PS::MOPS::DC::Instance;

my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');
my @keys = ('S000001');


my $res = PS::MOPS::XFN::xfnCallFunction(
    prog => 'shape.x',
    keyidx => 0,                # objectName is in 8th col
    input => \@keys,
    infn => sub {
        # Given an input key, convert to a line of text for our program
        my $objectName = shift;
        my $sh = modcsh_retrieve($inst, objectName => $objectName);
        my $obj = modcs_retrieve($inst, objectName => $objectName);
        return undef unless $sh;
        return (join ' ',
            (map { sprintf "%.10f", $_ } (
                $obj->q, $obj->e, $obj->i, $obj->node, $obj->argPeri, $obj->timePeri, $obj->hV, $obj->epoch
            )), 
            $obj->objectName,
            (map { sprintf "%.10f", $_ } (
                $sh->g, $sh->p, $sh->period_d, $sh->amp_mag, $sh->a_km, $sh->b_km, $sh->c_km, 
                    $sh->beta_deg, $sh->lambda_deg, $sh->ph_a, $sh->ph_d, $sh->ph_k,
                53379
            ))
            ) . "\n"
            ;
    },
    outfn => sub {
        # Given an output line of text from our program, split it into a hashref
        my $line = shift;
        my @goo = split /\s+/, $line;
        return {
            mag => $goo[-1],    # [-1] => shape mag
        };
    }
);
ok(abs($res->{S000001}->{mag} - 15.8) < .1, 'shape.x 1');

my $res2 = PS::MOPS::XFN::xfnSSMShapeMag(
    instance => $inst,
    epoch => 53379,
    objectNames => [ qw(S000001) ],
);
ok(abs($res2->{S000001}->{mag} - 15.8) < .1, 'shape.x 2');


my $res3 = PS::MOPS::XFN::xfnSSMShapeMag(
    instance => $inst,
    epoch => 53379,
    objectNames => [ qw(S000001) ],
    host => 'schmop00',
);
ok(abs($res3->{S000001}->{mag} - 15.8) < .1, 'shape.x 2');

