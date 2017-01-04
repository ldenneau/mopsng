package PS::MOPS::DC::Tracklet;

use 5.008;
use strict;
use warnings;
use Carp;

use base qw(Exporter Class::Accessor);

our @EXPORT = qw(
    modct_create
    modct_delete
    modct_retrieve
    modct_countInField
    modct_selectDetracklets
    modct_selectTracklets
    modct_selectTrackletIdsByFieldId
    modct_getLastFieldId
    modct_getKillableTrackletIDs
    modct_killTrackletByID
    modct_getDetIds
);
our $VERSION = '0.01';

use Params::Validate ':all';
use Astro::SLA;
use List::Util qw(sum);
use PS::MOPS::Lib;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Detection;
use PS::MOPS::DC::Iterator;
use PS::MOPS::DC::Field;

# Our stuff here.
__PACKAGE__->mk_accessors(qw(
    trackletId
    fieldId
    vRA
    vDEC
    vRASigma
    vDECSigma
    vTot
    posAng
    gcr_arcsec
    extEpoch
    extRA
    extDEC
    extMag
    probability
    digest
    detections
    status  
    classification
    ssmId
    knownId
));

# Used when creating or inserting by value.
our $new_validate_args = {
    trackletId => 0,
    fieldId => 0,
    detections => { type => ARRAYREF },
    vRA => 0,
    vDEC => 0,
    vRASigma => 0,
    vDECSigma => 0,
    vTot => 0,
    posAng => 0,
    gcr_arcsec => 0,
    extEpoch => 0,
    extRA => 0,
    extDEC => 0,
    extMag => 0,
    probability => 0,
    digest => 0,
    detections => 0,
    status => 0,
    classification => 1,
    ssmId => 0,
    knownId => 0,
};

# Build a regexp to test for correct status.
our $status_regexp = qr/^[${TRACKLET_STATUS_UNATTRIBUTED}${TRACKLET_STATUS_ATTRIBUTED}${TRACKLET_STATUS_KILLED}${TRACKLET_STATUS_KNOWN}]$/;


our $selectall_sql = <<"SQL";
select
    tracklet_id,
    field_id,
    v_ra,
    v_dec,
    v_tot,
    v_ra_sigma,
    v_dec_sigma,
    pos_ang_deg,
    gcr_arcsec,
    ext_epoch,
    ext_ra,
    ext_dec,
    ext_mag,
    probability,
    digest,
    status,
    classification,
    ssm_id,
    known_id
from tracklets
SQL

# String for selection "detracklets" -- tracklet-detection compositions that look like
# detections.  The ID column, normally used by detId, is replaced by a trackletId.
our $selectall_dt_sql = <<"SQL";
select 
    t.tracklet_id tracklet_id, 
    d.field_id field_id,
    d.ra_deg ra,
    d.dec_deg decl,
    d.epoch_mjd epoch,
    d.mag mag,
    d.ref_mag ref_mag,
    d.s2n s2n,
    d.ra_sigma_deg ra_sigma_deg,
    d.dec_sigma_deg dec_sigma_deg,
    d.mag_sigma mag_sigma,
    d.orient_deg orient_deg,
    d.length_deg length_deg,
    d.object_name object_name,
    d.is_synthetic is_synthetic,
    d.obscode obscode,
    d.filter_id filter
SQL


##sub _avg {
##    # Process input list, return avg.  If number of items is zero, die.
##    die "empty list" unless @_ > 0;
##    my $sum = 0;
##
##    $sum += $_ foreach @_;
##    return $sum / scalar(@_);
##}


