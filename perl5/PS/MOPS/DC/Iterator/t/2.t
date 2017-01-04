# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use Test::More tests => 3;
BEGIN { use_ok('PS::MOPS::DC::Iterator') };

#########################

sub new {
    # Test cheesy version of iterator, with preloaded results.
    my $counter = 0;
    my $factory = sub {
        if ($counter > 3) {
            return undef;
        }
        else {
            $counter++;
            return $counter;
        }
    };
    my $i = PS::MOPS::DC::Iterator->new($factory);
    return $i->next == 1 and $i->next == 2 and $i->next == 3 and $i->next == undef;
}

sub cheesy {
    # Test cheesy version of iterator, with preloaded results.
    my @results = (1, 2, 3);
    my $i = PS::MOPS::DC::Iterator->cheesy(\@results);
    return $i->next == 1 and $i->next == 2 and $i->next == 3 and $i->next == undef;
}

ok(new(), 'cheesy1');
ok(cheesy(), 'cheesy2');
