use Test::More tests => 1;
use strict;
use warnings;

#########################

my @goo = `mopsod t/1.dat`;
my $ok = 
    @goo == 2 && 
    $goo[0] =~ /^EINSUFFN/ &&
    $goo[1] !~ /^EINSUFFN/;

ok($ok, 'test test');
