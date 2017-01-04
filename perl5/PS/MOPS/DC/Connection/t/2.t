# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 2.t'

#########################

use Test::More tests => 3;
BEGIN { use PS::MOPS::DC::Connection ':all' };

#########################

sub try_connect {
    get_dbh();
}

sub new_connect {
    $dbh1 = get_dbh();
    $dbh2 = new_dbh();
    return "$dbh1" ne "$dbh2";
}

sub no_auto_commit {
    $dbh = new_dbh(flags => {
        AutoCommit => 0
    });
    return !$dbh->{AutoCommit};
}

ok(try_connect, 'get_dbh');
ok(new_connect, 'new_dbh');
ok(no_auto_commit, 'no_auto_commit');
