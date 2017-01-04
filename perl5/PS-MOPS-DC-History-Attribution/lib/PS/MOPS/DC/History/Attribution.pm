package PS::MOPS::DC::History::Attribution;

use 5.008;
use strict;
use warnings;

use base qw(Exporter PS::MOPS::DC::History);

our @EXPORT = qw(
	modcha_retrieve
);
our $VERSION = '0.01';


use Params::Validate;
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Iterator;


our $selectall_str = <<"SQL";
select
    event_id,
    tracklet_id,
    ephemeris_distance,
    ephemeris_uncertainty
SQL


__PACKAGE__->mk_accessors(qw(
    trackletId
    ephemerisDistance
    ephemerisUncertainty
));


sub new {
    my ($pkg, $inst, %args) = @_;
    # strip trackletId off input args
    my ($trackletId, $ephemerisDistance, $ephemerisUncertainty) =
        delete @args{qw(trackletId ephemerisDistance ephemerisUncertainty)};

    my $self = PS::MOPS::DC::History::new($pkg, $inst, %args,
        eventType => $PS::MOPS::DC::History::EVENT_TYPE_ATTRIBUTION);

    $self->trackletId($trackletId);
    $self->ephemerisDistance($ephemerisDistance);
    $self->ephemerisUncertainty($ephemerisUncertainty);
    return $self;
}


sub record {
    my ($self) = @_;
    $self->SUPER::record();     # insert History event row

    # Now insert our subclass-specific information.
    my $dbh = $self->{_inst}->dbh;
    my $numrows_inserted = $dbh->do(<<"SQL", undef, $self->eventId, $self->trackletId, $self->ephemerisDistance, $self->ephemerisUncertainty);
insert into history_attributions (
event_id, tracklet_id, ephemeris_distance, ephemeris_uncertainty
)
values (
?, ?, ?, ?
)
SQL

    return $numrows_inserted;
}


sub modcha_retrieve {
    # return history events.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, {
        eventId => 0,           # by eventId
        epoch_mjd => 0,         # by night/epoch
        fieldId => 0,
    });
    my $row;

    if ($args{eventId}) {
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{eventId});
$PS::MOPS::DC::History::selectall_str
from history h
where event_id=?
SQL
        if ($row) {
            my $attr_row = $dbh->selectrow_hashref(<<"SQL", undef, $row->{event_id}) or die $dbh->errstr;
select tracklet_id, ephemeris_distance, ephemeris_uncertainty
from history_attributions ha
where event_id=?
SQL

            return PS::MOPS::DC::History::Attribution->new($inst,
                eventId => $row->{event_id},
                eventType => $row->{event_type},
                eventTime => $row->{event_time},
                derivedobjectId => $row->{derivedobject_id},
                fieldId => $row->{field_id},
                orbitId => $row->{orbit_id},
                orbitCode => $row->{orbit_code},
                classification => $row->{classification},
                ssmId => $row->{ssm_id},
                trackletId => $attr_row->{tracklet_id},
                ephemerisDistance => $attr_row->{ephemeris_distance},
		ephemerisUncertainty => $attr_row->{ephemeris_uncertainty},
            );
        }
    }
    elsif ($args{epoch_mjd}) {
        my @hobjs;
        my $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
$PS::MOPS::DC::History::selectall_str
from history h, `fields` f
where h.event_type='$PS::MOPS::DC::History::EVENT_TYPE_ATTRIBUTION'
and h.field_id=f.field_id and f.epoch_mjd >= ? and f.epoch_mjd < ?
SQL

        $sth->execute($args{epoch_mjd}, $args{epoch_mjd} + 1) or die $sth->errstr;

        my $href;
        while ($href = $sth->fetchrow_hashref) {
            my $attr_row = $dbh->selectrow_hashref(<<"SQL", undef, $href->{event_id}) or die $dbh->errstr;
select tracklet_id, ephemeris_distance, ephemeris_uncertainty
from history_attributions ha
where event_id=?
SQL

            push @hobjs, PS::MOPS::DC::History::Attribution->new($inst,
                eventId => $href->{event_id},
                eventType => $href->{event_type},
                eventTime => $href->{event_time},
                derivedobjectId => $href->{derivedobject_id},
                fieldId => $href->{field_id},
                orbitId => $href->{orbit_id},
                orbitCode => $href->{orbit_code},
                classification => $href->{classification},
                ssmId => $row->{ssm_id},
                trackletId => $attr_row->{tracklet_id},
                ephemerisDistance => $attr_row->{ephemeris_distance},
                ephemerisUncertainty => $attr_row->{ephemeris_uncertainty},
            );
        }
        return \@hobjs;
    }
    elsif ($args{fieldId}) {
        my @hobjs;
        my $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
$PS::MOPS::DC::History::selectall_str
from history h, `fields` f
where h.event_type='$PS::MOPS::DC::History::EVENT_TYPE_ATTRIBUTION'
and h.field_id=f.field_id and f.field_id=?
SQL

        $sth->execute($args{fieldId}) or die $sth->errstr;

        my $href;
        while ($href = $sth->fetchrow_hashref) {
            my $attr_href = $dbh->selectrow_hashref(<<"SQL", undef, $href->{event_id}) or die $dbh->errstr;
select tracklet_id, ephemeris_distance, ephemeris_uncertainty
from history_attributions a
where event_id=?
SQL

            push @hobjs, PS::MOPS::DC::History::Attribution->new($inst,
                eventId => $href->{event_id},
                eventType => $href->{event_type},
                eventTime => $href->{event_time},
                derivedobjectId => $href->{derivedobject_id},
                fieldId => $href->{field_id},
                orbitId => $href->{orbit_id},
                orbitCode => $href->{orbit_code},
                classification => $href->{classification},
                ssmId => $href->{ssm_id},
                trackletId => $attr_href->{tracklet_id},
                ephemerisDistance => $attr_href->{ephemeris_distance},
                ephemerisUncertainty => $attr_href->{ephemeris_uncertainty},
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

PS::MOPS::DC::History::Attribution - Perl extension for history attribution events

=head1 SYNOPSIS

  use PS::MOPS::DC::History::Attribution;
  $evt = PS::MOPS::DC::History::Attribution->new(stuff);
  $evt->record();

  $evt_i = modchd_retrieve($inst, %args);
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
