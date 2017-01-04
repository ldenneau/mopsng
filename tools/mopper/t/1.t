use strict;
use warnings;
use Test::More tests => 3;


#########################

sub test_setenv {
    _setenv('foo', 'bar');
    return $ENV{foo} eq 'bar';
}

sub test_addenv {
    $ENV{foo} = 'bar';
    _addenv('foo', 'baz');
    return unless $ENV{foo} eq 'baz:bar';

    # Try again; should not be added this time
    _addenv('foo', 'baz');
    return unless $ENV{foo} eq 'baz:bar';

    _addenv('foo', 'zyzzx');
    return unless $ENV{foo} eq 'zyzzx:baz:bar';

    1;
}

ok(require 'support.pl', 'require');
ok(test_setenv, 'set env');
ok(test_addenv, 'add env');
