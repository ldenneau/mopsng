# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use strict;
use warnings;
use Test::More tests => 2;
use File::Temp qw(tmpnam);
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
    for my $i (0..$rows - 1) {
        $cols = scalar @{$x->[$i]};
        for my $j (0..$cols - 1) {
            return 0 unless $x->[$i]->[$j] eq $y->[$i]->[$j];
        }
    }
    return 1;
}


sub test_miti_write_array {
    my $tempfilename = tmpnam('/tmp', 'mitiXXXXX');
    my $same = 0;
    my @stuff;
    eval {
        miti_write($tempfilename, @raw_data);
        @stuff = miti_read($tempfilename);
        $same = same(\@raw_data, \@stuff);
    };
    warn $@ if $@;
    unlink $tempfilename;
    return $same;
}


sub test_miti_write_hash {
    my $tempfilename = tmpnam('/tmp', 'mitiXXXXX');
    my $same = 0;
    my @hashy_data;
    my @stuff;

    # Build hashy data.
    foreach my $item (@raw_data) {
        push @hashy_data, {
            ID => $item->[0],
            EPOCH_MJD => $item->[1],
            RA_DEG => $item->[2],
            DEC_DEG => $item->[3],
            MAG => $item->[4],
            OBSCODE => $item->[5],
            OBJECT_NAME => $item->[6],
        };
    }

    eval {
        miti_write($tempfilename, @hashy_data);
        @stuff = miti_read($tempfilename);
        $same = same(\@raw_data, \@stuff);
    };
    warn $@ if $@;
    unlink $tempfilename;
    return $same;
}

ok(test_miti_write_array, 'miti_write_array');
ok(test_miti_write_hash, 'miti_write_hash');

__DATA__
582148	53387.317804	96.047626	0.20647	23.673619	F51
582148	53387.326344	96.046643	0.206862	23.673685	F51
582149	53387.317804	96.22955	1.773013	23.569852	F51
582149	53387.326344	96.228475	1.773146	23.569929	F51
582150	53387.317804	96.161979	1.027589	23.966325	F51
582150	53387.326344	96.160935	1.027728	23.966395	F51
582172	53387.315604	95.296442	-3.41411	23.848057	F51
582172	53387.324004	95.295449	-3.41381	23.84812	F51
582175	53387.315604	95.553389	-3.077711	23.967769	F51 FOO
582175	53387.324004	95.552372	-3.077196	23.96783	F51 FOO
