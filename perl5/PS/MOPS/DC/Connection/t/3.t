#########################

use Test::More tests => 3;
use strict;
use warnings;
BEGIN { use PS::MOPS::DC::Connection };

#########################

sub t_push_autocommit {
    # Push autocommit, check that DBI driver sees it.
    my $dbh = modc_getdbh();
    modc_pushAutocommit(1);
    return $dbh->{AutoCommit} == 1;
}

sub t_pop_autocommit {
    # Push two autocommits, get em off stack.
    my $dbh = modc_getdbh();
    modc_pushAutocommit(1);
    modc_pushAutocommit(0);
    my ($p1, $p2);
    $p1 = modc_popAutocommit;
    $p2 = modc_popAutocommit;
    return ($p1 == 0 && $p2 == 1);
}

sub t_push_first {
    # Push before modc_getdbh; check DB handle OK.
    modc_pushAutocommit(1);
    my $dbh = modc_getdbh();
    my ($x) = $dbh->selectrow_array('select 1 from dual');
    return ($x == 1);
}

ok(t_push_autocommit(), 'push_autocommit');
ok(t_pop_autocommit(), 'pop_autocommit');
ok(t_push_first(), 'push_first');
