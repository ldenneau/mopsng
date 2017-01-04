#!perl -T

use Test::More tests => 1;

BEGIN {
	use_ok( 'PS::MOPS::DC::Build' );
}

diag( "Testing PS::MOPS::DC::Build $PS::MOPS::DC::Build::VERSION, Perl $], $^X" );
