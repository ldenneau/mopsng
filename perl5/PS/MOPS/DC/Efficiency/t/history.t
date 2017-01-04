use strict;
use warnings;

use Data::Dumper;
use PS::MOPS::Lib;

use Test::More tests => 5;
use PS::MOPS::DC::Field;
use PS::MOPS::DC::Efficiency;
use PS::MOPS::DC::Instance;

#########################

my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');
my $field = modcf_retrieve($inst, fieldId => 1);

#sub by_field {
#    my $res = modce_retrieve($inst, category => $EFF_DETECTIONS, fieldId => $field->fieldId);
#    $res;
#};

sub history {
    my ($category) = @_;
    my $res = modce_retrieve($inst, category => $category);
#    print STDERR Dumper($res);
    $res;
};

ok(history($EFF_DERIVEDOBJECTS), 'derivedobjects');
ok(history($EFF_ATTRIBUTIONS), 'attributions');
ok(history($EFF_PRECOVERIES), 'precoveries');
ok(history($EFF_IDENTIFICATIONS), 'identifications');
ok(history($EFF_REMOVALS), 'removals');
