# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use Test::More tests => 2;
BEGIN { use_ok('PS::MOPS::DC::Instance') };

#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.
my $DBNAME = 'psmops_unittest';
my $inst = PS::MOPS::DC::Instance->new(DBNAME => $DBNAME);
ok($inst->dbname eq $DBNAME);
