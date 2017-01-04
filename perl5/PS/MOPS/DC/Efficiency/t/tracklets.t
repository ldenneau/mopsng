use strict;
use warnings;

use Data::Dumper;
use PS::MOPS::Lib;

use Test::More tests => 2;
use PS::MOPS::DC::Field;
use PS::MOPS::DC::Efficiency;
use PS::MOPS::DC::Instance;

#########################

my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');
my $field = modcf_retrieve($inst, fieldId => 1);

#sub by_field {
#    my $res = modce_retrieve($inst, category => $EFF_TRACKLETS, fieldId => $field->fieldId);
#    $res;
#};

sub by_mjd {
    my $res = modce_retrieve($inst, category => $EFF_TRACKLETS, epoch_mjd => int($field->epoch));
    $res;
};

sub by_ocnum {
    my $res = modce_retrieve($inst, category => $EFF_TRACKLETS, ocnum => 83);
    $res;
};

ok(by_mjd(), 'by_mjd');
ok(by_ocnum(), 'by_mjd');
