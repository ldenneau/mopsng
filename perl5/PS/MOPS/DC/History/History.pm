package PS::MOPS::DC::History;

use 5.008;
use strict;
use warnings;
use Carp;

use base qw(Exporter Class::Accessor);

our @EXPORT = qw(
    modch_retrieve

    $EVENT_TYPE_DERIVATION
    $EVENT_TYPE_ATTRIBUTION
    $EVENT_TYPE_PRECOVERY
    $EVENT_TYPE_IDENTIFICATION
    $EVENT_TYPE_REMOVAL
    $EVENT_TYPE_LINK2
);
our %EXPORT_TAGS = ( 'all' => [ qw(
    modch_retrieve

    $EVENT_TYPE_DERIVATION
    $EVENT_TYPE_ATTRIBUTION
    $EVENT_TYPE_PRECOVERY
    $EVENT_TYPE_IDENTIFICATION
    $EVENT_TYPE_REMOVAL
    $EVENT_TYPE_LINK2
) ] );
our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our $VERSION = '0.01';

use Params::Validate;
use PS::MOPS::Lib qw(:all);
use PS::MOPS::Constants qw(:all);
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Iterator;
use PS::MOPS::DC::SSM;
use PS::MOPS::DC::Tracklet;
use PS::MOPS::DC::Orbit;

# Event types.  These correspond to the various occurrences that 
# we're tracking.
our $EVENT_TYPE_DERIVATION = 'D';       # new derived object created
our $EVENT_TYPE_ATTRIBUTION = 'A';      # tracklet attributed
our $EVENT_TYPE_PRECOVERY = 'P';        # tracklet precovered
our $EVENT_TYPE_IDENTIFICATION = 'I';   # orbit identification
our $EVENT_TYPE_REMOVAL = 'R';          # single tracklet removal
our $EVENT_TYPE_LINK2 = '2';            # link2 derivation

our $selectall_str = <<"SQL";
select
    h.event_id event_id, 
    h.event_type event_type, 
    h.event_time event_time,
    h.derivedobject_id derivedobject_id,
    h.field_id field_id,
    h.orbit_id orbit_id,
    h.orbit_code orbit_code,
    h.classification classification,
    h.ssm_id ssm_id,
    h.d3 d3,
    h.d4 d4,
    h.is_lsd
SQL


__PACKAGE__->mk_accessors(qw(
    eventId
    eventType
    eventTime
    derivedobjectId
    fieldId
    orbitId
    orbitCode
    classification
    ssmId
    d3
    d4
    isLSD
));

our $by_value_validate_args = {
    eventId => 0,
    eventType => 1,
    eventTime => 0,
    derivedobjectId => 1,
    fieldId => 1,
    orbitId => 1,
    orbitCode => 1,
    classification => 1,
    ssmId => 0,
    d3 => 0,
    d4 => 0,
    isLSD => 0,
};


sub new {
    my $pkg = shift;
    my $inst = shift;
    my %self = validate(@_, $by_value_validate_args);
    $self{_inst} = $inst;
    $self{eventId} ||= undef;
    $self{eventTime} ||= undef;
    $self{isLSD} ||= 'N';

    # If ssmId is defined, the compute D3 and D4 D-criteria. (How expensive are the orbit fetches?)
    if ($self{orbitid} and defined($self{ssmId})) {
        croak("orbitId not specified") unless $self{orbitId};
        my $my_orb = modco_retrieve($inst, orbitId => $self{orbitId});
        my $ssm_orb = modcs_retrieve($inst, ssmId => $self{ssmId});
        croak("got empty orbit for one of orbitId: $self{orbitId} ssmId: $self{ssmId}") 
            unless ($my_orb and $ssm_orb);
        my ($d3, $d4) = mopslib_calculateDCriterion($ssm_orb, $my_orb);
        $self{d3} = ($d3);
        $self{d4} = ($d4);
    }

    return bless \%self, $pkg;
}


sub _new_from_row {
    my ($inst, $row) = @_;
    return undef unless $row;   # sanity check
    my $dobj = PS::MOPS::DC::History->new($inst,
        eventId => $row->{event_id},
        eventType => $row->{event_type},
        eventTime => $row->{event_time},
        derivedobjectId => $row->{derivedobject_id},
        fieldId => $row->{field_id},
        orbitId => $row->{orbit_id},
        orbitCode => $row->{orbit_code},
        classification => $row->{classification},
        ssmId => $row->{ssm_id},
        d3 => $row->{d3},
        d4 => $row->{d4},
        isLSD => $row->{is_lsd},
    );

    return bless $dobj;      # make object
}


