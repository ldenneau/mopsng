#########################

use Test::More tests => 1;
BEGIN { use PS::MOPS::DC::Connection ':all' };

#########################

sub t_commit {
    modc_pushAutocommit(0);
    my $rc = modc_commit;
    modc_popAutocommit;
    $rc;
}

ok(t_commit(), 'commit');
