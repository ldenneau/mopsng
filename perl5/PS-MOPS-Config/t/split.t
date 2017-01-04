# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl PS-MOPS-Config.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More tests => 2;
BEGIN { use_ok('PS::MOPS::Config') };

#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.

my $text = join '', <DATA>;
$parsed = PS::MOPS::Config->new_from_string($text);

ok($parsed->{foo}->{bar} eq 'dog=cat', 'split');



__DATA__
# Test config file.  Put everything we might see in here.

# Solar system model, used for synthetic data generation.
foo {
    bar = 'dog=cat'                       # 1 => compute shape magnitudes for objects
}
