# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More tests => 1;
use strict;
use warnings;
use PS::MOPS::DC::Tracklet;

use PS::MOPS::Constants qw(:all);

#########################

#ok(glf, 'getLastFieldId');
ok(1, 'getLastFieldId');            # need test module
##ok(1, 'noDets');            # need test module
