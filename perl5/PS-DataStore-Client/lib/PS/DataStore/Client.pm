package PS::DataStore::Client;

use 5.008008;
use strict;
use warnings;

require Exporter;

our @ISA = qw(Exporter);

our %EXPORT_TAGS = ( 'all' => [ qw(
) ] );
our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );
our @EXPORT = qw(
	
);

our $VERSION = '0.01';


# Preloaded methods go here.

1;
__END__

=head1 NAME

PS::DataStore::Client - Perl extension to query PS datastores

=head1 SYNOPSIS

  use PS::DataStore::Client;
  $ds = PS::DataStore::Client->new($url);
  $ds->query($product, $fileset, $since);
  $ds->retrieve_fileset($product, $fileset);

=head1 DESCRIPTION

Query a Pan-STARRS datastore for new filesets, and retrieve files
from a datastore.

=head1 AUTHOR

Larry Denneau, Jr., University of Hawaii, University of Hawaii, E<lt>denneau@ifa.hawaii.eduE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2009 by Larry Denneau, Jr., University of Hawaii.

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.8 or,
at your option, any later version of Perl 5 you may have available.


=cut
