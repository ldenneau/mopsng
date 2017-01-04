# $Id: support.pl 1218 2006-08-03 19:16:53Z denneau $
# Separate support code stub for testing.

sub _setenv {
    my ($k, $v) = @_;
    $ENV{$k} = $v;
}


sub _addenv {
    # Add the specified item to the environment PATH-style.
    # First check to see if the thing we're adding is already there; if so, don't
    # do anything.
    my ($k, $v) = @_;
    my $re = qr/$v/;            # save regexp

    if (!$ENV{$k}) {
        $ENV{$k} = $v;          # wasn't set, so set it
    }
    else {
        my @stuff = split /:/, $ENV{$k};
        foreach my $item (@stuff) {
            return if $item =~ $re;     # found it, so bail
        }
        $ENV{$k} = join ':', $v, @stuff;
    }
}

1;
