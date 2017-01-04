package PS::MOPS::DC::Field;

use 5.008;
use strict;
use warnings;
use Carp;

use base qw(Exporter Class::Accessor);

our @EXPORT = qw(
    modcf_retrieve
    modcf_retrieveSurveyModes
    modcf_delete
);

our $VERSION = '0.01';


# Field member descriptions:  see POD below.
__PACKAGE__->mk_accessors(qw(
    fieldId
    de
    dec
    decSigma
    epoch
    ocnum
    nn
    filter
    limitingMag
    obscode
    ra
    raSigma
    surveyMode
    timeStart
    timeStop
    status
    parentId
    fpaId
    expId
    diffId
    xyidxSize
    pa_deg
    FOV_deg
    se_deg
    eb_deg
    gb_deg
));

use Params::Validate qw(:all);
use PS::MOPS::Lib qw(:all);
use PS::MOPS::Constants qw(:all);
use PS::MOPS::DC::Iterator;
use PS::MOPS::Admin qw(update_runs_table);

# Dummy filter until we're using real filters.  'DD' is inserted
# automatically when the instance is created.
our $DUMMY_FILTER_ID = 'DD';
our $DUMMY_SURVEY_MODE = 'DD';


# Used when creating field or inserting by value.
our $by_value_validate_args = {
    de => {type => ARRAYREF},
    dec => 1,
    decSigma => 1,
    epoch => 1,
    ocnum => 0,
    nn => 0,
    filter => 1,
    limitingMag => 1,
    obscode => 1,
    ra => 1,
    raSigma => 1,
    surveyMode => 1,
    timeStart => 1,
    timeStop => 1,
    status => 0,
    parentId => 0,
    fpaId => 0,
    expId => 0,
    diffId => 0,
    xyidxSize => 0,
    pa_deg => 0,
    FOV_deg => 0,
    se_deg => 0,
    eb_deg => 0,
    gb_deg => 0,
};


# SELECT part of query for typical SELECT * operations.  MySQL does
# not allow DEC as a column name, so it's DECL there.
our $selectall_str = <<"SQL";
select
    field_id,
    epoch_mjd, ra_deg, dec_deg, survey_mode, time_stop, time_start, status, 
    filter_id, limiting_mag, ra_sigma, dec_sigma, obscode,
    de1, de2, de3, de4, de5, de6, de7, de8, de9, de10,
    ocnum, nn, parent_id, fpa_id, exp_id, diff_id, xyidx_size, pa_deg, fov_deg, se_deg, eb_deg, gb_deg
from `fields`
SQL


sub new {
    # Create a new unattached field object, for later insertion into MOPS DC.
    # Indirect syntax is available: my $field = PS::MOPS::DC::Field->new(),
    # or modcf_create()
    my $pkg = shift;
    my $inst = shift;
    my %self = validate(@_, $by_value_validate_args);
    $self{_inst} = $inst;
    $self{fieldId} = undef;   # undef until inserted

    # Validate that survey and filter fields exist in DB.
    # need stuff here.

    # Populate OC number from epoch if not present.
    my $cfg = $inst->getConfig();

    my $gmt_offset_hours = $cfg->{site}->{gmt_offset_hours};
    croak "can't get gmt_offset_hours" unless defined($gmt_offset_hours);

    if (!$self{FOV_deg}) {
        my $field_size_deg2 = $cfg->{site}->{field_size_deg2};
        croak "nonexistent or bogus field_size_deg2" unless $field_size_deg2;
        $self{FOV_deg} = 2 * sqrt($field_size_deg2 / $PI);
    }

    $self{pa_deg} ||= 0;        # provide default position angle

    if (!$self{xyidxSize}) {
        $self{xyidxSize} = $cfg->{site}->{field_xyidx_size};
        $self{xyidxSize} ||= 1;     # default; no spatial indexing
    }

    if (!$self{se_deg}) {
        $self{se_deg} = mopslib_computeSolarElongation(
            $self{epoch}, $self{ra}, $self{dec},
            $cfg->{site}->{site_longitude_deg} || 0,
            $cfg->{site}->{site_latitude_deg} || 0,
        );
    }

    if (!$self{eb_deg}) {
        $self{eb_deg} = mopslib_computeEclipticLatitude(
            $self{epoch}, $self{ra}, $self{dec},
        );
    }

    if (!$self{gb_deg}) {
        $self{gb_deg} = mopslib_computeGalacticLatitude(
            $self{ra}, $self{dec},
        );
    }

    $self{ocnum} ||=  mopslib_mjd2ocnum($self{epoch});
    $self{nn} ||=  mopslib_mjd2nn($self{epoch}, $gmt_offset_hours);
    $self{status} ||= $FIELD_STATUS_READY;
    $self{surveyMode} ||= $DUMMY_SURVEY_MODE;
    return bless \%self;
}


