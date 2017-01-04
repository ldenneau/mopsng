package PS::MOPS::DC::Iterator;

use 5.008;
use strict;
use warnings;

use base qw(Exporter);
our @EXPORT = qw();
our $VERSION = '0.01';


sub new {
    # Standard invocation.  Caller provides a function that will return a new
    # result of appropriate type simply upon invocation.  Typically this will
    # be a closure containing a $sth and object definition.
    my ($pkg, $objfactory) = @_;
    my $self = {
        _objfactory => $objfactory,
    };
    return bless $self;
}


sub cheesy {
    # Accept a list of objects, and just run through them.  Lame.
    my ($pkg, $objs) = @_;
    my $self = {
        _prefetched => $objs,
        _current => 0,
    };
    return bless $self;
}

sub next {
    # Return the next item from our results.
    my ($self) = @_;
    my $next;
    if ($self->{_objfactory}) {
        return &{$self->{_objfactory}}; # grab next one
    }
    elsif ($self->{_prefetched}) {
        # used cheesy method, just check counter.
        if ($self->{_current} < @{$self->{_prefetched}}) {
            $next = $self->{_prefetched}->[$self->{_current}];
            $self->{_current}++;
            return $next;
        }
        else {
            return undef;
        }
    }
    else {
        return undef;   # XXX no workie
    }
}


1;
__END__

=head1 NAME

PS::MOPS::DC::Iterator - Iterator class for DC query methods

=head1 SYNOPSIS

  # Creating an iterator.
  use PS::MOPS::DC::Iterator;
  $sth = $dbh->prepare($sql);
  $sth->execute;
  return PS::MOPS::DC::Iterator->new($sth);

  # Using iterator.
  $obs_i = modcm_retrieve(date => 53372);   # all observations for MJD 53372
  while (defined($obs = $obs_i->next)) {
    # Do something with Observation object.
    doSomething($obs);
  }

=head1 DESCRIPTION

Use the DC Iterator class to iterator through rows of large result-set
MOPS DC queries.  To create an iterator class, simply pass a DBI statement
handle that has successfully executed and is awaiting row fetches.

To use the DC Iterator as a client, simply fetch the result from next(),
which will be an instance of appropriate class.

=head2 EXPORT

None by default.



=head1 SEE ALSO

DBI
PS::MOPS::DC

=head1 AUTHOR

Larry Denneau, denneau@ifa.hawaii.edu

=head1 COPYRIGHT AND LICENSE

Copyright 2005 by Institute for Astronomy, University of Hawaii.

=cut
