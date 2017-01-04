package PS::MOPS::DC::Detection;

use 5.008;
use strict;
use warnings;

use base qw(Exporter Class::Accessor);
our @EXPORT = qw(
    modcd_retrieve
    modcd_delete
    modcd_classifyDetections
    modcd_deserialize
);
our $VERSION = '0.01';


# Our stuff here.
__PACKAGE__->mk_accessors(qw(
    detId
    fieldId
    ra
    dec
    epoch
    mag
    refMag
    filter
    s2n
    raSigma
    decSigma
    magSigma
    length_deg
    orient_deg
    objectName
    obscode
    isSynthetic
    detNum
    status
    xyidx
    procId
    rawattr_v2
    rawattr_v3
    dev_x
    dev_y
    dev_id
));

use Params::Validate ':all';
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Iterator;


# Used when creating field or inserting by value.
our $by_value_validate_args = {
    detId => { default => undef },
    ra => 1,
    dec => 1,
    epoch => 1,
    mag => 1,
    refMag => 0,
    filter => 1,
    s2n => 1,
    raSigma => 0,
    decSigma => 0,
    magSigma => 0,
    length_deg => 0,
    orient_deg => 0,
    fieldId => 0,
    objectName => 0,
    obscode => 0,
    isSynthetic => 1,
    detNum => 0,
    status => 0,
    xyidx => 0,
    procId => 0,
    rawattr_v2 => 0,      # PS1-specific attributes
    rawattr_v3 => 0,      # ATLAS-specific attributes
    dev_x => 0,
    dev_y => 0,
    dev_id => 0,
};


our $selectall_sql;
our $DEC;
$selectall_sql = <<"SQL";
select
    d.det_id,
    d.field_id, d.object_name,
    d.orient_deg, d.dec_deg, d.dec_sigma_deg, d.epoch_mjd, d.filter_id, d.is_synthetic, d.det_num, d.status, d.length_deg,
    d.mag, d.mag_sigma, d.ra_deg, d.ra_sigma_deg, d.obscode, d.s2n, d.xyidx, d.proc_id, d.dev_x, d.dev_y, d.dev_id
SQL


sub new {
    my $pkg = shift;
    my $inst = shift;
    my %self = validate(@_, $by_value_validate_args);
    $self{_inst} = $inst;
    $self{status} ||= $DETECTION_STATUS_FOUND;              # default 
    return bless \%self;
}


sub _new_from_row {
    # Return a new Detection object from DBI row handle.
    my ($inst, $row) = @_;
    return undef if !$row;  # sanity check

    # Fill in defaults.
#    $row->{refMag} = $row->{mag} unless exists($row->{refmag});

    my $det;
    $det = PS::MOPS::DC::Detection->new(
        $inst,
        ra => $row->{ra_deg},
        dec => $row->{dec_deg},
        epoch => $row->{epoch_mjd},
        mag => $row->{mag},
        refMag => $row->{ref_mag},
        fieldId => $row->{field_id},
        s2n => $row->{s2n},
        raSigma => $row->{ra_sigma_deg},
        decSigma => $row->{dec_sigma_deg},
        magSigma => $row->{mag_sigma},
        orient_deg => $row->{orient_deg},
        length_deg => $row->{'length_deg'},
        filter => $row->{filter_id},
        objectName => $row->{object_name},
        obscode => $row->{obscode},
        isSynthetic => $row->{is_synthetic},
        detNum => $row->{det_num},
        status => $row->{status},
        xyidx => $row->{xyidx},
        procId => $row->{proc_id},
        dev_x => $row->{dev_x},
        dev_y => $row->{dev_y},
        dev_id => $row->{dev_id},
    );
    $det->{detId} = $row->{det_id};
    return bless $det;
}


