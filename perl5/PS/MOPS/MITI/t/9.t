# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use strict;
use warnings;
use Test::More tests => 1;
use PS::MOPS::MITI;

#########################

sub test_miti_parse {
    my $line;
    my %m;

    $line = <DATA>;
    %m = miti_parse($line);      # convert to MITI hash
    return 0 unless
        $m{ID} eq 'A'
        and $m{EPOCH_MJD} == 50000
        and $m{RA_DEG} == 10
        and $m{DEC_DEG} == -5
        and $m{MAG} == 15.0
        and $m{OBSCODE} eq '128'
        and !$m{OBJECT_NAME};

    $line = <DATA>;
    %m = miti_parse($line);      # convert to MITI hash
    return 0 unless
        $m{ID} eq 'B'
        and $m{EPOCH_MJD} == 51111
        and $m{RA_DEG} == 234
        and $m{DEC_DEG} == 10
        and $m{MAG} == 15.2
        and $m{OBSCODE} eq 'F51'
        and $m{OBJECT_NAME} eq 'Chuck';

    1;
}

ok(test_miti_parse, 'miti_parse');

__DATA__
A   50000   10  -5  15.0    128
B   51111   234     10  15.2    F51 Chuck