sub _new_from_row {
    # Return a new Field object from DBI row handle.
    my ($inst, $row) = @_;
    my $field;
    my @de_array;
    
    return undef if !$row;  # sanity check
    @de_array = map { $row->{$_} } qw(de1 de2 de3 de4 de5 de6 de7 de8 de9 de10);
    $field = PS::MOPS::DC::Field->new(
        $inst,
        epoch => $row->{epoch_mjd},
        ra => $row->{ra_deg},
        dec => $row->{dec_deg},
        raSigma => $row->{ra_sigma},
        decSigma => $row->{dec_sigma},
        surveyMode => $row->{survey_mode},
        timeStop => $row->{time_stop},
        timeStart => $row->{time_start},
        filter => $row->{filter_id},
        limitingMag => $row->{limiting_mag},
        obscode => $row->{obscode},
        de => \@de_array,
        status => $row->{status},
        parentId => $row->{parent_id},
        fpaId => $row->{fpa_id},
        expId => $row->{exp_id},
        diffId => $row->{diff_id},
        xyidxSize => $row->{xyidx_size},
        pa_deg => $row->{pa_deg},
        FOV_deg => $row->{fov_deg},
        se_deg => $row->{se_deg},
        eb_deg => $row->{eb_deg},
        gb_deg => $row->{gb_deg},
        ocnum => $row->{ocnum},
        nn => $row->{nn},
    );
    $field->{fieldId} = $row->{field_id};

    return $field;
}


sub modcf_retrieveSurveyModes {
    # Retrieve a list of survey modes for the specified night number.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { 
        nn => 1,
    });

    my $nn = $args{nn};
    my $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
select survey_mode, count(*) ct
from fields f
where nn=?
group by survey_mode
SQL

    $sth->execute($nn) or croak $dbh->errstr;
    return @{$sth->fetchall_arrayref()};
}


