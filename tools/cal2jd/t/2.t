use Test::More tests => 1;
use strict;
use warnings;

#########################

my ($PROG) = 'cal2jd';


sub run {
    my $arglist = join " ", @_;
    my $res = `$PROG $arglist`;
    chomp $res;
    return $res;
}


sub buncha_dates {
   run(2005, 1, 18) == 2453388.5
   and run(1, 18, 2005) == 2453388.5
   and run('18-JAN-2005') == 2453388.5
   and run('2005-JAN-18') == 2453388.5
   and run('18-jaN-2005') == 2453388.5
   and run('2005-jAn-18') == 2453388.5
}

ok(buncha_dates, 'buncha_dates');
