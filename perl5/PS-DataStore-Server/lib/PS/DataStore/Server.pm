package PS::DataStore::Server;

use 5.008008;
use strict;
use warnings;

our $VERSION = '0.01';


# Support routines
use subs qw(
);

sub new {
    my ($pkg, $ds_dir) = @_;

    # Attempt to locate the datastore root directory.
    if (!$ds_dir) {
        if (exists($ENV{MOPS_HOME}) {
            $ds_dir = "$ENV{MOPS_HOME}/ds/dsroot";
        }
    }
    die "Can't determine a datastore root directory" unless $ds_dir;
    die "Not a directory: $ds_dir" unless -d $ds_dir;

    return bless $ds_dir;
}


sub create_product {
    my ($self, $product) = @_;
}


sub add_fileset {
    my ($self, $product, $fileset) = @_;
}


sub remove_fileset {
    my ($self, $product, $fileset) = @_;
}


sub remove_product {
    my ($self, $product) = @_;
}




1;
__END__

=head1 NAME

PS::DataStore::Server - Server-side manipulation of PS datastore

=head1 SYNOPSIS

    use PS::DataStore::Server;
    $ds = new PS::DataStore::Server;

    $rv = $ds->exists($product);
    $rv = $ds->exists($product, $fileset);

    @products = $ds->list_products();
    $ds->create_product($product_name);
    $ds->remove_product($product_name);

    @filesets = $ds->list_filesets($product_name);
    $ds->create_fileset($product_name, $fileset_name, $type, @files);
    $ds->remove_fileset($product_name, $fileset_name);

    @files = $ds->list_fileset($product_name, $fileset_name);


=head1 DESCRIPTION

Manipulate files in  datastore hierarchy on the local system.  Use
file locking since other MOPS processes may be using the datastore.

=head1 AUTHOR

Larry Denneau, Jr., Institute for Astronomy, University of Hawaii, E<lt>denneau@ifa.hawaii.eduE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2009 by Larry Denneau, Jr., University of Hawaii

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.8 or,
at your option, any later version of Perl 5 you may have available.

=cut
