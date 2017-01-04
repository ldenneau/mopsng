use strict;
use warnings;
use Benchmark;

use Test::More tests => 2;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);
use PS::MOPS::DC::Orbit;
use File::Temp;

my $test_module_name = 'PS-MOPS-Lib-3';
my $g_start = new Benchmark;


my $args = {
    ZAG => 'zag',
    ZIG => 'buz',
    ZORG => 'quux',
};

my @data = <DATA>;


sub test_parse1 {
    # Make a temp file from the contents of __DATA__.  Then we'll read it in.
    my ($fh, $filename) = File::Temp::tempfile(UNLINK => 1);
    print $fh @data;
    $fh->close();

    my $section = 'FOO';
    my $stuff = mopslib_parseOrbfitOptsFile($filename, $section, $args);
    my $answer = <<"EOF";
here
is zag
some stuff
EOF

    return 1 if $answer eq $stuff;
}

sub test_parse2 {
    # Make a temp file from the contents of __DATA__.  Then we'll read it in.
    my ($fh, $filename) = File::Temp::tempfile(UNLINK => 1);
    print $fh @data;
    $fh->close();

    my $section = 'BAR';
    my $stuff = mopslib_parseOrbfitOptsFile($filename, $section, $args);
    my $answer = <<"EOF";
there
goes buz quux
some stuff
EOF

    return 1 if $answer eq $stuff;
}


ok(test_parse1);
ok(test_parse2);

my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";



__DATA__
!!FOO
here
is $ZAG
some stuff
!!BAR
there
goes $ZIG $ZORG
some stuff
