# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use strict;
use warnings;
use File::Basename;
use Test::More tests => 1;
use PS::MOPS::MITI;

#########################

my @raw_data;
my $line;
while (defined($line = <DATA>)) {
    push @raw_data, [ split /\s+/, $line ] unless $line =~ /^#/;
}


sub same {
    # Given two ARRAYREFs, return whether they're basically the same.  The
    # "reference" list should be the first one.
    my ($x, $y) = @_;
    my $rows;
    my $cols;

    $rows = scalar @$x;
    $cols = scalar @{$x->[0]};
    for my $i (0..$rows - 1) {
        for my $j (0..$cols - 1) {
            return 0 unless $x->[$i]->[$j] eq $y->[$i]->[$j];
        }
    }
    return 1;
}


sub test_miti_read {
    my ($dirname) = dirname($0);
    my $filename = "$dirname/test1.miti";
    my @stuff = miti_read($filename);
    return same(\@raw_data, \@stuff);
}

ok(test_miti_read, 'miti_read');

__DATA__
# ID    MJD RA  DEC MAG OBSCODE OBJECT_NAME
447307	53375.429174	103.087304	1.943655	24.498118	F51	St50U5a
447307	53375.430044	103.087182	1.943649	24.498119	F51	St50U5a
447307	53375.437644	103.086121	1.943592	24.498125	F51	St50U5a
447307	53375.441704	103.085554	1.943561	24.498128	F51	St50U5a
447309	53375.429174	104.516891	2.015167	23.507873	F51	S0004ug
447309	53375.430044	104.516553	2.01519	23.507861	F51	S0004ug
447309	53375.437644	104.513602	2.015386	23.50776	F51	S0004ug
447309	53375.441704	104.512025	2.01549	23.507706	F51	S0004ug
