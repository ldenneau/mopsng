# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use strict;
use warnings;
use Test::More tests => 7;
use PS::MOPS::Ephem;

my $EPSILON = .0000001;         # threshold for close FP values

# a-priori test results.  We expect these back.  Build from __DATA__.
my $APR;
my ($obj, $epoch, $ra, $dec, $mag);
my $line;
while (defined($line = <DATA>)) {
    chomp $line;
    $line =~ s/^.*://;   # chuck leading MJD
    ($obj, $epoch, $ra, $dec, $mag) = split /\s+/, $line;
    $APR->{$obj}->{$epoch} = {
        ra_deg => $ra,
        dec_deg => $dec,
        mag => $mag,
    };
}

my $DEFAULTOBJ = 'S000001';
my $DEFAULTEPOCH = 53373.416666;
my %S000001_ORB = (
    q_AU => 1.252068,
    e => 0.382,
    i_deg => 9.3,
    node_deg => 252.1,
    argPeri_deg => 185.6,
    timePeri_MJD => 53015.0711320161,
    hV => 10.315,
    epoch_MJD => 52860,
##    objectName => 'S000001',
    objectName => 'Bob',
);
my %St51aLn_ORB = (
    q_AU => 4.874238183606,
    e => 0.066694,
    i_deg => 20.4055,
    node_deg => 265.612103,
    argPeri_deg => 81.186754,
    timePeri_MJD => 51791.9370310229,
    hV => 16.21,
    epoch_MJD => 53400,
##    objectName => 'St51aLn',
    objectName => 'Ted',
);
my %S100dT8_ORB = (
    q_AU => 2.75382081036,
    e => 0.131996,
    i_deg => 2.164653,
    node_deg => 70.458996,
    argPeri_deg => 211.183789,
    timePeri_MJD => 51837.7319141864,
    hV => 16.251,
    epoch_MJD => 52860,
##    objectName => 'S100dT8',
    objectName => 'Alice',
);
my %ORBITS = (
    S000001 => \%S000001_ORB,
    St51aLn => \%St51aLn_ORB,
    S100dT8 => \%S100dT8_ORB,
);
my @EPOCHS = (
    53373.416666,
    53374.416666,
    53375.416666,
);
# Want to use nonexistent names so we can't accidentally look up a real object.
$APR->{Bob} = $APR->{S000001};
$APR->{Ted} = $APR->{St51aLn};
$APR->{Alice} = $APR->{S100dT8};


sub _eq {
    # Return whether two floating-pt values are 'about the same'
    my ($x, $y) = @_;
    return 1 if $x == $y;
    return abs($x - $y) / abs($x + $y) < $EPSILON;
}


sub _got_goodresult {
    # Compare two hashrefs, check that results are "same".
    my ($a, $b) = @_;
    return 
        _eq($a->{ra_deg}, $b->{ra_deg})
        and _eq($a->{dec_deg}, $b->{dec_deg})
        and _eq($a->{mag}, $b->{mag});
}


sub sobjse {
    my $s = mopsephem_calcEphemerides(
        objectName => $DEFAULTOBJ,
        ephemEpoch_MJD => $DEFAULTEPOCH,
    );  # scalar version

    my %a = mopsephem_calcEphemerides(
        objectName => $DEFAULTOBJ,
        ephemEpoch_MJD => $DEFAULTEPOCH,
    );  # list version

    return 
        _got_goodresult($APR->{$DEFAULTOBJ}->{$DEFAULTEPOCH}, $s) 
        and _got_goodresult($APR->{$DEFAULTOBJ}->{$DEFAULTEPOCH}, \%a);
}


sub sorbse {
    my $s = mopsephem_calcEphemerides(
        %S000001_ORB,
        ephemEpoch_MJD => $DEFAULTEPOCH,
    );  # scalar version

    my %a = mopsephem_calcEphemerides(
        %S000001_ORB,
        ephemEpoch_MJD => $DEFAULTEPOCH,
    );  # list version

    return 
        _got_goodresult($APR->{$DEFAULTOBJ}->{$DEFAULTEPOCH}, $s) 
        and _got_goodresult($APR->{$DEFAULTOBJ}->{$DEFAULTEPOCH}, \%a);
}


