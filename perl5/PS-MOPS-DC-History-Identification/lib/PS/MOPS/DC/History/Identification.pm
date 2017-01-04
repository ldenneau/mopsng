package PS::MOPS::DC::History::Identification;

use 5.008;
use strict;
use warnings;

use base qw(Exporter PS::MOPS::DC::History);

our @EXPORT = qw(
	modchi_retrieve
);
our $VERSION = '0.01';


use Params::Validate;
use PS::MOPS::DC::Iterator;


our $selectall_str = <<"SQL";
select
    event_id,
    childobject_id
SQL


__PACKAGE__->mk_accessors(qw(
    childobjectId
    parentOrbitId
    childOrbitId
));


sub new {
    my ($pkg, $inst, %args) = @_;
    my ($childobjectId, $parentOrbitId, $childOrbitId) = 
        delete @args{qw(childobjectId parentOrbitId childOrbitId)};

    my $self = PS::MOPS::DC::History::new($pkg, $inst, %args,
        eventType => $PS::MOPS::DC::History::EVENT_TYPE_IDENTIFICATION);

    $self->childobjectId($childobjectId);
    $self->parentOrbitId($parentOrbitId);
    $self->childOrbitId($childOrbitId);
    return $self;
}


sub record {
    my ($self) = @_;
    $self->SUPER::record();     # insert History event row

    # Now insert our subclass-specific information.
    my $dbh = $self->{_inst}->dbh;
    my $numrows_inserted = $dbh->do(<<"SQL", undef, $self->eventId, $self->childobjectId, $self->parentOrbitId, $self->childOrbitId);
insert into history_identifications (
event_id, childobject_id, parent_orbit_id, child_orbit_id
)
values (
?, ?, ?, ?
)
SQL

    return $numrows_inserted;
}


sub modchi_retrieve {
    # return history events.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, {
        eventId => 0,           # by eventId
        epoch_mjd => 0,         # by night/epoch
    });
    my $row;

    if ($args{eventId}) {
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{eventId});
$PS::MOPS::DC::History::selectall_str
from history h
where event_id=?
SQL
        if ($row) {
            my $ident_row = $dbh->selectrow_hashref(<<"SQL", undef, $row->{event_id}) or die $dbh->errstr;
select childobject_id, parent_orbit_id, child_orbit_id
from history_identifications hi
where event_id=?
SQL

            return PS::MOPS::DC::History::Identification->new($inst,
                eventId => $row->{event_id},
                eventType => $row->{event_type},
                eventTime => $row->{event_time},
                derivedobjectId => $row->{derivedobject_id},
                fieldId => $row->{field_id},
                orbitId => $row->{orbit_id},
                orbitCode => $row->{orbit_code},
                classification => $row->{classification},
                ssmId => $row->{ssm_id},
                childobjectId => $ident_row->{childobject_id},
                parentOrbitId => $ident_row->{parent_orbit_id},
                childOrbitId => $ident_row->{child_orbit_id},
            );
        }
    }
#    elsif ($args{epoch_mjd}) {
    elsif (exists($args{epoch_mjd})) {
        my @hobjs;
#        my $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
#$PS::MOPS::DC::History::selectall_str
#from history h, `fields` f
#where h.event_type='$PS::MOPS::DC::History::EVENT_TYPE_IDENTIFICATION'
#and h.field_id=f.field_id and f.epoch_mjd >= ? and f.epoch_mjd < ?
#SQL
        my $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
$PS::MOPS::DC::History::selectall_str
from history h
where h.event_type='$PS::MOPS::DC::History::EVENT_TYPE_IDENTIFICATION'
SQL

#        $sth->execute($args{epoch_mjd}, $args{epoch_mjd} + 1) or die $sth->errstr;
        $sth->execute or die $sth->errstr;

        my $href;
        while ($href = $sth->fetchrow_hashref) {
            my $ident_row = $dbh->selectrow_hashref(<<"SQL", undef, $href->{event_id}) or die $dbh->errstr;
select childobject_id
from history_identifications ha
where event_id=?
SQL

            push @hobjs, PS::MOPS::DC::History::Identification->new($inst,
                eventId => $href->{event_id},
                eventType => $href->{event_type},
                eventTime => $href->{event_time},
                derivedobjectId => $href->{derivedobject_id},
                fieldId => $href->{field_id},
                orbitId => $href->{orbit_id},
                orbitCode => $href->{orbit_code},
                classification => $href->{classification},
                ssmId => $row->{ssm_id},
                childobjectId => $ident_row->{childobject_id},
                parentOrbitId => $ident_row->{parent_orbit_id},
                childOrbitId => $ident_row->{child_orbit_id},
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

PS::MOPS::DC::History::Identification - Perl extension for history identification events

=head1 SYNOPSIS

  use PS::MOPS::DC::History::Identification;
  $evt = PS::MOPS::DC::History::Identification->new($inst, %args);
  $evt->record();

  $evt_i = modchd_retrieve(%args);
  while ($evt = $evt_i->next) {
    do_something($evt);
  }

=head1 DESCRIPTION


=head2 EXPORT

modchd_retrieve

=head1 SEE ALSO

=head1 AUTHOR

=head1 COPYRIGHT AND LICENSE

=cut
