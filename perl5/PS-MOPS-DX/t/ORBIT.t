use strict;
use warnings;

use Test::More tests => 1;
use PS::MOPS::DX;
use PS::MOPS::Constants qw(:all);

# Test object.
my %dummy_orb = (
    objectName => 'foo',
    q => 1.2,
    e => .23,
    i => 5.6,
    node => 7.8,
    argPeri => 9.9,
    timePeri => 53400,
    hV => 12,
    epoch => 53900,
    residual => 42.00
);
my $dummy_orb_str = "foo COM 1.2 .23 5.6 7.8 9.9 53400 12 53900 1 6 -1 MOPS";

sub _compare_orb_hashref {
    my ($orb, $hashref) = @_;
    return
        $orb->{OID} eq $hashref->{OID} and
        $orb->{q} == $hashref->{q} and
        $orb->{e} == $hashref->{e} and
        $orb->{i} == $hashref->{i} and
        $orb->{node} eq $hashref->{node} and
        $orb->{argPeri} eq $hashref->{argPeri} and
        $orb->{timePeri} eq $hashref->{timePeri} and
        $orb->{H} eq $hashref->{H} and
        $orb->{epoch} == $hashref->{epoch}
}


# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.
my ($thing1) = modx_fromORBIT($dummy_orb_str);
my ($thing2) = modx_toORBIT(\%dummy_orb);
ok(
    _compare_orb_hashref($thing1, modx_fromORBIT($thing2)),
    'toORBIT'
);

