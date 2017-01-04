# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use strict;
use warnings;
use Test::More tests => 1;
use PS::MOPS::MITI;

#########################

sub test_miti_format_hash {
    1;
}

ok(test_miti_format_hash, 'miti_format_hash');
