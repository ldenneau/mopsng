use Test::More tests => 1;
use strict;
use warnings;

#########################

my $GEN_EPHEM_EXE = "genEphem2";
my $GEN_EPHEM = "$ENV{MOPS_HOME}/bin/$GEN_EPHEM_EXE";
die "can't find $GEN_EPHEM_EXE" unless -e $GEN_EPHEM;

ok(1, 'test test');
