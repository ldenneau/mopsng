# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 2.t'

#########################

use Test::More tests => 4;
BEGIN { use PS::MOPS::DC::Instance };

#########################

sub try_connect {
    my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');
    $inst->dbh;
}

sub new_connect {
    my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');
    $dbh1 = $inst->dbh();
    $dbh2 = $inst->new_dbh();
    return "$dbh1" ne "$dbh2";
}

sub no_auto_commit {
    my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');
    $dbh = $inst->new_dbh(flags => {
        AutoCommit => 0
    });
    return !$dbh->{AutoCommit};
}

sub forget {
    my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');
    if ($inst->dbh) {
        $inst->forget_dbh;          # undef DBH
        if (!$inst->{_DBH}) {       # verify undef
            return $inst->dbh;      # def it
        }
    }
    return 0;
}

ok(try_connect, 'dbh');
ok(new_connect, 'new_dbh');
ok(no_auto_commit, 'no_auto_commit');
ok(forget, 'forget');