sub _insert {
    # General routine to add detections to field, usable
    # via $self->addToField()
    # TODO: If there are multiple detections, turn off $dbh->autocommit
    # so we can slam into DB.
    my ($inst, $field, @dets) = @_;
    my $dbh = $inst->dbh;
    my @args;

    $inst->pushAutocommit(0);   # disable auto-commit for multiple insert
#    eval {
        my $sth;
        $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
insert into detections (
ra_deg, dec_deg, epoch_mjd, mag, ref_mag, filter_id, is_synthetic, det_num, status,
s2n, ra_sigma_deg, dec_sigma_deg, mag_sigma, orient_deg, length_deg, object_name, field_id, obscode,
xyidx, proc_id, dev_x, dev_y, dev_id
)
values (
?, ?, ?, ?, ?, ?, ?, ?, ?,
?, ?, ?, ?, ?, ?, ?, ?, ?,
?, ?, ?, ?, ?
)
SQL

        my $det;
        foreach $det (@dets) {
            @args = @{$det}{qw(
                ra dec epoch mag refMag filter isSynthetic detNum status
                s2n raSigma decSigma magSigma orient_deg length_deg
            )};
            push @args, $det->objectName;

            # Compute xyidx here if we weren't provided it.
            if (!defined($det->xyidx)) {
                $det->xyidx(mopslib_computeXYIdx($field, $det));
            }

            push @args, $field->fieldId, $field->obscode, $det->xyidx, $det->procId;
            $sth->execute(@args) or die "can't insert detection";
            $det->detId($dbh->{'mysql_insertid'});   # OK to set field ID in det now
            $det->fieldId($field->fieldId);
            $det->obscode($field->obscode);     # denormalized obscode in detection

            # If we have extended attributes, insert them here.
            if ($det->{rawattr_v3}) {
# Peak  Sky   chin var-krn Pstar Pkast Preal  star dstar mstar kast dkast
                $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
insert into det_rawattr_v3 (
    det_id,
    peak, sky, chin,
    pstar, preal,
    star, dstar, mstar
)
values (
    ?,
    ?, ?, ?,
    ?, ?,
    ?, ?, ?
)
SQL
                $sth->execute($det->detId, @{$det->rawattr_v3}) or die $sth->errstr;
             } 
             elsif ($det->{rawattr_v2}) {
                $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
insert into det_rawattr_v2 (
    det_id,
    psf_chi2, psf_dof,
    cr_sig, ext_sig,
    psf_major, psf_minor, psf_theta, psf_quality, psf_npix,
    moments_xx, moments_xy, moments_yy,
    n_pos, f_pos, ratio_bad, ratio_mask, ratio_all,
    flags,
    ipp_idet,
    psf_inst_flux, psf_inst_flux_sig,
    ap_mag, ap_mag_raw, ap_mag_radius, ap_flux, ap_flux_sig,
    peak_flux_as_mag,
    cal_psf_mag, cal_psf_mag_sig,
    sky, sky_sigma,
    psf_qf_perfect,
    moments_r1, moments_rh,
    kron_flux, kron_flux_err, kron_flux_inner, kron_flux_outer,
    diff_r_p, diff_sn_p, diff_r_m, diff_sn_m,
    flags2, n_frames
)
values (
    ?,
    ?, ?,
    ?, ?,
    ?, ?, ?, ?, ?,
    ?, ?, ?,
    ?,  , ?, ?, ?,
    ?,
    ?,
    ?, ?,
    ?, ?, ?, ?, ?,
    ?,
    ?, ?,
    ?, ?,
    ?,
    ?, ?,
    ?, ?, ?, ?,
    ?, ?, ?, ?,
    ?, ?
)
SQL
                $sth->execute($det->detId, @{$det->rawattr_v2}) or die $sth->errstr;
            }
        }
#    };
    $sth->finish;
    $dbh->commit;
$inst->popAutocommit();
die $@ if $@;
}


