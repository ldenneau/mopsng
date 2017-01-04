package PS::MOPS::DC::EONQueue;

use 5.008;
use strict;
use warnings;

use base qw(Exporter Class::Accessor);

our @EXPORT = qw(
    modcq_submit
    modcq_retrieve
    modcq_update
    modcq_flush
);
our $VERSION = '0.01';

use PS::MOPS::Constants qw(:all);

# Field member descriptions:  see POD below.
__PACKAGE__->mk_accessors(qw(
    derivedobjectId
    eventId
    insertTimestamp
    status
));


sub modcq_submit {
    my $inst = shift;
    my ($derivedobjectId, $eventId, $status) = @_;
    $status ||= $EONQUEUE_STATUS_NEW;
    
    my $dbh = $inst->dbh;
    
    # First see if this object is already in the queue.  If so, delete it.
    my $existing = $dbh->do("select derivedobject_id from eon_queue where derivedobject_id=?", undef, $derivedobjectId);
    if ($existing) {
        $dbh->do("delete from eon_queue where derivedobject_id=?", undef, $derivedobjectId) 
            or die "can't remove derived object ID $derivedobjectId";
    }
# July 7, 2011 - Denver Green
# Removing code that updates the aQueue table as the update of the aQueue table
# is now done by a seperate python program whose sole purpose is to update this
# table.    
    # Insert an entry into AQueue.
    #my $sql = <<"SQL";
#insert into aqueue (derivedobject_id, event_id, status)
#values (?, ?, "N")
#SQL
#    eval {$dbh->do($sql, undef, $derivedobjectId, $eventId)};
	
    # Now insert the EONQueue entry.
    my $sql = <<"SQL";
insert into eon_queue (derivedobject_id, event_id, status)
values (?, ?, ?)
SQL
    return $dbh->do($sql, 
                    undef, 
                    $derivedobjectId, 
                    $eventId,
                    $status) or die "can't do SQL: $sql";
}


sub modcq_retrieve {
    # Return all derived objects in the queue with matching status.
    my ($inst, $status, $event_type) = @_;
    my $objectIds = [];         # list of returned objects
    
    my $dbh = $inst->dbh;
    my $sql;
   
    if ($event_type) {
        $sql = <<"SQL";
select derivedobject_id, event_id, insert_timestamp, status
from eon_queue, history
where eon_queue.event_id=history.event_id
and history.event_type='$event_type'
and eon_queue.status=?
SQL
    }
    else {
        $sql = <<"SQL";
select derivedobject_id, event_id, insert_timestamp, status
from eon_queue
where status=?
SQL
    }

    my $sth = $dbh->prepare($sql);
    $sth->execute($status) or die $dbh->errstr;

    my $row;
    while ($row = $sth->fetchrow_arrayref) {
        push @{$objectIds}, bless {
            _inst => $inst,
            derivedobjectId => $row->[0],
            eventId => $row->[1],
            insertTimestamp => $row->[2],
            status => $row->[3],
        };
    }
    return $objectIds;
}


sub modcq_update {
    my ($inst, $derivedobjectId, $status) = @_;
    my $dbh = $inst->dbh;
    my $sql = <<"SQL";
update eon_queue set status=? where derivedobject_id=?
SQL
    $dbh->do($sql, undef, $status, $derivedobjectId) or die "can't update eon_queue derivedobjectId $derivedobjectId";
}


sub modcq_flush {
    my ($inst) = @_;
    my $dbh = $inst->dbh;
    my $sql = <<"SQL";
delete from eon_queue
where status=?
SQL
    $dbh->do($sql, undef, $EONQUEUE_STATUS_RETIRED) or die "can't flush eon_queue";
}


1;
__END__

=head1 NAME

PS::MOPS::DC::EONQueue - module for manipulating MOPS end-of-night (EON) queue

=head1 SYNOPSIS

  use PS::MOPS::DC::EONQueue;


  # add object to queue
  modcq_submit($inst, $do->derivedobjectId);

  # get all items in queue with specified status
  $listref = modcq_retrieve($inst, $status);

  # update status of item in queue
  modcq_update($inst, $do->derivedobjectId, $status);
  
  # remove all items from queue
  modcq_flush($inst)

=head1 DESCRIPTION

This module manages the MOPS end-of-night (EON) queue.  Objects placed in
this queue are retrieved after all linking jobs are completed for a night.
Objects created in the linking process or a prior attribution are placed
into this queue for subsequent orbit identification and precovery.

First, the EON process retrieves all objects from the queue and performs
an ID2 orbit identification.  The survivors (unmerged objects) from this
step are marked with status 'P' indicating they are eligible for precovery.
The merged objects are marked with status 'X' indicating they are retired
from the queue.

After the EON processing is completed, the queue is flushed.

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2007 Institute for Astronomy, University of Hawaii

=cut
