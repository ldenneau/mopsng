# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl PS-MOPS-DC-History-Identification.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More tests => 4;
use PS::MOPS::DC::Test::Instance;
use PS::MOPS::DC::History;
BEGIN { use_ok('PS::MOPS::DC::History::Identification') };
use PS::MOPS::Constants qw(:all);

#########################

my $test_inst = PS::MOPS::DC::Test::Instance->new();
my $inst = $test_inst->getPSMOPSInstance();


sub create {
    my $evt = PS::MOPS::DC::History::Identification->new($inst,
        derivedobjectId => 42,
        fieldId => 43,
        orbitId => 44,
        orbitCode => $MOPS_EFF_ORBIT_OK,
        classification => $MOPS_EFF_CLEAN,
        childobjectId => 142,
    );

    return 0 unless $evt->eventType eq $PS::MOPS::DC::History::EVENT_TYPE_IDENTIFICATION;
    return $evt;
}


sub subclass {
    $evt = create();
    return 
        $evt->derivedobjectId == 42 &&
        $evt->childobjectId == 142;
}

sub record {
    my $evt = create();
    $evt->record();
}


ok(create(), 'create');
ok(subclass(), 'subclass');
ok(record(), 'record');
