# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use strict;
use warnings;
use Test::More tests => 7;
BEGIN { use_ok('PS::MOPS::IDManager') };

#########################

my $mgr = new PS::MOPS::IDManager;

my $line;
my ($id, $resid);
my $stuff;
my @members;

while (defined($line = <DATA>)) {
    chomp $line;
    ($id, $resid) = split /\s+/, $line;
    @members = split /=/, $id;
    $stuff = {
        ID => $id,
        MEMBERS => [ @members ],
        FOM => 1 / $resid,
    };
    $mgr->add($stuff);
}
$mgr->analyze();

ok(!$mgr->query_rejected('E=F=G'), 'keep E=F=G');
ok(!$mgr->query_rejected('K=I=L'), 'keep K=I=L');
ok(!$mgr->query_rejected('A=B=C=D'), 'keep A=B=C=D');
ok(!$mgr->query_rejected('M=N=O'), 'keep M=N=O');
ok($mgr->query_rejected('C=F=O'), 'reject C=F=O');
ok($mgr->query_rejected('A=I=J'), 'reject A=I=J');

# Set up our data area.  These are ficticious derived objects whose IDs
# describe their tracklet compositions, which we'll need for the test.
# The surviving objects should be
#   E=F=G
#   K=I=L
#   A=B=C=D
#   M=N=O
# and rejects should be
#   C=F=O
#   A=I=J
__DATA__
A=B=C=D     0.4
E=F=G       0.1
A=I=J       0.5
K=I=L       0.2
M=N=O       0.6
C=F=O       0.3
