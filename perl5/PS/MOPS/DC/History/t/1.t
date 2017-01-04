use strict;
use warnings;

use PS::MOPS::Lib;

use Test::More tests => 5;
BEGIN { use_ok('PS::MOPS::DC::History') };
use PS::MOPS::Constants qw(:all);
use PS::MOPS::DC::Field;
use PS::MOPS::DC::Orbit;
use PS::MOPS::DC::DerivedObject;
use PS::MOPS::Test::Instance;

#########################

my $test_inst = PS::MOPS::Test::Instance->new();
my $inst = $test_inst->getPSMOPSInstance();


sub make_field {
    my ($inst) = @_;
    my $epoch_mjd = 54323;
    my $exp_d = 30 / 86400;         # 30 sec in days
    my @de = [0..9];
    my $field = PS::MOPS::DC::Field->new($inst,
        ra => 10.0,
        dec => 90.0,
        raSigma => .1,
        decSigma => .1,
        epoch => $epoch_mjd,
        filter => 'r',
        surveyMode => 'DD',
        limitingMag => 23.5,
        timeStart => $epoch_mjd - $exp_d / 2,
        timeStop => $epoch_mjd + $exp_d / 2,
        de => \@de,
        obscode => '568',
    );
    $field->insert();
    return $field;
}


sub make_orbit {
    my ($inst) = @_;
    my $orbit = PS::MOPS::DC::Orbit->new($inst,
        q => 1.3,
        e => .01,
        i => 4.2,
        node => 53,
        argPeri => 127,
        timePeri => 57238,
        epoch => 57829,
        hV => 15,
        residual => .223,
    );
    $orbit->insert();
    return $orbit;
}


sub make_do {
    my ($inst, $orbit, $field) = @_;
    my $do = PS::MOPS::DC::DerivedObject->new($inst,
        orbitId => $orbit->orbitId,
    );
    $do->insert();
    return $do;
}


sub create {
    my $field = make_field($inst);
    my $orbit = make_orbit($inst);
    my $do = make_do($inst, $orbit, $field);

    my $h = PS::MOPS::DC::History->new($inst,
        eventType => $EVENT_TYPE_DERIVATION,
        derivedobjectId => $do->derivedobjectId,
        fieldId => $field->fieldId,
        orbitId => $orbit->orbitId,
        d3 => .5,
        d4 => .25,
        orbitCode => $MOPS_EFF_ORBIT_IODFAIL,
        classification => $MOPS_EFF_CLEAN,
    );
    return $h;
};

sub record {
    my $field = make_field($inst);
    my $orbit = make_orbit($inst);
    my $do = make_do($inst, $orbit, $field);

    my $h = PS::MOPS::DC::History->new($inst,
        eventType => $EVENT_TYPE_DERIVATION,
        derivedobjectId => $do->derivedobjectId,
        fieldId => $field->fieldId,
        orbitId => $orbit->orbitId,
        d3 => .5,
        d4 => .25,
        orbitCode => $MOPS_EFF_ORBIT_IODFAIL,
        classification => $MOPS_EFF_CLEAN,
    );
    $h->record();
    return $h;
};

sub retrieve {
    my $zug = record();
    my $id = $zug->eventId;
    $zug = modch_retrieve($inst, eventId => $id);
    return $zug;
}


sub ssmid_fieldid {
    # XXX LD flush out
    return 1;
}

ok(create(), 'create');
ok(record(), 'record');
ok(retrieve(), 'retrieve');
ok(ssmid_fieldid(), 'ssm_id/field_id');