sub _insert_multiple {
# General routine to add multiple detections to field, via a single DB commit.
my ($inst, $field, $dets_aref) = @_;
my $dbh = $inst->dbh;
my @args;

$inst->atomic($dbh, sub {
    my $sth = $dbh->prepare(<<"SQL") or die "prepare of SQL statement failed\n";
insert into detections (
ra_deg, dec_deg, epoch_mjd, mag, ref_mag, filter_id, is_synthetic, det_num, status,
s2n, ra_sigma_deg, dec_sigma_deg, mag_sigma, orient_deg, length_deg, object_name, field_id, obscode,
xyidx, proc_id, dev_x, dev_y, dev_id)
values (
?, ?, ?, ?, ?, ?, ?, ?, ?,
?, ?, ?, ?, ?, ?, ?, ?, ?,
?, ?, ?, ?, ?
)
SQL
    my $sth_attr3 = $dbh->prepare(<<"SQL") or die $dbh->errstr;
insert into det_rawattr_v3 (
    det_id,
    peak, sky, chin,
    pstar, preal,
    star, dstar, mstar
)
values (
    ?,
    ?, ?, ?,
    ?, ?,
    ?, ?, ?
)
SQL

    my $sth_attr = $dbh->prepare(<<"SQL") or die $dbh->errstr;
insert into det_rawattr_v2 (
    det_id,
    psf_chi2, psf_dof,
    cr_sig, ext_sig,
    psf_major, psf_minor, psf_theta, psf_quality, psf_npix,
    moments_xx, moments_xy, moments_yy,
    n_pos, f_pos, ratio_bad, ratio_mask, ratio_all,
    flags,
    ipp_idet,
    psf_inst_flux, psf_inst_flux_sig,
    ap_mag, ap_mag_raw, ap_mag_radius, ap_flux, ap_flux_sig,
    peak_flux_as_mag,
    cal_psf_mag, cal_psf_mag_sig,
    sky, sky_sigma,
    psf_qf_perfect,
    moments_r1, moments_rh,
    kron_flux, kron_flux_err, kron_flux_inner, kron_flux_outer,
    diff_r_p, diff_sn_p, diff_r_m, diff_sn_m,
    flags2, n_frames
)
values (
    ?,
    ?, ?,
    ?, ?,
    ?, ?, ?, ?, ?,
    ?, ?, ?,
    ?, ?, ?, ?, ?,
    ?,
    ?,
    ?, ?,
    ?, ?, ?, ?, ?,
    ?,
    ?, ?,
    ?, ?,
    ?,
    ?, ?,
    ?, ?, ?, ?,
    ?, ?, ?, ?,
    ?, ?
)
SQL

    my $det;
    foreach $det (@{$dets_aref}) {
        @args = @{$det}{qw(
            ra dec epoch mag refMag filter isSynthetic detNum status
            s2n raSigma decSigma magSigma orient_deg length_deg
        )};
        push @args, $det->objectName;

        # Need to compute xyidx here.
        $det->xyidx(mopslib_computeXYIdx($field, $det));

        push @args, $field->fieldId, $field->obscode, $det->xyidx, $det->procId, $det->dev_x, $det->dev_y, $det->dev_id;
        $sth->execute(@args) or die "can't insert detection";
        $det->detId($dbh->{'mysql_insertid'});   # OK to set field ID in det now
        $det->fieldId($field->fieldId);
        $det->obscode($field->obscode);     # denormalized obscode in detection

        # If we have extended attributes, insert them here.
        if ($det->{rawattr_v3}) {
            $sth_attr3->execute($det->detId, @{$det->rawattr_v3}[0..7]) or die $sth_attr->errstr;
        }
        elsif ($det->{rawattr_v2}) {
            $sth_attr->execute($det->detId, @{$det->rawattr_v2}[0..43]) or die $sth_attr->errstr;
        }
    }
    $sth->finish;
});
}


sub addToField {
# Create the detection in the database, and associate to field.
my ($self, $field) = @_;
_insert($self->{_inst}, $field, $self);  # add one
}


sub getField {
# Method getField: return object representing this detection's
# owning field.
my ($self) = @_;
return $self->{_inst}->modcf_retrieve(fieldId => $self->fieldId);
}


