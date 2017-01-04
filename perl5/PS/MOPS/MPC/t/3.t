use Test::More tests => 1;
BEGIN { use PS::MOPS::MPC ':all' };

#########################

use warnings;

my @stuff = <DATA>;         # slurp all from __DATA__
chomp @stuff;               # toss line endings
my @MPC = @stuff[0..5];     # first six lines are MPC
my @MITI = @stuff[6..11];   # next six are MITI equiv

sub test_miti {
    # Test that the MITI data in <DATA> is written correctly in MPC.
    for my $i (0..3) {
        my @things = split /\s+/, $MITI[$i];    # grab separate MITI fields
        my %stuff;
        @stuff{qw(ID EPOCH_MJD RA_DEG DEC_DEG MAG OBSCODE)} = @things;  # assign em
        if (mpc_format_miti(%stuff) ne $MPC[$i]) { 
            print STDERR "Differ:\n", mpc_format_miti(%stuff), "\n", $MPC[$i], "\n";
#            return 0;
        }
    }
    return 1;   # looks good
}


ok(test_miti(), 'miti');

__DATA__
     4591022  C2005 01 04.55848 08 34 49.522+19 36 12.02         24.4 V      500
     4591009  C2005 01 04.55848 08 37 51.343+19 08 18.39         23.4 V      500
     4591008  C2005 01 04.55848 08 46 16.377+19 15 30.23         23.8 V      500
     4591007  C2005 01 04.55848 08 41 05.207+20 09 25.93         21.2 V      500
     4591022  C2005 01 04.55848 23 58 49.522+19 36 12.02         24.4 V      500
     4591022  C2005 01 04.55848 23 58 49.522+19 36 12.02         24.4 V      500
4591022 53374.5584836111        128.70634       19.603339       24.361331       500     SC00bWF
4591009 53374.5584836111        129.463929      19.138443       23.444187       500     St511B2
4591008 53374.5584836111        131.568236      19.258396       23.767602       500     St50RM8
4591007 53374.5584836111        130.271697      20.157203       21.228109       500     St50SQT
4591022 53374.5584836111        359.70634       19.603339       24.361331       500     SC00bWF
4591022 53374.5584836111         -0.23966       19.603339       24.361331       500     SC00bWF

