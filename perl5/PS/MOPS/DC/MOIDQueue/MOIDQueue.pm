package PS::MOPS::DC::MOIDQueue;

use 5.008;
use strict;
use warnings;

use base qw(Class::Accessor);
our $VERSION = '0.01';

use PS::MOPS::Constants;
use PS::MOPS::DC::Orbit;


# Field member descriptions:  see POD below.
__PACKAGE__->mk_accessors(qw(
    orbitId
    eventId
    insertTimestamp
));


sub submit {
    my ($pkg, $inst, $orbitId, $eventId) = @_;
    my $dbh = $inst->dbh;

    # First see if this object is already in the queue.  If so, delete it.
    my $existing = $dbh->do("select orbit_id from moid_queue where orbit_id=?", undef, $orbitId);
    if ($existing) {
        $dbh->do("delete from moid_queue where orbit_id=?", undef, $orbitId) 
            or die "can't remove derived object ID $orbitId";
    }
    
    # Now insert the MOIDQueue entry.
    my $sql = <<"SQL";
insert into moid_queue (orbit_id, event_id)
values (?, ?)
SQL
    return $dbh->do($sql, undef, $orbitId, $eventId) or die "can't do SQL: $sql";
}


sub retrieve {
    # Return an arrayref of orbits from the MOID queue.
    my ($pkg, $inst) = @_;

    my $orbit_i = modco_retrieve($inst, moid_queue => 1);
    my $orbit;
    my @orbits;
    while ($orbit = $orbit_i->next()) {
        push @orbits, $orbit;
    }

    return \@orbits;
}


sub flush {
    my ($pkg, $inst) = @_;
    my $dbh = $inst->dbh;
    my $sql = <<"SQL";
delete from moid_queue
SQL
    $dbh->do($sql) or die "can't flush moid_queue";
}


1;
__END__

=head1 NAME

PS::MOPS::DC::MOIDQueue - module for manipulating MOPS MOID queue

=head1 SYNOPSIS

  use PS::MOPS::DC::MOIDQueue;

  # Add orbit to queue.
  PS::MOPS::DC::MOIDQueue->submit($inst, $orbit);

  # Get all items in queue.
  $orbits_aref = PS::MOPS::DC::MOIDQueue->retrieve($inst);

  # Remove all items from queue
  PS::MOPS::DC::MOIDQueue->flush($inst)

=head1 DESCRIPTION

The MOIDQueue is a simple queue for aggregating orbits that need their
MOID computed by some batch process.

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2007 Institute for Astronomy, University of Hawaii

=cut
