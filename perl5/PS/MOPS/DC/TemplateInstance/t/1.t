# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use Test::More tests => 3;
BEGIN { use_ok('PS::MOPS::DC::TemplateInstance') };

#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.
my $tinst = PS::MOPS::DC::TemplateInstance->new(
    GMT_OFFSET_HOURS => -10,
);
ok($tinst->dbh, 'dbh');
ok($tinst->dbname, 'dbname');