sub modch_retrieve {
    # return history events using various rules.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, {
        eventId => 0,           # by eventId
        derivedobjectId => 0,   # by derivedobjectId

        # look up previously inserted event (usually unfound DO) by ssmId/fieldId
        ssmId => 0,
        fieldId => 0,
    });
    my $row;

    if ($args{eventId}) {
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{eventId});
$selectall_str
from history h
where event_id=?
SQL
        if ($row) {
            return _new_from_row($inst, $row);
        }
    }
    elsif ($args{ssmId} and $args{fieldId}) {
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{ssmId}, $args{fieldId});
$selectall_str
from history h
where ssm_id=? and field_id=?
SQL
        if ($row) {
            return _new_from_row($inst, $row);
        }
    }
    elsif ($args{ssmId}) {
        my @histobjs;
        my $sth;
        $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
select event_id, event_type 
from history h, ssm s
where h.ssm_id=s.ssm_id and s.ssm_id=?
SQL

        $sth->execute($args{ssmId}) or die $sth->errstr;

        require PS::MOPS::DC::History::Derivation;
        require PS::MOPS::DC::History::Attribution;
        require PS::MOPS::DC::History::Precovery;
        require PS::MOPS::DC::History::Identification;
        require PS::MOPS::DC::History::Link2;
        require PS::MOPS::DC::History::Removal;

        my $href;
        while ($href = $sth->fetchrow_hashref) {
            my $obj;
            if ($href->{event_type} eq $EVENT_TYPE_DERIVATION) {
                $obj = PS::MOPS::DC::History::Derivation::modchd_retrieve($inst, eventId => $href->{event_id});
            }
            elsif ($href->{event_type} eq $EVENT_TYPE_ATTRIBUTION) {
                $obj = PS::MOPS::DC::History::Attribution::modcha_retrieve($inst, eventId => $href->{event_id});
            }
            elsif ($href->{event_type} eq $EVENT_TYPE_PRECOVERY) {
                $obj = PS::MOPS::DC::History::Precovery::modchp_retrieve($inst, eventId => $href->{event_id});
            }
            elsif ($href->{event_type} eq $EVENT_TYPE_IDENTIFICATION) {
                $obj = PS::MOPS::DC::History::Identification::modchi_retrieve($inst, eventId => $href->{event_id});
            }
            elsif ($href->{event_type} eq $EVENT_TYPE_REMOVAL) {
                $obj = PS::MOPS::DC::History::Removal::modchr_retrieve($inst, eventId => $href->{event_id});
            }
            elsif ($href->{event_type} eq $EVENT_TYPE_LINK2) {
                $obj = PS::MOPS::DC::History::Link2::modch2_retrieve($inst, eventId => $href->{event_id});
            }
            push @histobjs, $obj;
        }
        return \@histobjs;
    }
    elsif ($args{derivedobjectId}) {
        my @histobjs;
        my $sth;
        $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
select event_id, event_type 
from history h
where derivedobject_id=?
SQL

        $sth->execute($args{derivedobjectId}) or die $sth->errstr;

        require PS::MOPS::DC::History::Derivation;
        require PS::MOPS::DC::History::Attribution;
        require PS::MOPS::DC::History::Precovery;
        require PS::MOPS::DC::History::Identification;
        require PS::MOPS::DC::History::Link2;
        require PS::MOPS::DC::History::Removal;
        my $href;
        while ($href = $sth->fetchrow_hashref) {
            my $obj;
            if ($href->{event_type} eq $EVENT_TYPE_DERIVATION) {
                $obj = PS::MOPS::DC::History::Derivation::modchd_retrieve($inst, eventId => $href->{event_id});
            }
            elsif ($href->{event_type} eq $EVENT_TYPE_ATTRIBUTION) {
                $obj = PS::MOPS::DC::History::Attribution::modcha_retrieve($inst, eventId => $href->{event_id});
            }
            elsif ($href->{event_type} eq $EVENT_TYPE_PRECOVERY) {
                $obj = PS::MOPS::DC::History::Precovery::modchp_retrieve($inst, eventId => $href->{event_id});
            }
            elsif ($href->{event_type} eq $EVENT_TYPE_IDENTIFICATION) {
                $obj = PS::MOPS::DC::History::Identification::modchi_retrieve($inst, eventId => $href->{event_id});
            }
            elsif ($href->{event_type} eq $EVENT_TYPE_REMOVAL) {
                $obj = PS::MOPS::DC::History::Removal::modchr_retrieve($inst, eventId => $href->{event_id});
            }
            elsif ($href->{event_type} eq $EVENT_TYPE_LINK2) {
                $obj = PS::MOPS::DC::History::Link2::modch2_retrieve($inst, eventId => $href->{event_id});
            }
            push @histobjs, $obj;
        }
        return \@histobjs;
    }
    else {
        return undef;
    }
}


sub record {
    # Generic insert handler; accepts hashref from \%args or object.
    my ($self) = @_;
    my $dbh = $self->{_inst}->dbh;
    my @args = @{$self}{qw(
        eventType derivedobjectId fieldId orbitId orbitCode classification ssmId d3 d4 isLSD
    )};

    # Fetch the most recent modifying event for this derived object and
    # save it in the last_event_id so that we can more easily reconstruct
    # the orbit paper trail.
    my @last_evt = $dbh->selectrow_array(<<"EOF", undef, $self->{derivedobjectId});
select event_id
from history h
where h.derivedobject_id=? and h.classification in ('N', 'C')
order by event_id desc
limit 1
EOF
    push @args, (@last_evt ? $last_evt[0] : undef);

    my $rv = $dbh->do(<<"SQL", undef, @args);
insert into history (
event_type, event_time, derivedobject_id, field_id, orbit_id, orbit_code, classification, ssm_id, d3, d4, is_lsd, last_event_id
)
values (
?, sysdate(), ?, ?, ?, ?, ?, ?, ?, ?, ?, ?
)
SQL
    if ($rv) {
        $self->eventId($dbh->{'mysql_insertid'});            # establish orbit ID
    }
                                                                                                              
    return $rv;
}


1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

PS::MOPS::DC::History - Module for manipulating MOPS efficiency tables

=head1 SYNOPSIS

  use PS::MOPS::DC::History;

=head1 DESCRIPTION

Manipulate MOPS DC efficiency tables.

=head2 EXPORT

None by default.

=head1 SEE ALSO

PS::MOPS::DC::Orbit

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2004 Institute for Astronomy, University of Hawaii

=cut
