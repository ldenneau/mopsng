#########################

use Test::More tests => 4;
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Detection;
use PS::MOPS::Constants qw(:all);

#########################

my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.

my %dummy_det = (
    ra => 30,
    dec => 45,
    mag => 22,
    refMag => 22.5,
    epoch => 53900,
    filter => 'DD',
    isSynthetic => 'Y',
    status => $DETECTION_STATUS_FOUND,
    raSigma => 0,
    decSigma => 0,
    magSigma => 0,
    length_deg => 1,
    orient_deg => 5,
    s2n => 8,
);

my %dummy_for_delete = (
    ra => 2,
    dec => 3,
    mag => 21,
    refMag => 22.5,
    epoch => 53500,
    filter => 'DD',
    isSynthetic => 'Y',
    status => $DETECTION_STATUS_FOUND,
    raSigma => 0,
    decSigma => 0,
    magSigma => 0,
    length_deg => 5,
    orient_deg => 32,
    s2n => 8,
);


my %dummy_field = (
    de => [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    dec => 45,
    decSigma => 0,
    epoch => 53900,
    filter => 'DD',
    limitingMag => 24,
    obscode => '568',
    ra => 30,
    raSigma => 0,
    surveyMode => 'DD',
    timeStart => 53900,
    timeStop => 53901,
);


sub _compare_det_hashref {
    my ($det, $hashref) = @_;
    return 
        $det->ra == $hashref->{ra} and
        $det->dec == $hashref->{dec} and
        $det->mag == $hashref->{mag} and
        $det->refMag == $hashref->{refMag} and
        $det->epoch == $hashref->{epoch} and
        $det->filter eq $hashref->{filter} and
        $det->isSynthetic eq $hashref->{isSynthetic} and
        $det->status eq $hashref->{status} and
        $det->raSigma == $hashref->{raSigma} and
        $det->decSigma == $hashref->{decSigma} and
        $det->magSigma == $hashref->{magSigma} and
        $det->orient_deg eq $hashref->{orient_deg} and
        $det->length_deg eq $hashref->{length_deg};
}


sub classify_clean {
    my %fields;
    my @dets;

    %fields = %dummy_det;
    $fields{objectName} = 'S123456';
    push @dets, PS::MOPS::DC::Detection->new($inst, %fields);

    %fields = %dummy_det;
    $fields{objectName} = 'S123456';
    push @dets, PS::MOPS::DC::Detection->new($inst, %fields);

    %fields = %dummy_det;
    $fields{objectName} = 'S123456';
    push @dets, PS::MOPS::DC::Detection->new($inst, %fields);

    my ($classification, $objectName) = modcd_classifyDetections($inst, @dets);
    return $classification eq $MOPS_EFF_CLEAN and $objectName eq 'S123456';
}


sub classify_mixed {
    my %fields;
    my @dets;

    %fields = %dummy_det;
    $fields{objectName} = 'S123456';
    push @dets, PS::MOPS::DC::Detection->new($inst, %fields);

    %fields = %dummy_det;
    $fields{objectName} = 'S424242';
    push @dets, PS::MOPS::DC::Detection->new($inst, %fields);

    %fields = %dummy_det;
    $fields{objectName} = 'S123456';
    push @dets, PS::MOPS::DC::Detection->new($inst, %fields);

    return modcd_classifyDetections($inst, @dets) eq $MOPS_EFF_MIXED;
}


sub classify_bad {
    my %fields;
    my @dets;

    %fields = %dummy_det;
    $fields{objectName} = 'S123456';
    push @dets, PS::MOPS::DC::Detection->new($inst, %fields);

    %fields = %dummy_det;
    $fields{objectName} = $MOPS_NONSYNTHETIC_OBJECT_NAME;
    push @dets, PS::MOPS::DC::Detection->new($inst, %fields);

    %fields = %dummy_det;
    $fields{objectName} = undef;
    push @dets, PS::MOPS::DC::Detection->new($inst, %fields);

    return modcd_classifyDetections($inst, @dets) eq $MOPS_EFF_BAD;
}


sub classify_nonsynth {
    my %fields;
    my @dets;

    %fields = %dummy_det;
    $fields{objectName} = '';
    push @dets, PS::MOPS::DC::Detection->new($inst, %fields);

    %fields = %dummy_det;
    $fields{objectName} = $MOPS_NONSYNTHETIC_OBJECT_NAME;
    push @dets, PS::MOPS::DC::Detection->new($inst, %fields);

    %fields = %dummy_det;
    $fields{objectName} = undef;
    push @dets, PS::MOPS::DC::Detection->new($inst, %fields);

    return modcd_classifyDetections($inst, @dets) eq $MOPS_EFF_NONSYNTHETIC;
}

ok(classify_clean(), 'classify_clean');
ok(classify_mixed(), 'classify_mixed');
ok(classify_bad(), 'classify_bad');
ok(classify_nonsynth(), 'classify_nonsynth');