sub modcd_retrieve {
# return detection specified by id
my $inst = shift;
my %args = validate(@_, { 
    detId => 0, 
    fieldId => 0, 
    fieldIds => 0, 
    minS2N => 0,
    trackletOnly => 0, 
    mjd => 0, 
    derivedobject => 0, 
    objectName => 0,
    mysql_store_result => 0,
    status => 0,
    dirtyLimit => 0,

    export_lsd => 0,
    nn => 0,
}); 
my $dbh = $inst->dbh;
my $row;
my $sth;
my $status = $args{status} || $DETECTION_STATUS_FOUND;
my $s2n_selector = $args{minS2N} ? "and s2n >= $args{minS2N}" : '';

my $dirty_selector = '';
my $d3_join = '';
if (defined($args{dirtyLimit})) {
    $dirty_selector = "and dirty <= $args{dirtyLimit}";
    $d3_join = "join det_rawattr_v3 d3 using(det_id)";
}

if ($args{detId}) {
    $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{detId});
$selectall_sql
from detections d
where d.det_id=?
SQL

    my $det;
    return _new_from_row($inst, $row);

    # These fields will be populated when asked for.
## obtain via modcd_* fns
##        $det->{orbits} = undef;
}
elsif ($args{fieldIds}) {
    # Retrieve all detections in this field.
    my $fields_str = join(', ', @{$args{fieldIds}});
    $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
$selectall_sql
from detections d $d3_join
where d.field_id in ($fields_str)
and status='$status' $s2n_selector $dirty_selector
order by d.epoch_mjd
SQL

    $sth->execute($args{fieldId}) or die $dbh->errstr;
    return PS::MOPS::DC::Iterator->new(sub {
        return _new_from_row($inst, $sth->fetchrow_hashref);
    });
}
elsif ($args{fieldId}) {
    my $sth;

    if ($args{trackletOnly}) {
        # Retrieve only detections that have tracklets.  Note that tracklet
        # status='Y' filters only "detected detections".
        $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
$selectall_sql
from detections d, tracklet_attrib ta, tracklets t
where d.det_id=ta.det_id
and ta.tracklet_id=t.tracklet_id
and d.field_id=?
and t.status='U'
order by d.epoch_mjd
SQL
    }
    else {
        # Retrieve all detections in this field.
        $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
$selectall_sql
from detections d $d3_join
where d.field_id=? $s2n_selector $dirty_selector
and status='$status'
order by d.epoch_mjd
SQL
    }

    $sth->execute($args{fieldId}) or die $dbh->errstr;
    return PS::MOPS::DC::Iterator->new(sub {
        return _new_from_row($inst, $sth->fetchrow_hashref);
    });
}
elsif ($args{objectName}) {
    my $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
$selectall_sql
from detections d
where d.object_name=?
and status='$status'
order by d.epoch_mjd
SQL

    $sth->execute($args{objectName}) or die $dbh->errstr;
    return PS::MOPS::DC::Iterator->new(sub {
        return _new_from_row($inst, $sth->fetchrow_hashref);
    });
}
elsif ($args{derivedobject}) {
    my $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
$selectall_sql
from detections d, tracklet_attrib ta, derivedobject_attrib da
where d.det_id=ta.det_id
and ta.tracklet_id=da.tracklet_id
and da.derivedobject_id=?
order by d.epoch_mjd
SQL

    $sth->execute($args{derivedobject}->derivedobjectId) or die $dbh->errstr;
    return PS::MOPS::DC::Iterator->new(sub {
        return _new_from_row($inst, $sth->fetchrow_hashref);
    });
}
elsif ($args{mjd}) {
    my $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
$selectall_sql
from detections d $d3_join
where status='$status' $s2n_selector $dirty_selector
and d.field_id in (
select field_id from `fields`
where epoch_mjd >= ? and epoch_mjd < ?)
SQL
    $sth->{mysql_use_result} = 1 unless $args{mysql_store_result};
    $sth->execute($args{mjd}, $args{mjd} + 1) or die $dbh->errstr;
    return PS::MOPS::DC::Iterator->new(sub {
        return _new_from_row($inst, $sth->fetchrow_hashref);
    });
}
elsif ($args{export_lsd}) {
    # Select all detections for the night that belong to a derived object; that is, 
    # belong to a tracklet with status='A'.
    my $nn = $args{nn} or die "no night number specified for export_lsd";
    my $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
$selectall_sql
from detections d
join fields f using (`field_id`)
join tracklet_attrib ta using (det_id)
join tracklets t using (tracklet_id)
where
t.status = '$TRACKLET_STATUS_ATTRIBUTED'
and f.nn=?
SQL
    $sth->{mysql_use_result} = 1 unless $args{mysql_store_result};
    $sth->execute($args{nn}) or die $dbh->errstr;
    return PS::MOPS::DC::Iterator->new(sub {
        return _new_from_row($inst, $sth->fetchrow_hashref);
    });
}
}