sub new {   
    # Instantiate a tracklet object.  If trackletId is undef, then we are creating a
    # new in-memory tracklet and we need to compute extrapolated parameters.  If
    # trackletId is specified, we simply need to populate the object.
    my $pkg = shift;
    my $inst = shift;
    my %self = validate(@_, $new_validate_args);
    $self{_inst} = $inst;

    # Defaults
    if (!$self{trackletId}) {
        # trackletId not specified.
        $self{probability} ||= 1;
        $self{digest} ||= 0;
        $self{status} ||= $TRACKLET_STATUS_UNATTRIBUTED;

        # Scan detections.  TODO: handle accelerations, etc. with 3+ dets.
        my $nd = scalar @{$self{detections}};     # num detections
        my @things;
        foreach my $det (@{$self{detections}}) {
            push @things, { ra => $det->ra, dec => $det->dec, epoch => $det->epoch, fieldId => $det->fieldId };
        }

        # Filter conversions so we store avg V mag.
        my $v2filt = $inst->getConfig()->{site}->{v2filt} || die "can'tget a v2filt for filter conversion";

        # Sort by epoch; get deltas for min/max epoch vals.
        my @sorted = sort { $a->{epoch} <=> $b->{epoch} } @things;
        my $first = $sorted[0];
        my $last = $sorted[-1];
        my $dra;
        my $ddec;
        my $delta_epoch_days = $last->{epoch} - $first->{epoch};  # delta epoch
        if ($delta_epoch_days != 0) {
            $dra = mopslib_dang($last->{ra}, $first->{ra});     # delta angle between RAs
            $ddec = mopslib_dang($last->{dec}, $first->{dec});  # delta angle between DECs
            $self{extEpoch} = $first->{epoch} + $delta_epoch_days / 2;
            #$self{extRA} = $first->{ra} + $dra / 2;
            #$self{extDEC} = $first->{dec} + $ddec / 2;
            ($self{extRA}, $self{extDEC}) = mopslib_normalizeRADEC(
                $first->{ra} + $dra / 2,
                $first->{dec} + $ddec / 2
            );
            $self{vRA} = $dra / $delta_epoch_days;                        # veloctiy in RA
            $self{vDEC} = $ddec / $delta_epoch_days;                      # veloctiy in DEC

            # avg V mag
            $self{extMag} = sum(map { mopslib_filt2V($_->{mag}, $_->{filter}, $v2filt) } @{$self{detections}}) / $nd;
    
            # Store total magnitude of velocity.  We will index on this
            # and segment our linking strategy according to bands
            # of tracklet velocities.
            $self{vTot} = slaDsep(
                $first->{ra} / $DEG_PER_RAD, $first->{dec} / $DEG_PER_RAD, 
                $last->{ra} / $DEG_PER_RAD, $last->{dec} / $DEG_PER_RAD
            ) * $DEG_PER_RAD / $delta_epoch_days;
        }
        else {
            $self{extEpoch} = $first->{epoch};
            $self{extRA} = $first->{ra};
            $self{extDEC} = $first->{dec};
            $self{vRA} = 0;
            $self{vDEC} = 0;
            $self{extMag} = mopslib_filt2V($first->{mag}, $first->{filter}, $v2filt)
        }
        $self{vRASigma} = 0;
        $self{vDECSigma} = 0;

        # Compute position angle of tracklet.  This is the direction from the first detection
        # to the last detection, with zero to the north, and PI/2 due east.
        $self{posAng} = slaBear(
            $first->{ra} / $DEG_PER_RAD, $first->{dec} / $DEG_PER_RAD, 
            $last->{ra} / $DEG_PER_RAD, $last->{dec} / $DEG_PER_RAD
        ) * $DEG_PER_RAD;

        # Assign field ID from last detection.
        $self{fieldId} = $last->{fieldId};
        
        return bless \%self;
    }
    return bless \%self;
}


# Methods
sub insert {
    my ($self) = @_;
    my $inst = $self->{_inst};
    my $dbh = $inst->dbh;
    my $sth;
    my $trackletId;

    $inst->pushAutocommit(0);

    eval {
        $sth = $dbh->prepare(<<"SQL") or die "prepare failed";
insert into tracklets (
field_id,
v_ra, v_dec, v_tot, v_ra_sigma, v_dec_sigma, pos_ang_deg, gcr_arcsec,
ext_epoch, ext_ra, ext_dec, ext_mag, 
probability, digest, status, classification, ssm_id, known_id
)
values (
?,
?, ?, ?, ?, ?, ?, ?,
?, ?, ?, ?, 
?, ?, ?, ?, ?, ?
)
SQL
        $sth->execute(@{$self}{qw(
            fieldId vRA vDEC vTot vRASigma vDECSigma posAng gcr_arcsec
            extEpoch extRA extDEC extMag 
            probability digest status classification ssmId knownId
        )});
        $sth->finish;
        $trackletId = $dbh->{'mysql_insertid'} or die "bogus mysql_insertid";

        # Now insert associated detections.
        $sth = $dbh->prepare(<<"SQL") or die "prepare failed";
insert into tracklet_attrib values ( ?, ? )
SQL
        foreach my $det (@{$self->detections}) {
            $sth->execute($trackletId, $det->detId);
        }
        $sth->finish;
        $dbh->commit;
    };
    if ($@) {
        $dbh->rollback;
        croak $@;
        $trackletId = undef;
    }

    $inst->popAutocommit();
    $self->{trackletId} = $trackletId;
    return $trackletId;
}


