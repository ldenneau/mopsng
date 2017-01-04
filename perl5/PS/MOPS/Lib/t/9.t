use strict;
use warnings;
use Benchmark;
use Test::More tests => 19 + 8;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);


my $epsilon = 1e-10;


sub _chk1 {
    my ($a, $b) = @_; 
    return (abs($a - $b) < $epsilon);
}


sub _chk2 {
    my ($ra, $dec, $tra, $tdec) = @_; 
    return (abs($ra - $tra) < $epsilon && abs($dec - $tdec) < $epsilon);
}


sub test_rnd {
    my ($ok) = 1;
    my ($ra, $dec);

    ($ra, $dec) = mopslib_rndGaussian(0, 0);
    ($ra, $dec) = mopslib_rndGaussian(1, 1);

    $ok;
}

my $test_module_name = 'PS-MOPS-Lib-9';
my $g_start = new Benchmark;
ok(_chk1(PS::MOPS::Lib::_fmod(1, 1), 0),"_chk1(PS::MOPS::Lib::_fmod(1, 1), 0)");               # 1 + 0 = 1
ok(_chk1(PS::MOPS::Lib::_fmod(5, 3), 2),"_chk1(PS::MOPS::Lib::_fmod(5, 3), 2)");               # 3 + 2 = 5
ok(_chk1(PS::MOPS::Lib::_fmod(5.3, 3), 2.3),"_chk1(PS::MOPS::Lib::_fmod(5.3, 3), 2.3)");           # 3 + 2.3 = 5.3
ok(_chk1(PS::MOPS::Lib::_fmod(371.5, 360), 11.5),"_chk1(PS::MOPS::Lib::_fmod(371.5, 360), 11.5)");      # 360 + 11.5 = 371.5
ok(_chk1(PS::MOPS::Lib::_fmod(-1, 1), 0),"_chk1(PS::MOPS::Lib::_fmod(-1, 1), 0)");              # -1 + 1 = 0
ok(_chk1(PS::MOPS::Lib::_fmod(-5, 3), 1),"_chk1(PS::MOPS::Lib::_fmod(-5, 3), 1)");              # -6 + 1 = -5
ok(_chk1(PS::MOPS::Lib::_fmod(-5.3, 3), 0.7),"_chk1(PS::MOPS::Lib::_fmod(-5.3, 3), 0.7)");          # -6 + 0.7 = -5.3
ok(_chk1(PS::MOPS::Lib::_fmod(-371.5, 360), 348.5),"_chk1(PS::MOPS::Lib::_fmod(-371.5, 360), 348.5)");    # -720 + 348.5 = -371.5

ok(_chk2(mopslib_normalizeRADEC(0.1, 0.1), 0.1, 0.1),"_chk2(mopslib_normalizeRADEC(0.1, 0.1), 0.1, 0.1)");
ok(_chk2(mopslib_normalizeRADEC(90.1, 0.1), 90.1, 0.1),"_chk2(mopslib_normalizeRADEC(90.1, 0.1), 90.1, 0.1)");
ok(_chk2(mopslib_normalizeRADEC(180.1, 0.1), 180.1, 0.1),"_chk2(mopslib_normalizeRADEC(180.1, 0.1), 180.1, 0.1)");
ok(_chk2(mopslib_normalizeRADEC(270.1, 0.1), 270.1, 0.1),"_chk2(mopslib_normalizeRADEC(270.1, 0.1), 270.1, 0.1)");
ok(_chk2(mopslib_normalizeRADEC(360.1, 0.1), 0.1, 0.1),"_chk2(mopslib_normalizeRADEC(360.1, 0.1), 0.1, 0.1)");
ok(_chk2(mopslib_normalizeRADEC(50.1, 45), 50.1, 45),"_chk2(mopslib_normalizeRADEC(50.1, 45), 50.1, 45)");
ok(_chk2(mopslib_normalizeRADEC(360 + 50.1, 45), 50.1, 45),"_chk2(mopslib_normalizeRADEC(360 + 50.1, 45), 50.1, 45)");
ok(_chk2(mopslib_normalizeRADEC(720 + 50.1, 45), 50.1, 45),"_chk2(mopslib_normalizeRADEC(720 + 50.1, 45), 50.1, 45)");
ok(_chk2(mopslib_normalizeRADEC(-360 + 50.1, 45), 50.1, 45),"_chk2(mopslib_normalizeRADEC(-360 + 50.1, 45), 50.1, 45)");
ok(_chk2(mopslib_normalizeRADEC(-720 + 50.1, 45), 50.1, 45),"_chk2(mopslib_normalizeRADEC(-720 + 50.1, 45), 50.1, 45)");
ok(_chk2(mopslib_normalizeRADEC(50.1, 360 + 45), 50.1, 45),"_chk2(mopslib_normalizeRADEC(50.1, 360 + 45), 50.1, 45)");
ok(_chk2(mopslib_normalizeRADEC(50.1, 720 + 45), 50.1, 45),"_chk2(mopslib_normalizeRADEC(50.1, 720 + 45), 50.1, 45)");
ok(_chk2(mopslib_normalizeRADEC(50.1, -360 + 45), 50.1, 45),"_chk2(mopslib_normalizeRADEC(50.1, -360 + 45), 50.1, 45)");
ok(_chk2(mopslib_normalizeRADEC(50.1, -720 + 45), 50.1, 45),"_chk2(mopslib_normalizeRADEC(50.1, -720 + 45), 50.1, 45)");
ok(_chk2(mopslib_normalizeRADEC(50.1, 90 + 45), 180 + 50.1, 45),"_chk2(mopslib_normalizeRADEC(50.1, 90 + 45), 180 + 50.1, 45)");
ok(_chk2(mopslib_normalizeRADEC(50.1, -45), 50.1, -45),"_chk2(mopslib_normalizeRADEC(50.1, -45), 50.1, -45)");
ok(_chk2(mopslib_normalizeRADEC(50.1, -90 - 45), 180 + 50.1, -45),"_chk2(mopslib_normalizeRADEC(50.1, -90 - 45), 180 + 50.1, -45)");
ok(_chk2(mopslib_normalizeRADEC(50.1, 90 + 45), 180 + 50.1, 45),"_chk2(mopslib_normalizeRADEC(50.1, 90 + 45), 180 + 50.1, 45)");
my $g_end = new Benchmark;
print "time: ${test_module_name}: " . timestr(timediff($g_end, $g_start), 'all') . "\n";

ok(test_rnd, 'rnd');
