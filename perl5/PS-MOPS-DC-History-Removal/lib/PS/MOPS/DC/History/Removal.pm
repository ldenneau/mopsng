package PS::MOPS::DC::History::Removal;

use 5.008;
use strict;
use warnings;

use base qw(Exporter PS::MOPS::DC::History);

our @EXPORT = qw(
	modchr_retrieve
	
	$TYPE_DERIVED
	$TYPE_TRACKLET
);
our %EXPORT_TAGS = ( 'all' => [ qw(
	modchr_retrieve
	
	$TYPE_DERIVED
	$TYPE_TRACKLET
) ] );
our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our $VERSION = '0.02';
our $TYPE_DERIVED = 'D';
our $TYPE_TRACKLET = 'T';

use Params::Validate;
use PS::MOPS::DC::Iterator;


our $selectall_str = <<"SQL";
select
    event_id,
    obj_id,
    obj_type
SQL


__PACKAGE__->mk_accessors(qw(
    objId objType
));


sub new {
    my ($pkg, $inst, %args) = @_;
    # strip the id of item to be removed off input args
    my ($objId, $objType) = delete @args{qw(objId objType)};     

    my $self = PS::MOPS::DC::History::new($pkg, $inst, %args,
        eventType => $PS::MOPS::DC::History::EVENT_TYPE_REMOVAL);

    $self->objId($objId);
    $self->objType($objType);
    return $self;
}


sub record {
    my ($self) = @_;
    $self->SUPER::record();     # insert History event row

    # Now insert our subclass-specific information.
    my $dbh = $self->{_inst}->dbh;
    my $numrows_inserted = $dbh->do(<<"SQL", undef, $self->eventId, $self->objId, $self->objType);
insert into history_removals (
event_id, object_id, object_type
)
values (
    ?, /* event_id */
    ?, /* obj_id */
    ?  /* obj_type */
)
SQL

    return $numrows_inserted;
}


sub modchr_retrieve {
    # return history events.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, {
        eventId => 0,           # by eventId
        epoch_mjd => 0,         # by night/epoch
        fieldId => 0,           # by field ID
    });
    my $row;
    
    if ($args{eventId}) {
        # Retrieve info using eventId.
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{eventId});
$PS::MOPS::DC::History::selectall_str
from history h
where event_id=?
SQL
        if ($row) {
            my $removal_row = $dbh->selectrow_hashref(<<"SQL", undef, $row->{event_id}) or die $dbh->errstr;
select object_id, object_type
from history_removals hr
where event_id=?
SQL
            return PS::MOPS::DC::History::Removal->new($inst,
                eventId => $row->{event_id},
                eventType => $row->{event_type},
                eventTime => $row->{event_time},
                derivedobjectId => $row->{derivedobject_id},
                fieldId => $row->{field_id},
                orbitId => $row->{orbit_id},
                orbitCode => $row->{orbit_code},
                classification => $row->{classification},
                ssmId => $row->{ssm_id},
                objId => $removal_row->{object_id},
                objType => $removal_row->{object_type},
            );
        }
    }
    elsif ($args{epoch_mjd}) {
        # Retrieve info using epoch.
        my @hobjs;
        my $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
$PS::MOPS::DC::History::selectall_str
from history h, `fields` f
where h.event_type='$PS::MOPS::DC::History::EVENT_TYPE_REMOVAL'
and h.field_id=f.field_id and f.epoch_mjd >= ? and f.epoch_mjd < ?
SQL

        $sth->execute($args{epoch_mjd}, $args{epoch_mjd} + 1) or die $sth->errstr;
        while ($row = $sth->fetchrow_hashref) {
            my $removal_row = $dbh->selectrow_hashref(<<"SQL", undef, $row->{event_id}) or die $dbh->errstr;
select object_id, object_type
from history_removals hr
where event_id=?
SQL

            push @hobjs, PS::MOPS::DC::History::Removal->new($inst,
                eventId => $row->{event_id},
                eventType => $row->{event_type},
                eventTime => $row->{event_time},
                derivedobjectId => $row->{derivedobject_id},
                fieldId => $row->{field_id},
                orbitId => $row->{orbit_id},
                orbitCode => $row->{orbit_code},
                classification => $row->{classification},
                ssmId => $row->{ssm_id},
                objId => $removal_row->{object_id},
                objType => $removal_row->{object_type},
            );
        }
        return \@hobjs;
    }
    elsif ($args{fieldId}) {
        # Retrieve info using fieldId.
        my @hobjs;
        my $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
$PS::MOPS::DC::History::selectall_str
from history h, `fields` f
where h.event_type='$PS::MOPS::DC::History::EVENT_TYPE_REMOVAL'
and h.field_id=f.field_id and f.field_id=?
SQL

        $sth->execute($args{fieldId}) or die $sth->errstr;
        while ($row = $sth->fetchrow_hashref) {
            my $removal_row = $dbh->selectrow_hashref(<<"SQL", undef, $row->{event_id}) or die $dbh->errstr;
select object_id, object_type
from history_removals hr
where event_id=?
SQL

            push @hobjs, PS::MOPS::DC::History::Precovery->new($inst,
                eventId => $row->{event_id},
                eventType => $row->{event_type},
                eventTime => $row->{event_time},
                derivedobjectId => $row->{derivedobject_id},
                fieldId => $row->{field_id},
                orbitId => $row->{orbit_id},
                orbitCode => $row->{orbit_code},
                classification => $row->{classification},
                ssmId => $row->{ssm_id},
                objId => $removal_row->{object_id},
                objType => $removal_row->{object_type},
            );
        }
        return \@hobjs;
    }
    else {
        return undef;
    }
}

1;
__END__
=head1 NAME

PS::MOPS::DC::History::Removal - Perl extension for history removal events

=head1 SYNOPSIS

  use PS::MOPS::DC::History::Removal;
  $evt = PS::MOPS::DC::History::Removal->new($inst, %args);
  $evt->record();

  $evt_i = modchr_retrieve(%args);
  while ($evt = $evt_i->next) {
    do_something($evt);
  }

=head1 DESCRIPTION


=head2 EXPORT

modchr_retrieve

=head1 SEE ALSO

=head1 AUTHOR
Denver Green <dgreen@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE
Copyright 2011 Institute for Astronomy, University of Hawaii

=cut