sub delete {
    my ($self) = shift;
    
}


sub status {
    my ($self, $status) = @_;
    my $inst = $self->{_inst};
    if ($status) {
        croak "bogus status $status" unless $status =~ $status_regexp;  # /^[UAKW]$/;
        my $dbh = $inst->dbh;
        $dbh->do(<<"SQL", undef, $status, $self->trackletId) or croak $dbh->errstr;
update tracklets set status=? where tracklet_id=?
SQL
        $self->{status} = $status;
    }
    else {
        return $self->{status};
    }
}


sub probability {
    my ($self, $probability) = @_;
    my $inst = $self->{_inst};
    if (defined($probability)) {
        my $dbh = $inst->dbh;
        $dbh->do(<<"SQL", undef, $probability, $self->trackletId) or croak $dbh->errstr;
update tracklets set probability=? where tracklet_id=?
SQL
        $self->{status} = $probability;
    }
    else {
        return $self->{probability};
    }
}


sub digest {
    my ($self, $digest) = @_;
    my $inst = $self->{_inst};
    if (defined($digest)) {
        my $dbh = $inst->dbh;
        $dbh->do(<<"SQL", undef, $digest, $self->trackletId) or croak $dbh->errstr;
update tracklets set digest=? where tracklet_id=?
SQL
        $self->{digest} = $digest;
    }
    else {
        return $self->{digest};
    }
}


sub gcr_arcsec {
    my ($self, $gcr_arcsec) = @_;
    my $inst = $self->{_inst};
    if (defined($gcr_arcsec)) {
        my $dbh = $inst->dbh;
        $dbh->do(<<"SQL", undef, $gcr_arcsec, $self->trackletId) or croak $dbh->errstr;
update tracklets set gcr_arcsec=? where tracklet_id=?
SQL
        $self->{status} = $gcr_arcsec;
    }
    else {
        return $self->{gcr_arcsec};
    }
}


sub knownId {
    # Update known ID for tracklet.
    my ($self, $knownId) = @_;
    my $inst = $self->{_inst};
    if ($knownId) {
        my $dbh = $inst->dbh;
        $dbh->do(<<"SQL", undef, $knownId, $self->trackletId) or croak $dbh->errstr;
update tracklets set known_id=? where tracklet_id=?
SQL
        $self->{knownId} = $knownId;
    }
    else {
        return $self->{knownId};
    }
}


sub _fetch_detections {
    # Fetch detections from database, in order of ascending epoch.
    my ($inst, $trackletId) = @_;

    # An MJD was specified; get lunation's worth.
    my @dets;
    my $dbh = $inst->dbh;
    my $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
select det_id
from tracklet_attrib
where tracklet_id=?
SQL
    $sth->execute($trackletId) or croak $dbh->errstr;
    my $aref;
    while ($aref = $sth->fetchrow_arrayref) {
        push @dets, modcd_retrieve($inst, detId => $aref->[0]);
    }
    @dets = sort { $a->epoch <=> $b->epoch } @dets; # sort by epoch ascending
    return @dets;
}


# Static
##sub modct_create {
##    # Create a new tracklet, given a list of detections and other parameters.
##    my $pkg = shift;
##    my $trk = $pkg->new(@_);
##    $trk->insert();
##}


sub modct_countInField {
    my $inst = shift;
    my $field_id = shift;
    my $dbh = $inst->dbh;
    my $sql = <<"SQL";
select count(*) from tracklets where field_id=?
SQL
    my ($num) = $dbh->selectrow_array('select count(*) from tracklets where field_id=?', undef, $field_id);
    return $num;
}


