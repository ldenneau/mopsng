use strict;
use warnings;
use Benchmark;

use Test::More tests => 3;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);

my $test_module_name = 'PS-MOPS-Lib-3';
my $g_start = new Benchmark;

my @lines = <DATA>;
chomp @lines;

ok(mopslib_formatTimingMsg( 
    subsystem => 'DTCTL', 
    nn => 127, 
    time_sec => 345.6,
) eq $lines[0]);

ok(mopslib_formatTimingMsg( 
    subsystem => 'DTCTL', 
    subsubsystem => 'FOO', 
    nn => 127, 
    time_sec => 345.6, 
    comment => 'comment',
) eq $lines[1]);

ok(mopslib_formatTimingMsg( 
    subsystem => 'DTCTL', 
    nn => 0, 
    time_sec => 123.4,
) eq $lines[2]);

my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";

# name q_au e i_deg node_deg argperi_deg timeperi_mjd epoch_mjd
__DATA__
TIMING DTCTL 345.600 127
TIMING DTCTL/FOO 345.600 127 # comment
TIMING DTCTL 123.400 0
