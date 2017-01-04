# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use strict;
use warnings;
use Test::More tests => 8;
BEGIN { use_ok('PS::MOPS::MITI') };

#########################
ok($MITI_COLUMN_ID == 0, 'col0');
ok($MITI_COLUMN_EPOCH == 1, 'col1');
ok($MITI_COLUMN_RA == 2, 'col2');
ok($MITI_COLUMN_DEC == 3, 'col3');
ok($MITI_COLUMN_MAG == 4, 'col4');
ok($MITI_COLUMN_OBSCODE == 5, 'col5');
ok($MITI_COLUMN_OBJECT_NAME == 6, 'col6');
