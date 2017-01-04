# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use Test::More tests => 4;
use PS::MOPS::Lib qw(:all);

#########################

sub test_dang1 {
    mopslib_dang(1, 5) == -4
}

sub test_dang2 {
    mopslib_dang(1, 5) == -4
    and mopslib_dang(5, 1) == 4
    and mopslib_dang(-3, -5) == 2
    and mopslib_dang(355, 10) == -15
    and mopslib_dang(10, 355) == 15
    and mopslib_dang(355, -10) == 5
    and mopslib_dang(-10, 355) == -5
    and mopslib_dang(359, -2) == 1
}

sub test_dang3 {
    mopslib_dang(-3, -5) == 2
    and mopslib_dang(355 + 360, 10) == -15
    and mopslib_dang(355 + 720, 10) == -15
    and mopslib_dang(10 - 360, 355) == 15
    and mopslib_dang(10 - 720, 355) == 15
    and mopslib_dang(355 + 360, -10 - 720) == 5
    and mopslib_dang(720, -360) == 0
}


sub test_dang4 {
    my ($a1, $a2) = (720, -360);
    mopslib_dang($a1, $a2) == 0 and $a1 == 720 and $a2 == -360;
}


ok(test_dang1, 'basic');
ok(test_dang2, '0-360 crossings');
ok(test_dang3, 'large angles');
ok(test_dang4, q{don't modify args});
