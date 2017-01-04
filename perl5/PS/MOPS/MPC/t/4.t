use Test::More tests => 2;
BEGIN { use PS::MOPS::MPC ':all' };

#########################

use warnings;
use PS::MOPS::MITI;

my @stuff = <DATA>;         # slurp all from __DATA__
chomp @stuff;               # toss line endings
my @MPC = @stuff[0,1];      # first two lines are MPC
my @MITI = ($stuff[2],);    # next is MITI

sub test_mpc2miti {
    # Test that the MPC data in <DATA> can be converted to MITI.
    my %h = %{mpc_to_miti($MPC[0])};
    my %m = miti_parse($MITI[0]);

    foreach my $k (keys %m) {
        next if ($k eq 'OBJECT_NAME');  # skip; MPC doesn't have
        if ($k eq 'ID' or $k eq 'OBSCODE') {
            if ($m{$k} ne $h{$k}) {
                printf STDERR "%s: %s != %s\n", $k, $m{$k}, $h{$k};
                return 0;
            }
        }
        else {
            if (abs($m{$k} - $h{$k}) > .0001) {
                printf STDERR "%s: %s != %s\n", $k, $m{$k}, $h{$k};
                return 0;
            }
        }
    }

    return 1;   # looks good
}

sub test_mpc2miti_no_designation {
    # Test that the MPC data in <DATA> can be converted to MITI.
    my %h = %{mpc_to_miti($MPC[1])};

    return $h{ID} eq 'FOO';
}


ok(test_mpc2miti(), 'mpc2miti');
ok(test_mpc2miti_no_designation(), 'mpc2miti_no_designation');

__DATA__
     4591022  C2005 01 04.55848408 34 49.522+19 36 12.02         24.36V      500
FOO           C2005 01 04.55848408 34 49.522+19 36 12.02         24.36V      500
4591022 53374.5584836111        128.70634       19.603339       24.36           500
