# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 2.t'

#########################

use Test::More tests => 1;
use PS::MOPS::DC::Instance;
use File::Temp;

#########################

sub test_import {
    my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');
    $inst->dbh and $inst->new_dbh;

    my $tmpdir = File::Temp::tempdir(CLEANUP => 1);
    $inst->makeNNDir(NN => 54100, SUBSYS => 'foo');
    return -d join('/', $inst->getEnvironment('NNDIR'), '54100');
}

ok(test_import, 'test_import');
