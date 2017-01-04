#########################

use Test::More tests => 1;
BEGIN { use PS::MOPS::DC::Instance };

#########################

sub t_commit {
    my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');
    $inst->pushAutocommit(0);
    my $rc = modc_commit;
    $inst->popAutocommit;
    $rc;
}

ok(t_commit(), 'commit');
