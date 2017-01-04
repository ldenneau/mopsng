use strict;
use warnings;
use Benchmark;
use Test::More tests => 34;

use Test::MockObject;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);

my $test_module_name = 'PS-MOPS-Lib-assembleTTITuples';
my $g_start = new Benchmark;


sub make_field {
    # Given a line for our DATA section, make a mock field.
    my ($line) = @_;
    return undef unless $line;

    my @stuff = split /\s+/, $line;

    # $line is TEST_ID FIELD_ID RA_DEG DEC_DEG EPOCH_MJD 
#    my $field = Test::MockObject->new();
#    $field->mock('testID', sub { $stuff[0] });
#    $field->mock('fieldId', sub { $stuff[1] });
#    $field->mock('ra', sub { $stuff[2] });
#    $field->mock('dec', sub { $stuff[3] });
#    $field->mock('epoch', sub { $stuff[4] });
#    $field->mock('filter', sub { $stuff[5] });

    my $field = {
        testId => $stuff[0],
        fieldId => $stuff[1],
        ra => $stuff[2],
        dec => $stuff[3],
        epoch => $stuff[4],
        filter => $stuff[5],
    };

    return $field;
}


# Test various tuple assemblage scenarios.  We want to test:
# 1. Basic organization into pairs
# 2. Extended tuples (triples, fourples)
# 3. Correct labeling of orphans
#   - singles
#   - tuple fields separated "too much"
# 4. Identification of deep stacks

# Get all our fields
my %test_fields;
my $field;
my $line;
while (defined($line = <DATA>)) {
    if ($line !~ /^\s*$/) {
        $field = make_field($line);
        push @{$test_fields{$field->{testId}}}, $field;
    };
}

my $res;            # assembleTTITuples result
my %args;

# Test 1: Basic organization in to pairs.
%args = (
    fields => $test_fields{A},
    min_fields => 2,
    max_fields => 2,
    min_tti_min => 10,
    max_tti_min => 30,
);
$res = mopslib_assembleTTITuples(%args);
ok(keys %{$res->{TTI_TUPLES}} == 3, 'pairs_num_tuples');
ok(scalar @{$res->{TTI_TUPLES}->{11}} == 2, 'pairs_num_fields');
ok(keys %{$res->{DEEP_STACKS}} == 0, 'pairs_no_deep_stacks');
ok(scalar @{$res->{ORPHANS}} == 0, 'pairs_no_orphans');


# Test 2: Extended tuples (triples).
%args = (
    fields => $test_fields{B},
    min_fields => 2,
    max_fields => 3,
    min_tti_min => 10,
    max_tti_min => 30,
);
$res = mopslib_assembleTTITuples(%args);
ok(keys %{$res->{TTI_TUPLES}} == 2, 'triples_num_tuples');
ok(scalar @{$res->{TTI_TUPLES}->{3}} == 3, 'triples_num_fields');
ok(keys %{$res->{DEEP_STACKS}} == 0, 'triples_no_deep_stacks');
ok(scalar @{$res->{ORPHANS}} == 0, 'triples_no_orphans');


# Test 3: Finding orphans.
%args = (
    fields => $test_fields{C},
    min_fields => 3,
    max_fields => 3,
    min_tti_min => 10,
    max_tti_min => 30,
);
$res = mopslib_assembleTTITuples(%args);
ok(keys %{$res->{TTI_TUPLES}} == 2, 'orphans_tuples');
ok(scalar @{$res->{TTI_TUPLES}->{3}} == 3, 'orphans_fields');
ok(keys %{$res->{DEEP_STACKS}} == 0, 'orphans_stacks');
ok(scalar @{$res->{ORPHANS}} == 2, 'orphans_num_orphans');
ok($res->{ORPHANS}->[0]->{fieldId} == 20, 'orphans_orphan_1');
ok($res->{ORPHANS}->[1]->{fieldId} == 4, 'orphans_orphan_2');


# Test 4: Finding multiple tuples at same pointing.
%args = (
    fields => $test_fields{D},
    min_fields => 3,
    max_fields => 3,
    min_tti_min => 10,
    max_tti_min => 30,
);
$res = mopslib_assembleTTITuples(%args);
ok(keys %{$res->{TTI_TUPLES}} == 3, 'orphans_tuples');
ok(scalar @{$res->{TTI_TUPLES}->{3}} == 3, 'orphans_tuple1');
ok(scalar @{$res->{TTI_TUPLES}->{6}} == 3, 'orphans_tuple2');
ok(scalar @{$res->{TTI_TUPLES}->{13}} == 3, 'orphans_tuple3');
ok(keys %{$res->{DEEP_STACKS}} == 0, 'orphans_stacks');
ok(scalar @{$res->{ORPHANS}} == 1, 'orphans_num_orphans');
ok($res->{ORPHANS}->[0]->{fieldId} == 20, 'orphans_orphan_1');


