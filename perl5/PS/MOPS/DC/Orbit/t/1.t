use strict;
use warnings;

use PS::MOPS::Lib;

use Test::More tests => 7;
BEGIN { use_ok('PS::MOPS::DC::Orbit') };
use PS::MOPS::Test::Instance;

#########################

my $test_inst = PS::MOPS::Test::Instance->new();
my $inst = $test_inst->getPSMOPSInstance();


sub create {
    my $orb = PS::MOPS::DC::Orbit->new($inst,
        q => 5,                     # AU
        e => 0.1,                   # dimensionless
        i => 6.0,                   # deg
        node => 35.0,               # deg
        argPeri => 72.0,            # deg
        timePeri => 53800,          # MJD
        hV => 17,                   # dimensionless
        residual => 0.123,          # arcsec
        chiSquared => 7,
        epoch => 53900,             # MJD
        moid_1 => 2.5,
        arcLength_days => 200,
    );
    return 0 unless ($orb && $orb->argPeri == 72.0 && $orb->moid_1 == 2.5 && $orb->arcLength_days == 200);
    $orb;
}

sub create_no_resid {
    # Same as above, unattached, no residual.
    my $orb = PS::MOPS::DC::Orbit->new($inst,
        q => 5,                     # AU
        e => 0.1,                   # dimensionless
        i => 6.0,                   # deg
        node => 35.0,               # deg
        argPeri => 72.0,            # deg
        timePeri => 53800,          # MJD
        hV => 17,                   # dimensionless
        chiSquared => 7,
        epoch => 53900,             # MJD
    );
    return 0 unless ($orb && $orb->argPeri == 72.0);
    $orb;
};


sub create_from_iod {
    my @stuff = (
        5,                      # AU
        0.1,                    # dimensionless
        6.0,                    # deg
        35.0,                   # deg
        72.0,                   # deg
        53800,                  # MJD
        17,                     # dimensionless
        53900,                  # MJD
        'DUMMEH',
        0.123,                  # arcsec
    );
    my $str = join " ", @stuff;
    my $orb = PS::MOPS::DC::Orbit->new_from_iodtxt($inst, $str);
    return 0 unless ($orb && $orb->argPeri == 72.0);
    return $orb;
};


sub insert {
    my $orb;
    $orb = create();
    $orb->insert();
    return $orb;
}


sub retrieve {
    my $orb;
    $orb = create();
    $orb->insert();

    my $fetched = modco_retrieve($inst, orbitId => $orb->orbitId);
    return $fetched;
}


sub covariance {
    my $orb = PS::MOPS::DC::Orbit->new($inst,
        q => 5,                     # AU
        e => 0.1,                   # dimensionless
        i => 6.0,                   # deg
        node => 35.0,               # deg
        argPeri => 72.0,            # deg
        timePeri => 53800,          # MJD
        hV => 17,                   # dimensionless
        residual => 0.123,          # arcsec
        chiSquared => 7,
        epoch => 53900,             # MJD

        covariance => [
          100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
          110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
          120,
        ],
    );
    $orb->insert();
    my $fetched = modco_retrieve($inst, orbitId => $orb->orbitId);
    $fetched && $fetched->covariance && @{$fetched->covariance} == 21 && $fetched->covariance->[20] == 120;
};


ok(create(), 'create');
ok(create_no_resid(), 'create_no_resid');
ok(insert(), 'insert');
ok(retrieve(), 'retrieve');
ok(create_from_iod(), 'create_from_iod');
ok(covariance(), 'covariance');
#ok(no_covariance(), 'no_covariance');
#ok(bogus_covariance(), 'bogus_covariance');