sub modcd_delete {
# return count of detections associated with this field (necessary? just use getDetectionIds)
my $inst = shift;
my %args = validate(@_, { detId => 1 });
my $dbh = $inst->dbh;
return $dbh->do(<<"SQL", undef, $args{detId});
delete
from detections
where det_id=?
SQL
}


sub modcd_classifyDetections {
# Given a list of detections, classify them as CLEAN, MIXED, BAD, or NONSYNTHETIC.
# We do this by inspecting the object name.  Names beginning with 'S' are Pan-STARRS
# synthetic objects; everything else is non-synthetic.  Although we do not look at
# the nonsynthetic names, they should all be $MOPS_NONSYNTHETIC_OBJECT_NAME.
my $inst = shift;       # unused
my (@dets) = @_;
my $det;
my $classification = 
    mopslib_effClassifyObjectNames(map { $_->{objectName} } @dets);

if ($classification eq $MOPS_EFF_CLEAN) {
    return $classification, $dets[0]->{objectName}; # clean, so return name as well
}
else {
    return $classification;
}
}


sub _chkundef {
return defined($_[0]) ? $_[0] : 'undef';
}


my @_serialization_members = qw(
detId 
fieldId
ra
dec
epoch
mag
refMag
filter
s2n
raSigma
decSigma
magSigma
length_deg
orient_deg
objectName
obscode
isSynthetic
detNum
status
xyidx
procId
);


sub serialize {
# Method which writes out a detection to a format we can use to create a detection later.
# For now this is used only by false detection stuff.
my ($self) = @_;

# The following operation creates a space-delimited string from each
# member of $self as listed in @_serialization_members, passing each
# item through _chkundef() first.
return join(" ", 
    'MIF-D',
    map { _chkundef($self->{$_}) } @_serialization_members
);
}


sub _dchkundef {
return $_[0] eq 'undef' ? undef : $_[0];
}


sub modcd_deserialize {
# Create a detection from serialization string.
my ($inst, $str) = @_;
return undef if (!$str or $str !~ /^MIF-D/);
my @stuff = split /\s+/, $str;
my %hash;

# The following operation bulk-assigns all the keys enumerated in
# @_serialization_members the values listed in @stuff, passing
# each item through _dchkundef() first.
@hash{
    @_serialization_members
} = map {
    _dchkundef($stuff[$_])
} 1..$#_serialization_members + 1;

return PS::MOPS::DC::Detection->new($inst, %hash);
}


1;
__END__

=head1 NAME

PS::MOPS::DC::Detection - Module for manipulating MOPS DC detection
objects.

=head1 SYNOPSIS

use PS::MOPS::DC::Detection ':all';
my $field = PS::MOPS::DC::Detection->new(
params here
);

=head1 DESCRIPTION

Stub documentation for PS::MOPS::DC::Detection, created by h2xs. 

=head2 EXPORT

None by default.

=head1 SEE ALSO

PS::MOPS::DC::Field
PS::MOPS::DC::Orbit

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2004 Institute for Astronomy, University of Hawaii

=cut
