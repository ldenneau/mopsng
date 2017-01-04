# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 2.t'

#########################

use Test::More tests => 1;
BEGIN { use PS::MOPS::DC::Instance };

#########################

sub test_import {
    my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');
    $inst->dbh and $inst->new_dbh;
}

ok(test_import, 'test_import');
