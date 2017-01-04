package PS::MOPS::DC::Known;

=head1 NAME

PS::MOPS::DC::Known - Module for manipulating known object table

=head1 SYNOPSIS

  use PS::MOPS::DC::Known

  $known_id = modck_add('2009LD5')          # add designation to known table
  $tracklet->knownId($known_id);            # assign to tracklet

=head1 DESCRIPTION

Add and lookup entries for known objects.  Known object assigments
will generally be managed external to MOPS proper.  Assignments of
MOPS tracklets to known objects will be for decoration and lookup only.
No verification whatsoever on known decorators is done with MOPS.

=head2 EXPORT

None by default.

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2009 Institute for Astronomy, University of Hawaii

=cut

use 5.008;
use strict;
use warnings;
use Carp;

use base qw(Exporter Class::Accessor);

our @EXPORT = qw(
    modck_add
    modck_lookup
);

our $VERSION = '0.01';


# Our stuff.
__PACKAGE__->mk_accessors(qw(
    knownId
    knownName
));

use Params::Validate;
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Iterator;

our $by_value_validate_args = {
    knownName => 1,         # MPC/IAU name or designation
};


sub modck_add {
    # Add a new name to the table of known objects.  No-op if the name already exists.
    # Return the known ID.
    my $inst = shift;
    my $name = shift or die "no name specified";
    my ($q_AU, $e, $i_deg, $node_deg, $arg_peri_deg, $time_peri_mjd, $epoch_mjd, $hV) = @_;
    my $dbh = $inst->dbh;

    my @existing = $dbh->selectrow_array(<<"SQL", undef, $name);
select known_id from known where known_name=?
SQL
    if (@existing) {
        return $existing[0];
    }
    else {
        my $rv = $dbh->do(<<"SQL", undef, $name, $q_AU, $e, $i_deg, $node_deg, $arg_peri_deg, $time_peri_mjd, $epoch_mjd, $hV);
insert into known (known_name, q, e, i, node, arg_peri, time_peri, epoch, h_v)
values (?, ?, ?, ?, ?, ?, ?, ?, ?)
SQL
        $rv = $dbh->{'mysql_insertid'} if $rv;
        return $rv;
    }
}


sub modck_lookup {
    # return internal known ID for obj, undef if it doesn't exist.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { knownId => 0, knownName => 0});
    my $row;

    if ($args{knownId}) {
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{knownId});
select known_id, known_name, q, e, i, node, arg_peri, time_peri, epoch, h_v
from known
where known_id=?
SQL
    }
    elsif ($args{knownName}) {
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{knownName}) or croak "can't prepare sql";
select known_id, known_name, q, e, i, node, arg_peri, time_peri, epoch, h_v
from known
where known_name=?
SQL
    }
                                                                                                                                       
    return $row;
}
                                                                                                                                       
                                                                                                                                       
1;
