# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More tests => 6;
use strict;
use warnings;
BEGIN { use_ok('PS::MOPS::DC::Tracklet') };

use PS::MOPS::Constants qw(:all);
use PS::MOPS::Test::Instance;

#########################

my $t_inst = PS::MOPS::Test::Instance->new();
my $inst = $t_inst->getPSMOPSInstance();

sub new_tracklet {
    use PS::MOPS::DC::Detection;

    my $det1 = PS::MOPS::DC::Detection->new($inst,
        ra => 1,
        dec => 25,
        epoch  => 53375,
        mag => 24,
        filter => 'DD',
        isSynthetic => 1,
        fieldId => 100,
        s2n => .01
    );
    my $det2 = PS::MOPS::DC::Detection->new($inst,
        ra => 359,
        dec => 26,
        epoch  => 53376,
        mag => 24,
        filter => 'DD',
        isSynthetic => 1,
        fieldId => 101,
        s2n => .01
    );
    my $trk = PS::MOPS::DC::Tracklet->new($inst,
        detections => [$det1, $det2],
        probability => 1,
        digest => 99,
        classification => $MOPS_EFF_CLEAN,
    );

    return
        $trk->vRA == -2
        and $trk->vDEC == 1
        and $trk->extEpoch == 53375.5
        and $trk->extRA == 0
        and $trk->extDEC == 75
        and $trk->fieldId == 101;
}


sub insert_tracklet {
    use PS::MOPS::DC::Field;
    use PS::MOPS::DC::Detection;

    my $field = PS::MOPS::DC::Field->new($inst,
        de => [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
        dec => 25,
        decSigma => 0,
        epoch => 50000,
        filter => 'DD',
        limitingMag => 25,
        obscode => '568',
        ra => 40,
        raSigma => 0,
        surveyMode => 'DD',
        timeStart => 50000,
        timeStop => 50000,
    );
    $field->insert();

    my $det1 = PS::MOPS::DC::Detection->new($inst,
        ra => 1,
        dec => 25,
        epoch  => 50000,
        mag => 24,
        filter => 'DD',
        isSynthetic => 1,
        s2n => .01
    );
    my $det2 = PS::MOPS::DC::Detection->new($inst,
        ra => 359,
        dec => 26,
        epoch  => 50001.0,
        mag => 24,
        filter => 'DD',
        isSynthetic => 1,
        s2n => .01
    );
    $det1->addToField($field);
    $det2->addToField($field);

    my $trk = PS::MOPS::DC::Tracklet->new($inst,
        detections => [$det1, $det2],
        probability => 1,
        digest => 99,
        classification => $MOPS_EFF_CLEAN,
    );
    $trk->insert();
}


sub check_status {
    use PS::MOPS::DC::Field;
    use PS::MOPS::DC::Detection;

    my $field = PS::MOPS::DC::Field->new($inst,
        de => [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
        dec => 25,
        decSigma => 0,
        epoch => 50000,
        filter => 'DD',
        limitingMag => 25,
        obscode => '568',
        ra => 40,
        raSigma => 0,
        surveyMode => 'DD',
        timeStart => 50000,
        timeStop => 50000,
    );
    $field->insert();

    my $det1 = PS::MOPS::DC::Detection->new($inst,
        ra => 1,
        dec => 25,
        epoch  => 50000,
        mag => 24,
        filter => 'DD',
        isSynthetic => 1,
        s2n => .01
    );
    my $det2 = PS::MOPS::DC::Detection->new($inst,
        ra => 359,
        dec => 26,
        epoch  => 50001.0,
        mag => 24,
        filter => 'DD',
        isSynthetic => 1,
        s2n => .01
    );
    $det1->addToField($field);
    $det2->addToField($field);

    my $trk = PS::MOPS::DC::Tracklet->new($inst,
        detections => [$det1, $det2],
        probability => 1,
        digest => 99,
        classification => $MOPS_EFF_CLEAN,
    );
    $trk->insert();
    return 0 unless $trk->status eq $TRACKLET_STATUS_UNATTRIBUTED;

    $trk->status($TRACKLET_STATUS_ATTRIBUTED);
    return 0 if $trk->status ne $TRACKLET_STATUS_ATTRIBUTED;

    $trk->status($TRACKLET_STATUS_UNATTRIBUTED);
    return 0 if $trk->status ne $TRACKLET_STATUS_UNATTRIBUTED;

    return 1;
}


sub select_detections {
    use PS::MOPS::DC::Field;
    use PS::MOPS::DC::Detection;
    my $half_exp_time = 30 / 86400 / 2;     # half of 30 seconds in MJD

    my $field1 = PS::MOPS::DC::Field->new($inst,
        de => [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
        dec => 25,
        decSigma => 0,
        epoch => 50002,
        filter => 'DD',
        limitingMag => 25,
        obscode => '568',
        ra => 40,
        raSigma => 0,
        surveyMode => 'DD',
        timeStart => 50002 - $half_exp_time,
        timeStop => 50002 + $half_exp_time,
    );
    $field1->insert();
    my $det1 = PS::MOPS::DC::Detection->new($inst,
        ra => 1,
        dec => 25,
        epoch  => 50002,
        mag => 24,
        filter => 'DD',
        isSynthetic => 1,
        s2n => 6.66,
    );
    $det1->addToField($field1);

    my $field2 = PS::MOPS::DC::Field->new($inst,
        de => [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
        dec => 25,
        decSigma => 0,
        epoch => 50002.1,
        filter => 'DD',
        limitingMag => 25,
        obscode => '568',
        ra => 40,
        raSigma => 0,
        surveyMode => 'DD',
        timeStart => 50002.1 - $half_exp_time,
        timeStop => 50002.1 + $half_exp_time,
    );
    $field2->insert();
    my $det2 = PS::MOPS::DC::Detection->new($inst,
        ra => 359,
        dec => 26,
        epoch  => 50002.1,
        mag => 24,
        filter => 'DD',
        isSynthetic => 1,
        s2n => 5.55,
    );
    $det2->addToField($field2);


    my $trk = PS::MOPS::DC::Tracklet->new($inst,
        detections => [$det1, $det2],
        probability => 1,
        digest => 99,
        classification => $MOPS_EFF_CLEAN,
    );
    $trk->insert();

    my $ti = modct_selectDetracklets($inst, fieldId => $field2->fieldId);
    return $ti && $ti->next->s2n;
}

sub check_ext_wraparound1 {
    use PS::MOPS::DC::Detection;

    my $det1 = PS::MOPS::DC::Detection->new($inst,
        ra => 1,
        dec => 25,
        epoch  => 53375,
        mag => 24,
        filter => 'DD',
        isSynthetic => 1,
        fieldId => 100,
        s2n => .01
    );
    my $det2 = PS::MOPS::DC::Detection->new($inst,
        ra => 357,
        dec => 26,
        epoch  => 53376,
        mag => 24,
        filter => 'DD',
        isSynthetic => 1,
        fieldId => 101,
        s2n => .01
    );
    my $trk = PS::MOPS::DC::Tracklet->new($inst,
        detections => [$det1, $det2],
        probability => 1,
        digest => 99,
        classification => $MOPS_EFF_CLEAN,
    );
    return $trk->extRA == 359;
}


ok(new_tracklet, 'new_tracklet');
ok(insert_tracklet, 'insert_tracklet');
ok(check_status, 'status');
ok(select_detections, 'select_detections');
ok(check_ext_wraparound1, 'ext_wraparound1');
