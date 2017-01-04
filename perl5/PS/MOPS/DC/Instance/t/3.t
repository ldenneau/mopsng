#########################

use Test::More tests => 3;
use strict;
use warnings;
BEGIN { use PS::MOPS::DC::Instance };

#########################

sub t_push_autocommit {
    # Push autocommit, check that DBI driver sees it.
    my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');
    my $dbh = $inst->dbh;
    $inst->pushAutocommit(1);
    return $dbh->{AutoCommit} == 1;
}

sub t_pop_autocommit {
    # Push two autocommits, get em off stack.
    my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');
    my $dbh = $inst->dbh;
    $inst->pushAutocommit(1);
    $inst->pushAutocommit(0);
    my ($p1, $p2);
    $p1 = $inst->popAutocommit;
    $p2 = $inst->popAutocommit;
    return ($p1 == 0 && $p2 == 1);
}

sub t_push_first {
    # Push before modc_getdbh; check DB handle OK.
    my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');
    $inst->pushAutocommit(1);
    my $dbh = $inst->dbh;
    my ($x) = $dbh->selectrow_array('select 1 from dual');
    return ($x == 1);
}

ok(t_push_autocommit(), 'push_autocommit');
ok(t_pop_autocommit(), 'pop_autocommit');
ok(t_push_first(), 'push_first');