# Test 5: Deep stacks.
%args = (
    fields => $test_fields{E},
    min_fields => 3,
    max_fields => 6,
    min_tti_min => 10,
    max_tti_min => 30,
);
$res = mopslib_assembleTTITuples(%args);
ok(keys %{$res->{TTI_TUPLES}} == 3, 'deep_tuples');
ok(scalar @{$res->{TTI_TUPLES}->{3}} == 3, 'deep_tuple1');
ok(scalar @{$res->{TTI_TUPLES}->{6}} == 3, 'deep_tuple2');
ok(scalar @{$res->{TTI_TUPLES}->{14}} == 4, 'deep_tuple3');
ok(keys %{$res->{DEEP_STACKS}} == 1, 'deep_stacks');
ok(scalar @{$res->{DEEP_STACKS}->{35}} == 6, 'deep_stack1');
ok(scalar @{$res->{ORPHANS}} == 1, 'deep_num_orphans');
ok($res->{ORPHANS}->[0]->{fieldId} == 20, 'deep_orphan_1');


# Test 6: max_tti_min
%args = (
    fields => $test_fields{A},
    min_fields => 2,
    max_fields => 4,
    min_tti_min => 0,
    max_tti_min => 30,
);
$res = mopslib_assembleTTITuples(%args);
ok(keys %{$res->{TTI_TUPLES}} == 3, 'pairs_num_tuples');
ok(keys %{$res->{DEEP_STACKS}} == 0, 'pairs_no_deep_stacks');
ok(scalar @{$res->{ORPHANS}} == 0, 'pairs_no_orphans');

# Test 7: any_filter=1
%args = (
    fields => $test_fields{F},
    min_fields => 2,
    max_fields => 4,
    min_tti_min => 0,
    max_tti_min => 30,
    any_filter => 1
);
$res = mopslib_assembleTTITuples(%args);
ok(keys %{$res->{TTI_TUPLES}} == 1, 'any_filter=1');

# Test 7: any_filter=0
%args = (
    fields => $test_fields{F},
    min_fields => 2,
    max_fields => 4,
    min_tti_min => 0,
    max_tti_min => 30,
    any_filter => 0
);
$res = mopslib_assembleTTITuples(%args);
ok(keys %{$res->{TTI_TUPLES}} == 2, 'any_filter=0');


my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";

# TEST_ID FIELD_ID RA_DEG DEC_DEG EPOCH_MJD FILTER
__DATA__
A   1   10.0    10.0    54340.000 r
A   2   12.0    12.0    54340.001 r
A   3   14.0    14.0    54340.002 r
A   11  10.0    10.0    54340.010 r
A   12  12.0    12.0    54340.011 r
A   13  14.0    14.0    54340.012 r

B   1   10.0    10.0    54340.000 r
B   2   10.0    10.0    54340.010 r
B   3   10.0    10.0    54340.020 r
B   11  12.0    12.0    54340.002 r
B   12  12.0    12.0    54340.012 r
B   13  12.0    12.0    54340.022 r

C   1   10.0    10.0    54340.000 r
C   2   10.0    10.0    54340.010 r
C   3   10.0    10.0    54340.020 r
C   4   10.0    10.0    54340.320 r
C   11  12.0    12.0    54340.002 r
C   12  12.0    12.0    54340.012 r
C   13  12.0    12.0    54340.022 r
C   20  16.0    16.0    54340.222 r

D   1   10.0    10.0    54340.000 r
D   2   10.0    10.0    54340.010 r
D   3   10.0    10.0    54340.020 r
D   4   10.0    10.0    54340.320 r
D   5   10.0    10.0    54340.330 r
D   6   10.0    10.0    54340.340 r
D   11  12.0    12.0    54340.002 r
D   12  12.0    12.0    54340.012 r
D   13  12.0    12.0    54340.022 r
D   20  16.0    16.0    54340.222 r

E   1   10.0    10.0    54340.000 r
E   2   10.0    10.0    54340.010 r
E   3   10.0    10.0    54340.020 r
E   4   10.0    10.0    54340.320 r
E   5   10.0    10.0    54340.330 r
E   6   10.0    10.0    54340.340 r
E   11  12.0    12.0    54340.002 r
E   12  12.0    12.0    54340.012 r
E   13  12.0    12.0    54340.022 r
E   14  12.0    12.0    54340.032 r
E   20  16.0    16.0    54340.222 r
E   30  0.0     0.0     54340.400 r
E   31  0.0     0.0     54340.401 r
E   32  0.0     0.0     54340.402 r
E   33  0.0     0.0     54340.403 r
E   34  0.0     0.0     54340.404 r
E   35  0.0     0.0     54340.405 r

F   1   10.0    10.0    54340.000 r
F   2   10.0    10.0    54340.010 r
F   3   10.0    10.0    54340.020 i
F   3   10.0    10.0    54340.030 i
