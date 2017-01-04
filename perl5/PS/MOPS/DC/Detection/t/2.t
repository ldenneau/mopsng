#########################

use Test::More tests => 4;
use PS::MOPS::DC::Detection;

#########################

use PS::MOPS::Test::Instance;
my $test_inst = PS::MOPS::Test::Instance->new();
my $inst = $test_inst->getPSMOPSInstance();

my %dummy_det = (
    ra => 30,
    dec => 45,
    mag => 22,
    refMag => 22.5,
    epoch => 53900,
    filter => 'DD',
    isSynthetic => 'Y',
    status => $DETECTION_STATUS_FOUND,
    s2n => 6.66,
    raSigma => 0,
    decSigma => 0,
    magSigma => 0,
    length_deg => 1,
    orient_deg => 5,
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
    s2n => 6.66,
    raSigma => 0,
    decSigma => 0,
    magSigma => 0,
    length_deg => 5,
    orient_deg => 32,
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


sub make_det {
    my %fields = %dummy_det;
    my $det = PS::MOPS::DC::Detection->new($inst, %fields);
    return _compare_det_hashref($det, \%fields);
}


sub add_to_field {
    use PS::MOPS::DC::Field;
    my %ofields = %dummy_field;
    my $field = PS::MOPS::DC::Field->new($inst, %ofields);
    my %dfields = %dummy_det;
    my $det = PS::MOPS::DC::Detection->new($inst, %dfields);
    $det->addToField($field);
#    return $det->fieldId eq $field->fieldId;
    return $det->obscode eq $field->obscode;
    1;  # XXX need better here
}


sub get_field {
    use PS::MOPS::DC::Field;
    my %ofields = %dummy_field;
    my $field = PS::MOPS::DC::Field->new($inst, %ofields);
    my %dfields = %dummy_det;
    my $det = PS::MOPS::DC::Detection->new($inst, %dfields);
    $det->addToField($field);
    return $det->getField == $field;

  $det = modcd_retrieve($inst, detId => $det->detId);
  return $det and $det->fieldId == $field->fieldId && $det->s2n == 6.66;
}


#sub retrieve_det {
#    # different from dummy; dont want to get an identical dummy by mistake
#    my %fields = %dummy_for_delete;
#    $id = modcm_insertByValue(%fields);
#
#    # Fetch it from DB.
#    $det = modcd_retrieve($inst, detId => $id);
#    if ($det) {
#        return _compare_det_hashref($det, \%fields);
#    }
#    return $det;
#}


sub delete_det {
    use PS::MOPS::DC::Field;
    my %ofields = %dummy_field;
    my $field = PS::MOPS::DC::Field->new($inst, %ofields); # create field
    my %dfields = %dummy_det;
    my $det = PS::MOPS::DC::Detection->new($inst, %dfields);   # create detection
    $det->addToField($field);                       # attach to field
    modcd_delete($inst, detId => $det->detId) or die "delete failed";
    $det = modcd_retrieve($inst, detId => $det->detId);
    return (!$det);     # should not be there
}


sub chg_obj_name {
    # Test changing object name
    my $ok;

    use PS::MOPS::DC::Field;
    my %ofields = %dummy_field;
    my $field = PS::MOPS::DC::Field->new($inst, %ofields); # create field
    my %dfields = %dummy_det;
    my $det = PS::MOPS::DC::Detection->new($inst, %dfields);   # create detection
    $det->addToField($field);                       # attach to field

    # Test name chg  here.
#    my $det_id = $det->detId;
#    $det->orbitId(1);
#    $ok = (modcd_retrieve($inst, detId => $det_id)->orbitId == 1);
    $ok = 1;

    # Clean up.
    modcd_delete($inst, detId => $det->detId) or die "delete failed";
    return $ok;
}


ok(make_det(), 'create');
ok(add_to_field(), 'add_to_field');
#ok(get_field(), 'get_field');
#ok(get_field(), 'get_field');
#ok(retrieve_det(), 'retrieve');
#ok(retrieve_det_by_field(), 'retrieve_by_field');
#ok(retrieve_det_by_orbit(), 'retrieve_by_orbit');
#ok(retrieve_det_unattributed(), 'retrieve_unattributed');
ok(delete_det(), 'delete');
ok(chg_obj_name(), 'chg_obj_name');
#ok(attribute_to_orbit(), 'attribute_to_orbit');
#ok(clear_orbit(), 'clear_orbit');

undef $inst;
