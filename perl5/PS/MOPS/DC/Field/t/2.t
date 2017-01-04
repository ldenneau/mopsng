# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use Test::More tests => 7;
use strict;
use warnings;

use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Field;

#########################

my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');

my %dummy_field = (
    de => [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    dec => 45,
    decSigma => 0,
    epoch => 50000,
    filter => 'DD',
    limitingMag => 24,
    obscode => 568,
    ra => 30,
    raSigma => 0,
    surveyMode => 'DD',
    timeStart => 50000,
    timeStop => 50001,
    fpaId => 'FOO',
    pa_deg => 3.14,
    FOV_deg => 3.0,
);

my %dummy_for_delete = (
    de => [1, 2, 3, 4, 5, 6, 7, 8, 9, 10],
    dec => 23,
    decSigma => 0,
    epoch => 50000,
    filter => 'DD',
    limitingMag => 24,
    obscode => 568,
    ra => 25,
    raSigma => 0,
    surveyMode => 'DD',
    timeStart => 50000,
    timeStop => 50001,
    fpaId => 'FOO',
    pa_deg => 3.14,
    FOV_deg => 3.0,
);


sub _compare_de {
    # Return true if two de arrays contain the same values.
    my ($de1, $de2) = @_;
    my $n;
    for ($n = 0; $n < 10; $n++) {
        return 0 if $de1->[$n] != $de2->[$n];
    }
    return 1;   # all match
}


sub _compare_field_hashref {
    my ($field, $hashref) = @_;
    _compare_de($field->de, $hashref->{de}) and
    $field->dec == $hashref->{dec} and
    $field->decSigma == $hashref->{decSigma} and
    $field->epoch == $hashref->{epoch} and
    $field->filter eq $hashref->{filter} and
    $field->limitingMag == $hashref->{limitingMag} and
    $field->obscode eq $hashref->{obscode} and
    $field->ra == $hashref->{ra} and
    $field->raSigma == $hashref->{raSigma} and
    $field->surveyMode eq $hashref->{surveyMode} and
    $field->timeStart == $hashref->{timeStart} and
    $field->timeStop == $hashref->{timeStop} and
    $field->pa_deg == $hashref->{pa_deg} and
    $field->FOV_deg == $hashref->{FOV_deg};
}


sub make_field1 {
    my %fields = %dummy_field;
    my $field = PS::MOPS::DC::Field->new($inst, %fields);
    return 0 unless $field->ocnum == -53;
    return 0 unless $field->surveyMode eq 'DD';
    return _compare_field_hashref($field, \%fields);
}


sub make_field2 {
    my %fields = %dummy_field;
    my $field = PS::MOPS::DC::Field->new($inst, %fields);
    return 0 unless $field->ocnum == -53;
    return 0 unless $field->surveyMode eq 'DD';
    return _compare_field_hashref($field, \%fields);
}


sub insert_field {
    my $field = PS::MOPS::DC::Field->new($inst, %dummy_field);
    $field->epoch(50001);
    my $id = $field->insert($field);  
    return $id;
}


sub insert_method {
    my $field = PS::MOPS::DC::Field->new($inst, %dummy_field);
    $field->epoch(50002);
    my $id = $field->insert();  
    return $id;
}


sub retrieve_field {
    # different from dummy; dont want to get an identical dummy by mistake
    my %fields = %dummy_for_delete;
    my $field = PS::MOPS::DC::Field->new($inst, %fields);
    my $id = $field->insert;
    my $ret;

    # Fetch it from DB.
    $field = modcf_retrieve($inst, fieldId => $id);
    if ($field) {
        $ret =  _compare_field_hashref($field, \%fields);
    }
    modcf_delete($inst, fieldId => $id) if $id;
    return $ret;
}


sub delete_field1 {
    my $field = PS::MOPS::DC::Field->new($inst, %dummy_field);
    my $id = $field->insert;
    return unless $id;
    return modcf_delete($inst, fieldId => $field->fieldId);
}


sub delete_field2 {
    my $field = PS::MOPS::DC::Field->new($inst, %dummy_field);
    my $id = $field->insert;
    return unless $id;
    return $field->delete;
}


ok(make_field1(), 'create1');
ok(make_field2(), 'create2');
ok(insert_field(), 'insert');
ok(insert_method(), 'insert_method');
ok(retrieve_field(), 'retrieve');
ok(delete_field1(), 'delete1');
ok(delete_field2(), 'delete2');
