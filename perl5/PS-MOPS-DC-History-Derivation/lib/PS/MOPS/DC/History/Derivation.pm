package PS::MOPS::DC::History::Derivation;

use 5.008;
use strict;
use warnings;

use base qw(Exporter PS::MOPS::DC::History);

our @EXPORT = qw(
	modchd_retrieve
);
our $VERSION = '0.01';


use Params::Validate;
use PS::MOPS::DC::Iterator;


our $selectall_str = <<"SQL";
select
    event_id,
    tracklet_id
SQL


__PACKAGE__->mk_accessors(qw(
    trackletIds
));


sub new {
    my ($pkg, $inst, %args) = @_;
    my $trackletIds = delete $args{trackletIds};        # strip trackletIds off input args

    my $self = PS::MOPS::DC::History::new($pkg, $inst, %args,
        eventType => $PS::MOPS::DC::History::EVENT_TYPE_DERIVATION);

    $self->trackletIds($trackletIds);
    return $self;
}


sub record {
    my ($self) = @_;
    $self->SUPER::record();     # insert History event row

    # Now insert our subclass-specific information.
    my $dbh = $self->{_inst}->dbh;
    my $numrows_inserted = 0;

    foreach (@{$self->trackletIds}) {
        $numrows_inserted +=  $dbh->do(<<"SQL", undef, $self->eventId, $_);
insert into history_derivations (
event_id, tracklet_id
)
values (
?, ?
)
SQL
    }

    return $numrows_inserted == scalar @{$self->trackletIds};
}


sub modchd_retrieve {
    # return history events.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, {
        eventId => 0,           # by eventId
        epoch_mjd => 0,         # by night/epoch
        fieldId => 0,           # by field
    });
    my $row;

    if ($args{eventId}) {
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{eventId});
$PS::MOPS::DC::History::selectall_str
from history h
where event_id=?
SQL
        if ($row) {
            my $trackletIds_aref = $dbh->selectcol_arrayref(<<"SQL", undef, $row->{event_id}) or die $dbh->errstr;
select tracklet_id
from history_derivations d
where event_id=?
SQL

            return PS::MOPS::DC::History::Derivation->new($inst,
                eventId => $row->{event_id},
                eventType => $row->{event_type},
                eventTime => $row->{event_time},
                derivedobjectId => $row->{derivedobject_id},
                fieldId => $row->{field_id},
                orbitId => $row->{orbit_id},
                orbitCode => $row->{orbit_code},
                classification => $row->{classification},
                ssmId => $row->{ssm_id},
                trackletIds => $trackletIds_aref,
            );
        }
    }
    elsif ($args{epoch_mjd}) {
        my @hobjs;
        my $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
$PS::MOPS::DC::History::selectall_str
from history h, `fields` f
where h.event_type='$PS::MOPS::DC::History::EVENT_TYPE_DERIVATION'
and h.field_id=f.field_id and f.epoch_mjd >= ? and f.epoch_mjd < ?
SQL

        $sth->execute($args{epoch_mjd}, $args{epoch_mjd} + 1) or die $sth->errstr;

        my $href;
        while ($href = $sth->fetchrow_hashref) {
            my $trackletIds_aref = $dbh->selectcol_arrayref(<<"SQL", undef, $href->{event_id}) or die $dbh->errstr;
select tracklet_id
from history_derivations d
where event_id=?
SQL

            push @hobjs, PS::MOPS::DC::History::Derivation->new($inst,
                eventId => $href->{event_id},
                eventType => $href->{event_type},
                eventTime => $href->{event_time},
                derivedobjectId => $href->{derivedobject_id},
                fieldId => $href->{field_id},
                orbitId => $href->{orbit_id},
                orbitCode => $href->{orbit_code},
                classification => $href->{classification},
                ssmId => $href->{ssm_id},
                trackletIds => $trackletIds_aref,
            );
        }
        return \@hobjs;
    }
    elsif ($args{fieldId}) {
        my @hobjs;
        my $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
$PS::MOPS::DC::History::selectall_str
from history h, `fields` f
where h.event_type='$PS::MOPS::DC::History::EVENT_TYPE_DERIVATION'
and h.field_id=f.field_id and f.field_id=?
SQL

        $sth->execute($args{fieldId}) or die $sth->errstr;

        my $href;
        while ($href = $sth->fetchrow_hashref) {
            my $trackletIds_aref = $dbh->selectcol_arrayref(<<"SQL", undef, $href->{event_id}) or die $dbh->errstr;
select tracklet_id
from history_derivations d
where event_id=?
SQL

            push @hobjs, PS::MOPS::DC::History::Derivation->new($inst,
                eventId => $href->{event_id},
                eventType => $href->{event_type},
                eventTime => $href->{event_time},
                derivedobjectId => $href->{derivedobject_id},
                fieldId => $href->{field_id},
                orbitId => $href->{orbit_id},
                orbitCode => $href->{orbit_code},
                classification => $href->{classification},
                ssmId => $href->{ssm_id},
                trackletIds => $trackletIds_aref,
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

PS::MOPS::DC::History::Derivation - Perl extension for history derivation events

=head1 SYNOPSIS

  use PS::MOPS::DC::History::Derivation;
  $evt = PS::MOPS::DC::History::Derivation->new($inst, %args);
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
