#########################

use Test::More tests => 2;
use PS::MOPS::Test::Instance;
use PS::MOPS::DC::Runtime;

use strict;
use warnings;

#########################

my $test_inst;
my $inst;

sub build {
    $test_inst = PS::MOPS::Test::Instance->new();
    if ($test_inst) {
        $inst = $test_inst->getPSMOPSInstance();
        return $inst;
    }
    return undef;   # fail
}


sub insert {
    my $evt = PS::MOPS::DC::Runtime->new($inst,
        subsystem => $MOPS_SUBSYSTEM_GLOBAL,
        message => 'CREATE',
    );
    $evt->insert;
}

ok(build(), 'build');
ok(insert(), 'insert');

undef $inst;
