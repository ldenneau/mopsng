use strict;
use warnings;

use PS::MOPS::Lib;

use Test::More tests => 4;
BEGIN { use_ok('PS::MOPS::DC::DerivedObject') };
use PS::MOPS::Test::Instance;

#########################

my $test_inst = PS::MOPS::Test::Instance->new();
my $inst = $test_inst->getPSMOPSInstance();

sub create1 {
    my ($create_id) = @_;
    $create_id = 42 unless $create_id;      # provide default

    my $dobj = PS::MOPS::DC::DerivedObject->new($inst,
        orbitId => $create_id,
        status => 'Q',
    );
    return undef unless ($dobj && $dobj->orbitId == $create_id && $dobj->status eq 'Q');
    return $dobj;
};


sub create2 {
    my ($create_id) = @_;
    $create_id = 42 unless $create_id;      # provide default

    my $dobj = PS::MOPS::DC::DerivedObject->new($inst,
        orbitId => $create_id,
        classification => 'C',
        ssmId => 42,
        status => 'Q',
    );
    return undef unless ($dobj && $dobj->orbitId == $create_id &&
        $dobj->status eq 'Q' &&
        $dobj->classification eq 'C' && $dobj->ssmId == 42);
    return $dobj;
};


sub insert {
    my $dobj;
    my $x;

    $dobj = create1();
    $dobj->insert();
    $x = $dobj->derivedobjectId;
    return 0 unless $x;
    return 0 unless mopslib_toB62($x, 
        $PS::MOPS::DC::DerivedObject::B62_TEMPLATE) eq $dobj->objectName;    # test ID=>B62 conversion

    $dobj = create1();
    $dobj->insert();
    $x = $dobj->derivedobjectId;
    return 0 unless $x;
    return 0 unless mopslib_toB62($x, 
        $PS::MOPS::DC::DerivedObject::B62_TEMPLATE) eq $dobj->objectName;    # test ID=>B62 conversion
}


sub retrieve {
    return modcdo_retrieve($inst, all => 1) > 0;
}


ok(create1(), 'create1');
ok(create2(), 'create2');
# 'insert' fails with InnoDB because of FK; need to create orbit table/rows for this to work.
#ok(insert(), 'insert');
ok(retrieve(), 'retrieve');