sub mobjse {
    my $s = mopsephem_calcEphemerides(
        objectName => [ keys %ORBITS ],
        ephemEpoch_MJD => $DEFAULTEPOCH,
    );  # scalar version
    foreach my $k (keys %{$s}) {
        return 0 unless _got_goodresult($APR->{$k}->{$DEFAULTEPOCH}, $s->{$k});
    }

    my %a = mopsephem_calcEphemerides(
        objectName => [ keys %ORBITS ],
        ephemEpoch_MJD => $DEFAULTEPOCH,
    );  # list version
    foreach my $k (keys %{$s}) {
        return 0 unless _got_goodresult($APR->{$k}->{$DEFAULTEPOCH}, $a{$k});
    }

    return 1;
}


sub morbse {
    my $s = mopsephem_calcEphemerides(
        orbits => [ values %ORBITS ],
        ephemEpoch_MJD => $DEFAULTEPOCH,
    );  # scalar version
    foreach my $k (keys %{$s}) {
        return 0 unless _got_goodresult($APR->{$k}->{$DEFAULTEPOCH}, $s->{$k});
    }

    my %a = mopsephem_calcEphemerides(
        orbits => [ values %ORBITS ],
        ephemEpoch_MJD => $DEFAULTEPOCH,
    );  # list version
    foreach my $k (keys %{$s}) {
        return 0 unless _got_goodresult($APR->{$k}->{$DEFAULTEPOCH}, $a{$k});
    }

    return 1;
}



sub sobjme {
    my $i;
    my $s = mopsephem_calcEphemerides(
        objectName => $DEFAULTOBJ,
        ephemEpoch_MJD => \@EPOCHS,
    );  # scalar version
    $i = 0;
    foreach my $r (@{$s}) {
        return 0 unless _got_goodresult($APR->{$DEFAULTOBJ}->{$EPOCHS[$i]}, $r);
        $i++;
    }

    my @a = mopsephem_calcEphemerides(
        objectName => $DEFAULTOBJ,
        ephemEpoch_MJD => \@EPOCHS,
    );  # list version
    $i = 0;
    foreach my $r (@a) {
        return 0 unless _got_goodresult($APR->{$DEFAULTOBJ}->{$EPOCHS[$i]}, $r);
        $i++;
    }

    return 1;
}


sub sorbme {
    my $i;
    my $s = mopsephem_calcEphemerides(
        %S000001_ORB,
        ephemEpoch_MJD => \@EPOCHS,
    );  # scalar version
    $i = 0;
    foreach my $r (@{$s}) {
        return 0 unless _got_goodresult($APR->{'S000001'}->{$EPOCHS[$i]}, $r);
        $i++;
    }

    my @a = mopsephem_calcEphemerides(
        %S000001_ORB,
        ephemEpoch_MJD => \@EPOCHS,
    );  # list version
    $i = 0;
    foreach my $r (@a) {
        return 0 unless _got_goodresult($APR->{'S000001'}->{$EPOCHS[$i]}, $r);
        $i++;
    }

    return 1;
}



ok(!aa() and _eq(0, 0) and !_eq(0, 1) and _eq(1, 1) and _eq(.5, .499999999), 'close values');
ok(sobjse, 'single object single epoch');
ok(sorbse, 'single orbit single epoch');
ok(mobjse, 'multi object single epoch');
ok(morbse, 'multi orbit single epoch');
ok(sobjme, 'single object multiple epoch');
ok(sorbme, 'single orbit multiple epoch');


__DATA__
53373:S000001 53373.416666 240.194656 -23.630465 15.760725
53374:S000001 53374.416666 240.593014 -23.689687 15.764535
53375:S000001 53375.416666 240.989831 -23.747691 15.768154
53373:St51aLn 53373.416666 123.913574 4.761012 23.589968
53374:St51aLn 53374.416666 123.788000 4.744952 23.579557
53375:St51aLn 53375.416666 123.661003 4.730031 23.569276
53373:S100dT8 53373.416666 192.193383 -2.833041 22.171875
53374:S100dT8 53374.416666 192.350916 -2.889797 22.159943
53375:S100dT8 53375.416666 192.504932 -2.944948 22.147824