sub modct_retrieve {
    # Return a single tracklet object or iterator of tracklets.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, {
        trackletId => 0,
        detectionId => 0,
        fieldId => 0,               # all tracklets that terminate with this fieldId
        derivedobjectId => 0,
        noDets => 0,                # don't retrieve detection list
        status => 0,                # specify required status for tracklets
    });

    if ($args{trackletId}) {
        my $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
select t.tracklet_id, t.field_id, t.v_ra, t.v_dec, t.v_tot, t.v_ra_sigma, t.v_dec_sigma, t.pos_ang_deg, t.gcr_arcsec, t.ext_epoch, t.ext_ra, t.ext_dec, t.ext_mag, t.probability, digest, t.status, t.classification, t.ssm_id, t.known_id
from tracklets t
where t.tracklet_id=?
SQL
        $sth->execute($args{trackletId}) or croak $dbh->errstr;
        return _new_from_row($inst, $sth->fetchrow_hashref, $args{noDets}); 
    }
    elsif ($args{fieldId}) {
        my @where = 't.field_id=?';
        my $where_str;
        if (defined($args{status})) {
            push @where, qq[t.status='$args{status}'];
        }

        $where_str = 'where ' . join(' and ', @where);
        my $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
select t.tracklet_id, t.field_id, t.v_ra, t.v_dec, t.v_tot, t.v_ra_sigma, t.v_dec_sigma, t.pos_ang_deg, t.gcr_arcsec, t.ext_epoch, t.ext_ra, t.ext_dec, t.ext_mag, t.probability, digest, t.status, t.classification, t.ssm_id, t.known_id
from tracklets t
$where_str
SQL
        $sth->execute($args{fieldId}) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref, $args{noDets});
        });
    }
    elsif ($args{detectionId}) {
        my $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
select t.tracklet_id, t.field_id, t.v_ra, t.v_dec, t.v_tot, t.v_ra_sigma, t.v_dec_sigma, t.pos_ang_deg, t.gcr_arcsec, t.ext_epoch, t.ext_ra, t.ext_dec, t.ext_mag, t.probability, digest, t.status, t.classification, t.ssm_id, t.known_id
from tracklets t, tracklet_attrib ta
where t.tracklet_id=ta.tracklet_id
and ta.det_id=?
SQL
        $sth->execute($args{detectionId}) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref, $args{noDets});
        });
    }
    elsif ($args{derivedobjectId}) {
        my $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
select t.tracklet_id, t.field_id, t.v_ra, t.v_dec, t.v_tot, t.v_ra_sigma, t.v_dec_sigma, t.pos_ang_deg, t.gcr_arcsec, t.ext_epoch, t.ext_ra, t.ext_dec, t.ext_mag, t.probability, digest, t.status, t.classification, t.ssm_id, t.known_id
from tracklets t, derivedobject_attrib da
where t.tracklet_id=da.tracklet_id
and da.derivedobject_id=?
SQL
        $sth->execute($args{derivedobjectId}) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref, $args{noDets});
        });
    }
}


sub modct_delete {
    my $inst = shift;
    my $dbh = $inst->dbh;
}


sub _new_dt_from_row {
    # Return something that resembles a tracklet for selectTracklets purposes.
    # This means it has an ID, RA, DEC, extRA, extDEC, epoch, objectName.
    my ($inst, $row) = @_;
    return undef if !$row;  # sanity check

    my $trackletId = $row->{tracklet_id};
    my $det = PS::MOPS::DC::Detection->new(
            $inst,
            detId => $row->{tracklet_id},
            fieldId => $row->{field_id},
            ra => $row->{ra},
            dec => $row->{decl},
            epoch => $row->{epoch},
            mag => $row->{mag},
            refMag => $row->{ref_mag},
            filter => $row->{filter},
            s2n => $row->{s2n},
            raSigma => $row->{ra_sigma_deg},
            decSigma => $row->{dec_sigma_deg},
            magSigma => $row->{mag_sigma},
            orient_deg => $row->{angle},
            length_deg => $row->{'length'},
            objectName => $row->{object_name},
            obscode => $row->{obscode},
            isSynthetic => $row->{is_synthetic},
    );
    $det->{obscode} = $row->{obscode}; # chummy
    return $det;
}

