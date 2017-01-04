use strict;
use warnings;
use Benchmark;
use Test::More tests => 2;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);


my @DATA = <DATA>;  # slurp into global
chomp @DATA;

sub test_stdev1 {
    my ($avg, $std) = mopslib_stdev(@DATA);
    return 0 unless
        ($avg > 5.11 and $ avg < 5.12    # known distribution
        and $std > 2.01 and $std < 2.02);
    1;  # all OK
}

sub test_stdev2 {
    my ($avg, $std) = mopslib_stdev(());
    return 0 unless !defined($avg);
    1;  # all OK
}



my $test_module_name = 'PS-MOPS-Lib-8';
my $g_start = new Benchmark;
ok(test_stdev1, 'stdev');
ok(test_stdev2, 'no_input');
my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";

__DATA__
5.18624111
5.93692458
3.72809434
7.53505015
5.68798351
7.58152533
3.05535948
2.79011512
2.96297550
5.19544612
3.88439405
4.37114191
5.60729212
3.35427487
5.98561448
0.73155785
4.42999065
3.96602011
2.83267570
6.37182868
6.96597505
4.73289013
3.81204343
6.01553774
3.74446332
1.60926223
3.27690470
5.12534821
3.84402108
2.55460668
3.99818361
3.24492991
5.50851482
-0.30774021
6.37680376
5.44842082
4.24582863
6.56519711
7.48847127
5.91253668
3.89595652
8.35921979
1.53054833
5.00437645
3.75349855
6.12937808
6.59305584
6.94804358
5.27895260
7.63419557
6.41692126
6.41827857
4.90888577
3.80163991
4.37833369
5.76233083
7.41084671
2.99609017
7.48952556
7.78421021
9.58737040
3.60990894
9.73664522
4.99536759
2.49861145
5.63733447
4.26533055
3.91666257
8.60413027
3.68637300
8.31801629
7.80058742
5.34167224
3.11337149
1.56466079
5.86421120
7.06003642
6.84733260
3.14138508
5.61578590
7.16347528
4.57587940
6.15540147
5.75329351
2.83758926
6.87359881
2.61407995
2.09829569
4.80543342
7.14365625
6.51982021
3.33748198
8.28848696
6.89834130
8.42272115
6.05548954
3.68195546
5.25556272
6.94044244
1.08044934
