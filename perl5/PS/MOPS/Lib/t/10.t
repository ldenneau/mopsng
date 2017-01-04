use strict;
use warnings;
use Test::More tests => 1;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);


sub test_getocnum {
    return mopslib_getOCNum(53373) == 61;
}

ok(test_getocnum, 'getocnum');