sub _new_from_row {
    # Return a Tracklet object from DBI row handle.
    my ($inst, $row, $noDets) = @_;
    return undef if !$row;  # sanity check

    my $trackletId = $row->{tracklet_id};
    my @dets;
    unless ($noDets) {
        @dets = _fetch_detections($inst, $trackletId);
    }

    return PS::MOPS::DC::Tracklet->new(
        $inst,
        trackletId => $trackletId,
        fieldId => $row->{field_id},
        vRA => $row->{v_ra},
        vDEC => $row->{v_dec},
        vTot => $row->{v_tot},
        vRASigma => $row->{v_ra_sigma},
        vDECSigma => $row->{v_dec_sigma},
        posAng => $row->{pos_ang_deg},
        gcr_arcsec => $row->{gcr_arcsec},
        extEpoch => $row->{ext_epoch},
        extRA => $row->{ext_ra},
        extDEC => $row->{ext_dec},
        extMag => $row->{ext_mag},
        probability => $row->{probability},
        digest => $row->{digest},
        classification => $row->{classification},
        ssmId => $row->{ssm_id},
        knownId => $row->{known_id},
        status => $row->{status},
        detections => \@dets,
    );
}


sub modct_getKillableTrackletIDs {
    # Return an ARRAYREF of tracklet IDs for tracklets that should be "killed"; that is,
    # the tracklets
    #   * have status 'U'
    #   * contain detections belonging to other status='A' tracklets from EON objs
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, {
        eon_status => 1,
    });

    # An MJD was specified; get lunation's worth.
    my $aref;
    $aref = $dbh->selectcol_arrayref(<<"SQL");
select
    t1.tracklet_id
from
    tracklets t1
    join tracklet_attrib ta1 on t1.tracklet_id=ta1.tracklet_id
    join detections d on ta1.det_id=d.det_id
    join tracklet_attrib ta2 on d.det_id=ta2.det_id
    join tracklets t2 on ta2.tracklet_id=t2.tracklet_id
    join derivedobject_attrib doa on ta2.tracklet_id=doa.tracklet_id
    join eon_queue eonq on doa.derivedobject_id=eonq.derivedobject_id
where
    t1.status='U'
    and t2.status='A'
    and eonq.status='$EONQUEUE_STATUS_PRECOVERED'
SQL
    if (!defined($aref)) {
        croak "selectcol_arrayref failed";
    }
    return $aref;
}


sub modct_killTrackletByID {
    # Return an ARRAYREF of tracklet IDs for tracklets that should be "killed"; that is,
    # the tracklets
    #   * have status 'U'
    #   * contain detections belonging to other status='A' tracklets from EON objs
    my ($inst, $tid) = @_;
    my $status = $TRACKLET_STATUS_KILLED;
    my $dbh = $inst->dbh;
    $dbh->do(<<"SQL", undef, $status, $tid) or croak $dbh->errstr;
update tracklets set status=? where tracklet_id=?
SQL
}


sub modct_getDetIds {
    # Return a list of detection IDs for the tracklet with the specified tracklet ID.
    my ($inst, $tid) = @_;
    my $dbh = $inst->dbh;
    my $aref = $dbh->selectcol_arrayref(<<"SQL", undef, $tid) or croak $dbh->errstr;
select det_id from tracklet_attrib where tracklet_id=? order by det_id
SQL
    return @{$aref};
}


sub modct_selectTracklets {
    # Return an iterator object for all tracklets meeting our field specification requirements.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, {
        oc => 0,
        mjd => 0,
        mjd1 => 0,
        mysql_store_result => 0,
        noDets => 0,
    });

    if ($args{mjd}) {
        # An MJD was specified; get lunation's worth.
        my $sth;
        $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
select t.tracklet_id, t.field_id, t.v_ra, t.v_dec, t.v_tot, t.v_ra_sigma, t.v_dec_sigma, t.pos_ang_deg, t.gcr_arcsec, t.ext_epoch, t.ext_ra, t.ext_dec, t.ext_mag, t.probability, digest, t.status, t.classification, t.ssm_id, t.known_id
from tracklets t
where t.ext_epoch >= ? and t.ext_epoch < ?
SQL
        $sth->{mysql_use_result} = 1 unless $args{mysql_store_result};
        $sth->execute($args{mjd}, $args{mjd} + 30) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref, $args{noDets});
        });
    }
    elsif ($args{mjd1}) {
        # An MJD was specified; get lunation's worth.
        my $sth;
        $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
select t.tracklet_id, t.field_id, t.v_ra, t.v_dec, t.v_tot, t.v_ra_sigma, t.v_dec_sigma, t.pos_ang_deg, t.gcr_arcsec, t.ext_epoch, t.ext_ra, t.ext_dec, t.ext_mag, t.probability, digest, t.status, t.classification, t.ssm_id, t.known_id
from tracklets t
where t.ext_epoch >= ? and t.ext_epoch < ?
SQL
        $sth->{mysql_use_result} = 1 unless $args{mysql_store_result};
        $sth->execute($args{mjd1}, $args{mjd1} + 1) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref, $args{noDets});
        });
    }
}


