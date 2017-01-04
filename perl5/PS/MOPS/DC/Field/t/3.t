# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use Test::More tests => 2;
use strict;
use warnings;
BEGIN { use PS::MOPS::DC::Field };

#########################

sub check_globals {
    return defined($PS::MOPS::DC::Field::DUMMY_FILTER_ID) 
        and defined($PS::MOPS::DC::Field::DUMMY_SURVEY_MODE);
}


ok(check_globals(), 'globals');
