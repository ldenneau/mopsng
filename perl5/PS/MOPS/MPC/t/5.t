use Test::More tests => 12;
use Astro::SLA;
BEGIN { use PS::MOPS::MPC ':all' };

#########################

use warnings;

my @stuff = <DATA>;         # slurp all from __DATA__
chomp @stuff;               # toss line endings
my $foo = mpcorb_to_obj($stuff[0]);

ok($foo->{desig} eq '00001', 'desig');
ok($foo->{H} == 3.34, 'h');
ok($foo->{G} == 0.12, 'g');
ok($foo->{epoch_mjd_TT} == 55400, 'epoch_mjd_TT');
ok($foo->{epoch_mjd_UTC} == 55400 - slaDtt(55400) / 86400, 'epoch_mjd_UTC');
ok($foo->{argPeri_deg} == 72.58976, 'argPeri_deg');
ok($foo->{node_deg} == 80.39321, 'node_deg');
ok($foo->{i_deg} == 10.58682, 'i_deg');
ok($foo->{e} == 0.0791382, 'd');
ok($foo->{a_AU} == 2.7653485, 'a_AU');
ok($foo->{q_AU} == $foo->{a_AU} * (1 - $foo->{e}), 'q_AU');
ok($foo->{longname} eq '(1) Ceres');

__DATA__
00001    3.34  0.12 K107N 113.41048   72.58976   80.39321   10.58682  0.0791382  0.21432817   2.7653485  0 MPO110568  6063  94 1802-2006 0.61 M-v 30h MPCW       0000      (1) Ceres              20061025