sub modct_selectTrackletIdsByFieldId {
    # Return an ARRAYREF for all tracklet IDs containing detections in specified field.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, {
        fieldId => 1,
        minv_degperday => 0,
        maxv_degperday => 0,
        status => 0,
        anyInField => 0,            # if set, return tracklets with any det (instead of last det) in field
    });

    my $and = '';
    if (defined($args{minv_degperday}) and defined($args{maxv_degperday})) {
        $and = "and t.v_tot >= $args{minv_degperday} and t.v_tot <= $args{maxv_degperday}";
    }

    my $status = "and t.status='$TRACKLET_STATUS_UNATTRIBUTED'";
    if (exists($args{status})) {
        if (defined($args{status})) {
            $status = "and t.status='$args{status}'";
        }
        else {
            $status = '';
        }
    }

    my $det_sel = $args{anyInField} ? 
        "and d.field_id=$args{fieldId}" :       # join with any detection's fieldId = $args{fieldId}
        "and t.field_id=$args{fieldId}";        # join with last detection's fieldId = $args{fieldId}

    return $dbh->selectcol_arrayref(<<"SQL") or croak $dbh->errstr;
select ta.tracklet_id
from tracklets t, tracklet_attrib ta, detections d
where ta.det_id=d.det_id
and t.tracklet_id=ta.tracklet_id
$det_sel
$status
$and
SQL
}


