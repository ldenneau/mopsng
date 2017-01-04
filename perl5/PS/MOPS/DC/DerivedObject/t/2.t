use strict;
use warnings;

use PS::MOPS::Lib;

use Test::More tests => 2;

use PS::MOPS::DC::SSM;
use PS::MOPS::DC::DerivedObject;
use PS::MOPS::DC::Orbit;

#########################

use PS::MOPS::Test::Instance;
my $test_inst = PS::MOPS::Test::Instance->new();
my $inst = $test_inst->getPSMOPSInstance();


sub make_ssm {
    my $id = modcs_insertByValue(
        $inst,
        q => 1.23,
        e => .02,
        i => 4.0,
        node => 167,
        argPeri => 34,
        timePeri => 58792,
        epoch => 58789,
        hV => 15,
        objectName => 'Slippy',
    );
    return $id;
}


sub create {
    my ($ssm_id) = @_;
    my $orb = PS::MOPS::DC::Orbit->new($inst,
        q => 1,
        e => 2,
        i => 3,
        node => 4,
        argPeri => 5,
        timePeri => 6,
        epoch => 50000,
        chiSquared => 0,
        hV => 10,
        residual => .42,
    );
    $orb->insert();

    my $dobj = PS::MOPS::DC::DerivedObject->new($inst,
        orbitId => $orb->orbitId,
        classification => 'C',
        ssmId => $ssm_id,
        status => 'Q',
    );
    $dobj->insert();

    return $dobj;
};


sub retrieve1 {
    my ($ssm_id) = @_;
    my $dobj1 = create($ssm_id);
    my $dobj2 = modcdo_retrieve($inst, objectName => $dobj1->objectName);
    return 
        $dobj2->ssmId == $ssm_id
        and $dobj2->status eq 'Q';
}


sub retrieve2 {
    my ($ssm_id) = @_;
    my $dobj1 = create($ssm_id);
    my $dobj2 = modco_retrieve($inst, derivedobjectName => $dobj1->objectName);
    my $dobj3 = modco_retrieve($inst, derivedobjectNames => [$dobj1->objectName]);
    return $dobj2 && $dobj3;
}

my $ssm_id = make_ssm($inst);
ok(retrieve1($ssm_id), 'retrieve');
ok(retrieve2($ssm_id), 'retrieve via modco');
