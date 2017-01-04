use Test::More tests => 1;
use strict;
use warnings;

#########################

my @goo = `oid2obj --ref t/clean t/oids`;
chomp @goo;
my $ok = 
    (@goo and 
    $goo[0] eq 'S100o8d' and
    $goo[1] eq 'S0001b9' and
    $goo[2] eq 'S100esf' and
    $goo[3] eq 'S100kuU' and
    $goo[4] eq 'ENOTFOUND BOO');

ok($ok, 'test test');