sub modct_selectDetracklets {
    # Return an iterator object for "detracklets"; detection-like objects that have some
    # short-circuited other information to reduce complete object fetches from PSMOPS.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, {
# These are not really supported any longer.
#        ocnum => 0,
#        mjd => 0,               # MJD + 30 days
#        mjd1 => 0,              # single MJD
        trackletId => 0,        # by tracklet ID
        fieldId => 0,           # by single terminating field ID

        minv_degperday => 0,    # specify min velocity
        maxv_degperday => 0,    # specify max velocity

        # Selection by center, radius and velocity
        target_field_id => 0,
        target_field => 0,
        center_ra_deg => 0,
        center_dec_deg => 0,
        radius_deg => 0,
        minv => 0,
        maxv => 0,
        start_epoch_mjd => 0,
        end_epoch_mjd => 0,
        field_radius_deg => 0,
        match_target_field_status => 0, # if set, require tracklets to come from fields with same status as target
        min_probability => 0,
    });

    $args{min_probability} ||= 0;       # default

    my $sql;
    my $vel_and = '';
    if (defined($args{minv_degperday}) and defined($args{maxv_degperday})) {
        $vel_and = "and t.v_tot >= $args{minv_degperday} and t.v_tot <= $args{maxv_degperday}";
    }

    if ($args{mjd}) {
        # An MJD was specified; get lunation's worth.
        croak "mjd is unsupported";
        my $sth;
        $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
$selectall_dt_sql
from tracklets t, tracklet_attrib ta, detections d, `fields` f
where t.ext_epoch >= ? and t.ext_epoch < ?
and t.probability >= ?
and t.classification <> '$MOPS_EFF_UNFOUND'
and t.tracklet_id=ta.tracklet_id
and ta.det_id=d.det_id
and d.field_id=f.field_id
$vel_and
SQL
        $sth->{mysql_use_result} = 1;
        $sth->execute($args{mjd}, $args{mjd} + 30, $args{min_probability}) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_dt_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    elsif ($args{mjd1}) {
        # An MJD was specified; get single night's worth.
        croak "mjd1 is unsupported";
        my $sth;
        $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
$selectall_dt_sql
from tracklets t, tracklet_attrib ta, detections d, `fields` f
where t.ext_epoch >= ? and t.ext_epoch < ?
and t.probability >= ?
and t.classification <> '$MOPS_EFF_UNFOUND'
and t.tracklet_id=ta.tracklet_id
and ta.det_id=d.det_id
and d.field_id=f.field_id
$vel_and
SQL
        $sth->{mysql_use_result} = 1;
        $sth->execute($args{mjd1}, $args{mjd1} + 1, $args{min_probability}) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_dt_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    elsif ($args{fieldId}) {
        # fieldId specified
        my $sth;
        $sql = <<"SQL";
$selectall_dt_sql
from tracklets t, tracklet_attrib ta, detections d
where t.field_id=?
and t.probability >= ?
and t.status='$TRACKLET_STATUS_UNATTRIBUTED'
and t.classification <> '$MOPS_EFF_UNFOUND'
and t.tracklet_id=ta.tracklet_id
and ta.det_id=d.det_id
$vel_and
SQL
        $sth = $dbh->prepare($sql) or croak $dbh->errstr;
        $sth->execute($args{fieldId}, $args{min_probability}) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_dt_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    elsif ($args{trackletId}) {
        # TrackletId specified
        my $sth;
        $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
$selectall_dt_sql
from tracklets t, tracklet_attrib ta, detections d, `fields` f
where t.tracklet_id=?
and t.tracklet_id=ta.tracklet_id
and ta.det_id=d.det_id
and d.field_id=f.field_id
$vel_and
SQL
        $sth->execute($args{trackletId}) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_dt_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    elsif ($args{target_field} or $args{target_field_id}) {
        # Select tracklets using special selection parameters for linking.  Conceptually
        # we want to select the following tracklets for this link pass:
        # 1.  All tracklets that terminate in the target field
        # 2.  All tracklets from previous nights with some velocity cone
        # Using this method we should guarantee that we see no duplicated
        # tracks in the entire night, since for two tracks to be identical
        # they have to terminate in the same field, and all of these potential
        # tracks are processed by the same link pass.
        die "field_radius_deg must be specified" unless $args{field_radius_deg};
        die "maxv_degperday must be specified" unless $args{maxv_degperday};
        my $target_field = $args{target_field} || modcf_retrieve($inst, fieldId => $args{target_field_id});
        $args{minv_degperday} = 0 unless defined($args{minv_degperday});

        # Match target field status.  This mod allows "layering" or "striping" of field data
        # so that a layer can be linked separately from previous layers (e.g., TALCS).
        my $match_status_str = '';
        if ($args{match_target_field_status}) {
            $match_status_str = sprintf "and f.status='%s'", $target_field->status;
        }

        # Select tracklets.  This painful query selects tracklets from fields
        # in our expanding velocity cone, in a smart way: only those tracklets
        # in distant fields that have sufficient velocity are selected.  Also
        # we join with `fields` so that we are only scanning tracklets in relevant fields,
        # not all fields each night.

        # field_radius_deg says how big the fields are, which has to be accounted
        # for in the tracklet proximity calculation.  A tracklet is considered "adjacent" if
        # it can within a field radius of the target field center at the time of
        # the target field.  Another field can be considered adjacent or overlapping
        # if its field center is within 2 radii considering maxv_degperday (in other
        # words, an object moving at maxv_degperday could have appeared in both
        # fields).  The 1.2 factor is a slop factor allowing for velocities to be
        # changing during the lunation.

        #my $saved_mysql_result_state = $dbh->{mysql_use_result};
#        $dbh->{mysql_use_result} = 1;

        my $sql = sprintf <<"SQL", $target_field->fieldId, @args{qw( start_epoch_mjd end_epoch_mjd )}, $target_field->epoch, $target_field->dec, $target_field->dec, $target_field->ra, $target_field->epoch, $target_field->epoch, $target_field->fieldId, $target_field->dec, $target_field->dec, $target_field->ra, $target_field->epoch;
$selectall_dt_sql
from tracklets t, tracklet_attrib ta, detections d
where t.tracklet_id=ta.tracklet_id
and ta.det_id=d.det_id
and t.status='$TRACKLET_STATUS_UNATTRIBUTED' 
and t.classification <> '$MOPS_EFF_UNFOUND'
and t.v_tot < $args{maxv_degperday}
and t.v_tot > $args{minv_degperday}
and t.probability >= $args{min_probability}
and t.field_id in (
    /* restrict search to target field or fields in velocity cone */
    select f.field_id
    from `fields` f
    where 
        f.field_id=%d
        $match_status_str
        or (f.epoch_mjd >= %f and f.epoch_mjd <= %f and f.epoch_mjd <= %f
            and (abs(
                degrees(
                    /* spherical distance between fields */
                    acos(least(1.0, /* handle slight roundoff error causing arg > 1.0 */
                        sin(radians(f.dec_deg)) * sin(radians(%f))
                        + cos(radians(f.dec_deg)) * cos(radians(%f)) * cos(radians(f.ra_deg - %f))
                    ))
                )
            /* minus a field radius for src/target fields */
            ) - 2 * $args{field_radius_deg} * 1.2
        ) / abs(f.epoch_mjd - %f) < $args{maxv_degperday}
    )
)
/* restrict tracklets to those in velocity cone fast enough to appear in starting
and ending fields */
and t.ext_epoch < %f
and (
    t.field_id=%d or t.v_tot * 1.3 /* velocity variability */ > (
        abs(
            degrees(
                /* spherical distance from tracklet to target field center */
                acos(least(1.0, /* handle slight roundoff error causing arg > 1.0 */
                    sin(radians(t.ext_dec)) * sin(radians(%f))
                    + cos(radians(t.ext_dec)) * cos(radians(%f)) * cos(radians(t.ext_ra - %f))
                ))
            )
        /* minus field radius */
        ) - $args{field_radius_deg} * 1.2
    ) / abs(t.ext_epoch - %f)
)
order by t.tracklet_id, t.ext_epoch
SQL

        my $sth = $dbh->prepare($sql) or croak $dbh->errstr;
        eval {
            $sth->execute() or croak $dbh->errstr;
        };
        if ($@ and $@ =~ /Lost connection/) {
            # long query timed out. Retry.
            sleep 20;
            print STDERR "Retry...\n";
            $sth->execute() or croak $dbh->errstr;
        }

        return PS::MOPS::DC::Iterator->new(sub {
            return _new_dt_from_row($inst, $sth->fetchrow_hashref);
        });
    }
}


sub modct_getLastFieldId {
    # Given a list of tracklets, return the field ID of the
    # tracklet whos extEpoch is latest in time.
    my $inst = shift;   # dummy

    my @ary = @_;
    return undef unless @ary;

    # Schwartzian transform!
    my @tsorted =
        map { $_->[0] } 
        sort { $a->[1] <=> $b->[1] } 
        map { [$_, $_->extEpoch] } 
        @ary;

    return $tsorted[-1]->fieldId;
}



1;
__END__

=head1 NAME

PS::MOPS::DC::Tracklet - Perl extension for manipulating MOPS tracklets

=head1 SYNOPSIS

  use PS::MOPS::DC::Tracklet;

=head1 DESCRIPTION

Manipulate MOPS "tracklet" data.  Tracklets are simply tuples, or aggretations
of detection data.  Raw detections are processed by FindTracklets and the
detections are grouped into Tracklets.

=item modct_create

Static method to create a new tracklet from a list of detections.

=item modct_delete

Delete a tracklet from PSMOPS (unimplemented currently).

=item modct_retreive

Retrieve a single tracklet or an iterator returning tracklets
based on various input criteria.

=item modct_selectDetracklets

Retrieve an iterator returning "detracklets", or detection/tracklet
hybrid objects, based on various input criteria.

=item modct_selectTracklets

Retrieve an iterator returning tracklets based on various input criteria.

=item modct_selectTrackletIdsByFieldId

Return an ARRAYREF containing tracklet IDs of all tracklets with at
least one detection in the specified field.

=item modct_getLastFieldId

Utility routing to return the latest Field ID from a list of tracklets.

=head2 EXPORT

modct_create
modct_delete
modct_retrieve
modct_selectDetracklets
modct_selectTracklets
modct_selectTrackletIdsByFieldId
modct_getLastFieldId

=head1 SEE ALSO

PS::MOPS::DC
PS::MOPS::DC::Orbit
PS::MOPS::DC::Observation
PS::MOPS::DC::Detection

=head1 AUTHOR

Larry Denneau, denneau@ifa.hawaii.edu

=head1 COPYRIGHT AND LICENSE

Copyright 2005 by Larry Denneau, Institute for Astronomy, University of Hawaii.

=cut