sub modcf_retrieve {
    # return field objects.  The use of "date" or "date_mjd" is deprecated since
    # the lazily rely on the coincidence that all observations from Hawaii
    # occur on the same MJD; thus it is safe to gang fields together by
    # int(mjd).  But since night number is really a local concept, the
    # correct method is to use the GMT offset to convert to local "night
    # numbers".
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { 
        all => 0,                       # retrieve all
        date => 0,                      # on specified MJD (deprecated)
        date_mjd => 0,                  # on specified MJD (deprecated)

        nn => 0,                        # night number
        gmt_offset_hours => 0,          # GMT offset for obtaining night numbers (deprecated)

        begin_mjd => 0,                 # within MJD range (begin/end)
        end_mjd => 0,                   # 

        ocnum => 0,                     # in specified OC num
        fieldId => 0,                   # by fieldId
        fpaId => 0,                     # by fpaId
        diffId => 0,                    # by diff_id
        any => 0,                       # select any old field, not just NEW
        status => 0,                    # by status
        parent => 0,                    # selector for parent fields only

        oldestUnfinished => 0,          # oldest unfinished
        unfinishedNN => 0,				# unfinished field from night number.
        lastProcessed => 0,             # last completely processed field
    });
    my $row;
   
    if ($args{all}) {
        my $sth = $dbh->prepare(<<"SQL") or croak "can't prepare SQL";
$selectall_str
order by epoch_mjd
SQL
        $sth->execute or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    elsif ($args{date} || $args{date_mjd}) {
        my $date = ($args{date} || $args{date_mjd});      # allow both specifications; get one of them
        my $parent_str = $args{parent} ? "and parent_id is null" : '';
        my $status_str = $args{status} ? "and status='$args{status}'" : '';
        my $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
$selectall_str
where epoch_mjd >= ? and epoch_mjd < ?
$parent_str
$status_str
order by epoch_mjd
SQL

        $sth->execute($date, $date + 1) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    elsif ($args{nn}) {
        my $nn = $args{nn};
        my $parent_str = $args{parent} ? "and parent_id is null" : '';
        my $status_str = $args{status} ? "and status='$args{status}'" : '';
        my $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
$selectall_str
where nn=?
$parent_str
$status_str
order by epoch_mjd
SQL

        $sth->execute($nn) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    elsif ($args{begin_mjd} || $args{end_mjd}) {
        my $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
$selectall_str
where epoch_mjd >= ? and epoch_mjd < ?
order by epoch_mjd
SQL

        $sth->execute($args{begin_mjd}, $args{end_mjd}) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    elsif ($args{ocnum}) {
        my $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
$selectall_str
where ocnum=?
order by epoch_mjd
SQL

        $sth->execute($args{ocnum}) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    elsif ($args{status}) {
        my $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
$selectall_str
where status=?
order by epoch_mjd
SQL

        $sth->execute($args{status}) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    elsif ($args{fieldId}) {
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{fieldId});
$selectall_str
where field_id=?
SQL

        return _new_from_row($inst, $row);
    }
    elsif ($args{fpaId}) {
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{fpaId});
$selectall_str
where fpa_id=?
SQL

        return _new_from_row($inst, $row);
    }
    elsif ($args{diffId}) {
        my $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
$selectall_str
where diff_id=?
SQL
        $sth->execute($args{diffId}) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref);
        });
    }    
    elsif ($args{oldestUnfinished}) {
        # Return the stalest (oldest uninished) field.
        my $sql = <<"SQL";
$selectall_str
where status <> '$FIELD_STATUS_LINKDONE' and status <> '$FIELD_STATUS_INGESTED' and status <> '$FIELD_STATUS_SKIP'
order by nn
limit 1
SQL
        my $row = $dbh->selectrow_hashref($sql);
        return _new_from_row($inst, $row);
    }
    elsif ($args{unfinishedNN}) {
        # Return an unfinished field from specified night number.
        # Return a field entry that is least processed. i.e. a field with a new
        # status before returning a field with a tracklet status. The status 
        # array contains the list of statuses in the order that they should be
        # returned.
        my @status = ($FIELD_STATUS_READY, 
        			  $FIELD_STATUS_SYNTH,
        			  $FIELD_STATUS_TRACKLET,
        			  $FIELD_STATUS_POSTTRACKLET,
        			  $FIELD_STATUS_ATTRIBUTIONS,
        			  $FIELD_STATUS_LINK1);
        							
        my $sql = <<"SQL";
$selectall_str
where status = ?
and nn = ?
limit 1
SQL
		my $st;
		foreach $st (@status) {
        	my $row = $dbh->selectrow_hashref($sql, undef, ($st, $args{unfinishedNN}));
        	if (defined($row)){
        		return _new_from_row($inst, $row);
        	}
    	}
    	# Return undef if there are no unfinished fields for the night.
    	return undef; 
    }
    elsif ($args{lastProcessed}) {
        # Return the last completely processed field.
        my $sql = <<"SQL";
$selectall_str
where status = '$FIELD_STATUS_LINKDONE'
order by epoch_mjd desc
limit 1
SQL
        my $row = $dbh->selectrow_hashref($sql);
        return _new_from_row($inst, $row);
    }
    else {
        croak "no field selector specified";
    }
}


sub modcf_delete {
    # return count of detections associated with this obs (necessary? just use getDetectionIds)
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { fieldId => 1 });
    return $dbh->do(<<"SQL", undef, $args{fieldId});
delete
from `fields`
where field_id=?
SQL
}


sub _insert {
    # Generic insert handler; accepts hashref from \%args or Field object.
    my ($dbh, $stuff) = @_;
    my @args = @{$stuff}{qw(
        epoch ra dec surveyMode timeStart timeStop status
        filter limitingMag raSigma decSigma obscode parentId fpaId expId diffId xyidxSize pa_deg FOV_deg
        se_deg eb_deg gb_deg
    )};
    my $rv;

    # Tack on DEs.
    push @args, map { $stuff->{de}->[$_] } (0..9);

    # Handle OC number.
#    my $ocnum = mopslib_mjd2ocnum($stuff->{epoch});
    $rv = $dbh->do(<<"SQL", undef, @args, $stuff->{ocnum}, $stuff->{nn});
insert into `fields` (
epoch_mjd, ra_deg, dec_deg, survey_mode, time_start, time_stop, status, filter_id,
limiting_mag, ra_sigma, dec_sigma, obscode, parent_id, fpa_id, exp_id, diff_id, xyidx_size, pa_deg, fov_deg, se_deg, eb_deg, gb_deg,
de1, de2, de3, de4, de5, de6, de7, de8, de9, de10,
ocnum, nn
)
values (
?, ?, ?, ?, ?, ?, ?, ?,
?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,
?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 
?, ?
)
SQL
    if ($rv) {
        $rv = $dbh->{'mysql_insertid'} if $rv;
        $stuff->{fieldId} = $rv;
    }
    else {
        croak "Couldn't insert field: " . $dbh->errstr;
    }
    return $rv;
}


