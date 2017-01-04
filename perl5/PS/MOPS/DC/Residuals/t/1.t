# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More tests => 2;
use strict;
use warnings;
BEGIN { use_ok('PS::MOPS::DC::Residuals') };

use PS::MOPS::Constants qw(:all);
use PS::MOPS::Test::Instance;

#########################

my $t_inst = PS::MOPS::Test::Instance->new();
my $inst = $t_inst->getPSMOPSInstance();

sub new_residuals {
    my $resid = PS::MOPS::DC::Residuals->new($inst,
        detId => 1,
        trackletId => 2,
        raResid_deg => .001,
        decResid_Deg => .002,
        raError_deg => .09,
        decError_deg => .099,
        astReject => 1,
    );

    return
        $resid->detId == 1,
        and $resid->trackletId == 2;
}


sub insert_resid {
    my $resid = PS::MOPS::DC::Residuals->new($inst,
        detId => 1,
        trackletId => 2,
        raResid_deg => .001,
        decResid_Deg => .002,
        raError_deg => .09,
        decError_deg => .099,
        astReject => 1,
    );
    $resid->insert();
}


sub update_resid {
    my $resid = PS::MOPS::DC::Residuals->new($inst,
        detId => 1,
        trackletId => 2,
        raResid_deg => .001,
        decResid_Deg => .002,
        raError_deg => .09,
        decError_deg => .099,
        astReject => 1,
    );
    $resid->insert();

    $resid->raResid_deg = .004;
    $resid->decResid_deg = .005;
    $resid->update();
}

ok(new_resid, 'new_resid');
ok(insert_resid, 'insert_resid');
ok(update_resid, 'update_resid');
