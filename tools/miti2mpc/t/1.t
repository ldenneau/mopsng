use Test::More tests => 1;
use strict;
use warnings;

use File::Temp qw(tempdir);

#########################

my $happy = 1;
my @data = <DATA>;  # slurp all DATA
my @miti = @data[0..9];    # grab first 10 lines
my @mpc = @data[10..18];    # grab next 9 lines

# Need to execute script, write out to file, compare.
my $tmpdir = tempdir('/tmp/tmiti2mpcXXXXXX', CLEANUP => 1); # new tmp dir
my $tmpfile = "$tmpdir/test.miti";
open my $miti_fh, ">$tmpfile" or die "can't create $tmpfile";
print $miti_fh @miti;   # write MITI data
close $miti_fh;

open my $prog, "miti2mpc < $tmpfile |" or die "can't open miti2mpc";
my @newmpc = <$prog>;   # read MPC output
close $prog;

# Now compare MPC output to expected output.
$happy = 0 unless (
    @mpc == @newmpc     # same number of elements in each array
    && !grep { $mpc[$_] ne $newmpc[$_] } (0..$#mpc)   # unequal elements (should be none)
);

ok($happy, 'miti2mpc');

__DATA__
#DetId	Epoch_MJD	RA_deg	DEC_deg	Mag	ObsCode	ObjectName
2921403	53401.2906736111	39.250402	11.637122	23.44604	568	St702hb
2921390	53401.2906736111	37.157266	12.262515	24.286354	568	St801La
2921389	53401.2906736111	37.40778	12.165446	22.252198	568	St606Z6
2921388	53401.2906736111	38.148691	11.94938	24.0721	568	St605xy
2921387	53401.2906736111	39.124533	12.460783	24.24455	568	St608dr
2921386	53401.2906736111	37.435966	11.150738	23.725868	568	2001SO73
2921385	53401.2906736111	37.52246	12.54094	23.567339	568	S1000fB
2921384	53401.2906736111	38.882003	10.228817	23.451574	568	St701g5
2921383	53401.2906736111	38.863945	9.957858	24.10368	568	St60a8O
     2921403  C2005 01 31.29067402 37 00.096+11 38 13.64         23.45V      568
     2921390  C2005 01 31.29067402 28 37.744+12 15 45.05         24.29V      568
     2921389  C2005 01 31.29067402 29 37.867+12 09 55.61         22.25V      568
     2921388  C2005 01 31.29067402 32 35.686+11 56 57.77         24.07V      568
     2921387  C2005 01 31.29067402 36 29.888+12 27 38.82         24.24V      568
     2921386  C2005 01 31.29067402 29 44.632+11 09 02.66         23.73V      568
     2921385  C2005 01 31.29067402 30 05.390+12 32 27.38         23.57V      568
     2921384  C2005 01 31.29067402 35 31.681+10 13 43.74         23.45V      568
     2921383  C2005 01 31.29067402 35 27.347+09 57 28.29         24.10V      568
