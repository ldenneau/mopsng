# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl PS-MOPS-DC-History-Removal.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More tests => 4;
use PS::MOPS::DC::History;
BEGIN { use_ok('PS::MOPS::DC::History::Removal') };
use PS::MOPS::Constants qw(:all);
use PS::MOPS::DC::Instance;

#########################

my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');

sub create {
    my $evt = PS::MOPS::DC::History::Removal->new($inst,
        derivedobjectId => 42,
        fieldId => 43,
        orbitId => 44,
        orbitCode => $MOPS_EFF_ORBIT_IODFAIL,
        classification => $MOPS_EFF_CLEAN,
        trackletId => 242,
    );

    return 0 unless $evt->eventType eq $PS::MOPS::DC::History::EVENT_TYPE_REMOVAL;
    return $evt;
}


sub subclass {
    $evt = create();
    return $evt->derivedobjectId == 42 &&
        $evt->trackletId == 242;
}

sub record {
    my $evt = create();
    $evt->record();
}


ok(create(), 'create');
ok(subclass(), 'subclass');
ok(record(), 'record');
