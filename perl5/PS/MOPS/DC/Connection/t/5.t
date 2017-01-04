# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 2.t'

#########################

use Test::More tests => 1;
BEGIN { use PS::MOPS::DC::Connection };

#########################

sub test_import {
    modc_getdbh() and modc_newdbh();
}

ok(test_import, 'test_import');
