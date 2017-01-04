use strict;
use warnings;

use Test::More tests => 5;
use PS::MOPS::DX;
use PS::MOPS::Constants qw(:all);

# Test object.
my %dummy_det = (
    detId => 42,
    ra => 30,
    dec => 45,
    mag => 22,
    epoch => 53900,
    filter => 'W',
    s2n => 6.66,
    raSigma => 0,
    decSigma => 0,
    magSigma => 0,
    objectName => 'FOO',
    obscode => '568'
);
my $dummy_det_str = "42 53900 O 30 45 22 W 568 0 0 0 6.66 FOO";

# Test object with nonsynthetic detection (missing objectName)
my %dummy_det_ns = (
    detId => 42,
    ra => 30,
    dec => 45,
    mag => 22,
    epoch => 53900,
    filter => 'W',
    s2n => 6.66,
    raSigma => 0,
    decSigma => 0,
    magSigma => 0,
    obscode => '568'
);
my $dummy_det_str_ns = "42 53900 O 30 45 22 W 568 0.0000 0.0000 0.0000 6.66 "; # note OBJECT_NAME eq ''

# Test object with nonsynthetic detection ('NS' objectName)
my %dummy_det_ns2 = (
    detId => 42,
    ra => 30,
    dec => 45,
    mag => 22,
    epoch => 53900,
    filter => 'W',
    s2n => 6.66,
    raSigma => 0,
    decSigma => 0,
    magSigma => 0,
    obscode => '568',
    objectName => $MOPS_NONSYNTHETIC_OBJECT_NAME,
);
my $dummy_det_str_ns2 = "42 53900 O 30 45 22 W 568 0 0 0 6.66 "; # note OBJECT_NAME eq ''


sub _compare_det_hashref {
    my ($det, $hashref) = @_;
    return
        $det->{detId} == $hashref->{detId} and
        $det->{ra} == $hashref->{ra} and
        $det->{dec} == $hashref->{dec} and
        $det->{mag} == $hashref->{mag} and
        $det->{epoch} == $hashref->{epoch} and
        $det->{filter} eq $hashref->{filter} and
        $det->{s2n} eq $hashref->{s2n} and
        $det->{raSigma} == $hashref->{raSigma} and
        $det->{decSigma} == $hashref->{decSigma} and
        $det->{magSigma} == $hashref->{magSigma} and
        $det->{objectName} eq $hashref->{objectName} and
        $det->{obscode} eq $hashref->{obscode};
}


# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.
ok(
    _compare_det_hashref(
        modx_fromDETECTION($dummy_det_str),
        modx_fromDETECTION(modx_toDETECTION(\%dummy_det))
    ),
    'toDETECTION'
);

ok(
    _compare_det_hashref(
        modx_fromDETECTION($dummy_det_str_ns),
        modx_fromDETECTION(modx_toDETECTION(\%dummy_det_ns))
    ),
    'toDETECTION nonsynth1'
);

ok(
    _compare_det_hashref(
        modx_fromDETECTION($dummy_det_str_ns2),
        modx_fromDETECTION(modx_toDETECTION(\%dummy_det_ns2))
    ),
    'toDETECTION nonsynth2'
);

ok(1, 'fromDETECTION');

ok(1, 'fromDETECTION nonsynth');
