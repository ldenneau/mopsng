use Test::More tests => 2;
use strict;
use warnings;

#########################

my $PROG = 'listMJDs';

sub test_mjds {
    my @out = `$PROG --mjd 53377 53378`; 
    chomp @out;
    my $str = join ' ', @out;
    return $str eq '53377 53378';
}

sub test_mjds_coarse {
    my @out = `$PROG --coarse --mjd 53377 53378`; 
    chomp @out;
    my $str = join ' ', @out;
    return $str eq '53376 53377 53378 53379';
}

ok(test_mjds, 'test_mjds');
ok(test_mjds_coarse, 'test_mjds_coarse');
