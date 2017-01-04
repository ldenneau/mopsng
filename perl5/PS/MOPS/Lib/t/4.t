use strict;
use warnings;
use Benchmark;
use Test::More tests => 7;
use PS::MOPS::Lib qw(:all);

sub test_histogram0 {
    my @a = qw( 1 2 3);
    my $h = mopslib_histogram(data => \@a, bins => 3);
    exists($h->{hist}) and exists($h->{bins}) and exists($h->{min}) and exists($h->{max});
}

sub test_histogram1 {
    my @a = qw( 1 2 3);
    my $h = mopslib_histogram(data => \@a, bins => 3);
    my $hist = $h->{hist};
    $hist->[0] == 1 and $hist->[1] == 1 and $hist->[2] == 1;
}

sub test_histogram2 {
    my @a = qw( -1 -2 -3 -1 -2 -3);
    my $h = mopslib_histogram(data => \@a, bins => 3);
    my $hist = $h->{hist};
    $hist->[0] == 2 and $hist->[1] == 2 and $hist->[2] == 2;
}

sub test_histogram3 {
    my @a = qw( -1 -1 -1 1 1 1);
    my $h = mopslib_histogram(data => \@a, bins => 3);
    my $hist = $h->{hist};
    $hist->[0] == 3 and $hist->[1] == 0 and $hist->[2] == 3;
}

sub test_histogram4 {
    my @a = qw( 42 42 42 42 );
    my $h = mopslib_histogram(data => \@a, bins => 3);
    my $hist = $h->{hist};
    $hist->[0] == 4;
}

sub test_histogram5 {
    my @a = qw( 42 43 44 );
    my $h = mopslib_histogram(data => \@a, bins => 3);
    $h->{binsize} == 1 and $h->{numpts} == 3;
}

sub test_histogram6 {
    my @a = qw( 42 43 44 99 98 );
    my $h = mopslib_histogram(data => \@a, bins => 3, hmax => 44);
    $h->{binsize} == 1 and $h->{numpts} == 5 and $h->{max} == 44;
}


my $test_module_name = 'PS-MOPS-Lib-4';
my $g_start = new Benchmark;
ok(test_histogram1, 'histogram0');
ok(test_histogram1, 'histogram1');
ok(test_histogram2, 'histogram2');
ok(test_histogram3, 'histogram3');
ok(test_histogram4, 'histogram4/single_value');
ok(test_histogram5, 'histogram5/binsize');
ok(test_histogram6, 'histogram6/hmax');
my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";