##sub modcf_insertByValue {
##    # insert previously created field into DC.
##    my %args = validate(@_, $by_value_validate_args);
##    $args{status} ||= $FIELD_STATUS_NEW;
##    my $dbh = modc_getdbh();
##    return _insert($dbh, \%args);
##}


# Methods
sub insert {
    my $self = shift;
    return _insert($self->{_inst}->dbh, $self); # call generic handler
}


sub delete {
    my $self = shift;
    return modcf_delete($self->{_inst}, fieldId => $self->fieldId);
}


sub status {
    my ($self, $status, $run_id) = @_;
    
    # Return status if a status was not passed in as a parameter.
    if (!$status) {
        return $self->{status};    
    }
  
    # Update field status to that given.
    my $dbh = $self->{_inst}->dbh;  
    if ($status) {
        $dbh->do(<<"SQL", undef, $status, $self->fieldId) or croak $dbh->errstr;
update `fields` set status=? where field_id=?
SQL
        $self->{status} = $status;
    }
    
    # Update runs table if a run_id is given.
    if ($run_id) {
        update_runs_table($self->{_inst}, $run_id, $self->fieldId, $status);
    }
}


sub parentId {
    my ($self, $parentId) = @_;
    if (defined($parentId)) {
        my $dbh = $self->{_inst}->dbh;
        $dbh->do(<<"SQL", undef, $parentId, $self->fieldId) or croak $dbh->errstr;
update `fields` set parent_id=? where field_id=?
SQL
        $self->{parentId} = $parentId;
    }
    else {
        return $self->{parentId};
    }
}


sub addDetections {
    # Add multiple detections to a field.  Typically used when bulk-adding
    # false detections.
    my ($self, @dets) = @_;
    require PS::MOPS::DC::Detection;
    return PS::MOPS::DC::Detection::_insert_multiple($self->{_inst}, $self, \@dets);   # already provided in Detections.pm
}


1;
__END__

=head1 NAME

PS::MOPS::DC::Field - Module for manipulating MOPS DC field
(metadata) objects

=head1 SYNOPSIS

  use PS::MOPS::DC::Field;
  my $field = PS::MOPS::DC::Field->new(
    params here
  );

=head1 DESCRIPTION

Manipulate MOPS DC field (metadata) objects.

=item de

10-element array containing IPP detection efficiencies parameterization.

=item dec

Declination of the field center, in degrees.

=item decSigma

Error in the declination, in degrees.

=item epoch

Epoch (date) of the field, in MJD.

=item ocnum

MOPS Observing Cycle number for the Epoch.

=item filter

Filter used for data acquisition.

=item limitingMag

Limiting magnitude of the measurement.

=item obscode

MPC Observatory code.

=item ra

Right Ascension of the field center, in degrees.

=item raSigma

Error in the right ascension.

=item surveyMode

Survey mode used by the measurement.  References a row in the SURVEYS table.

=item timeStart

Start time of the field acquisition, in MJD.

=item timeStop

Stop time of the field acquisition, in MJD.

=item status

MOPS-defined column indicating ingest processing status, as follows:

  N - new, unprocessed
  P - in process
  T - need tracklets calculated
  F - finished ingest processing

=item parent_id

"Master" (last in time) field_id of a tuple-wise grouping.

=item fpaId

IPP FPA ID from field ingest.  This value will be used to associate with originating
IPP detection catalogs and low-confidence archives.

=item expID

IPP exposure ID from field ingest.  This value will be used to associate with originating
IPP detection catalogs and low-confidence archives.

=item diffId

IPP image diff ID from field ingest.  This value will be used to associate with originating
IPP detection catalogs and low-confidence archives.

=item xyidxSize

The number of bins, or granularity, in each of the X and Y directions
for spatial indexing.  This should be an odd number; if it's 1, then no
indexing is performed (all detections have the same index).  The total
number of bins is xyidxSize * xyidxSize.  Detections belonging to the
field are assigned a bin number from the "top-right" of the field,
increasing in the RA direction then in Declination.

=head1 SEE ALSO

PS::MOPS::DC::Connection
PS::MOPS::DC::Detection

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2004 Institute for Astronomy, University of Hawaii

=cut
