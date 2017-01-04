package PS::MOPS::DC::TemplateField;

use 5.008;
use strict;
use warnings;
use Carp;

use base qw(Class::Accessor);
our $VERSION = '0.01';


# Field member descriptions:  see POD below.
__PACKAGE__->mk_accessors(qw(
    ra
    raSigma
    dec
    decSigma
    epoch
    filter
    limitingMag
    obscode
    surveyMode
    timeStart
    timeStop
    de
    ocnum
    nn
    surveyId
));

use Params::Validate qw(:all);
use PS::MOPS::Lib qw(:all);
use PS::MOPS::DC::Iterator;


# Used when creating field or inserting by value.
our $by_value_validate_args = {
    epoch => 1,
    ra => 1,
    dec => 1,
    raSigma => 1,
    decSigma => 1,
    filter => 1,
    limitingMag => 1,
    obscode => 1,
    timeStart => 1,
    timeStop => 1,
    de => {type => ARRAYREF},
    surveyMode => 1,
    surveyId => 1,

    ocnum => 0,
    nn => 0,
};


# SELECT part of query for typical SELECT * operations.  MySQL does
# not allow DEC as a column name, so it's DECL there.
our $selectall_str = <<"SQL";
select
    epoch_mjd, ra_deg, dec_deg, ra_sigma, dec_sigma,
    filter_id, limiting_mag, obscode, 
    time_start, time_stop, 
    de1, de2, de3, de4, de5, de6, de7, de8, de9, de10,
    survey_mode, survey_id, ocnum, nn
from template_fields
where survey_id=?
SQL


sub new {
    # Create a new unattached field object, for later insertion into MOPS DC.
    # Indirect syntax is available: my $field = PS::MOPS::DC::TemplateField->new(),
    # or modcf_create()
    my $pkg = shift;
    my $inst = shift;
    my $gmt_offset_hours = $inst->gmt_offset_hours;

    my %self = validate(@_, $by_value_validate_args);
    $self{_inst} = $inst;

    # Validate that survey and filter fields exist in DB.
    # need stuff here.

    # Populate OC number from epoch if not present.
    $self{ocnum} ||=  mopslib_mjd2ocnum($self{epoch});
    $self{nn} ||=  mopslib_mjd2nn($self{epoch}, $gmt_offset_hours);
    return bless \%self;
}


sub _new_from_row {
    # Return a new Field object from DBI row handle.
    my ($inst, $row) = @_;
    my $field;
    my @de_array;
    
    return undef if !$row;  # sanity check
    @de_array = @{$row}{qw(de1 de2 de3 de4 de5 de6 de7 de8 de9 de10)};
    $field = PS::MOPS::DC::TemplateField->new(
        $inst,
        epoch => $row->{epoch_mjd},
        ra => $row->{ra_deg},
        dec => $row->{dec_deg},
        raSigma => $row->{ra_sigma},
        decSigma => $row->{dec_sigma},
        filter => $row->{filter_id},
        limitingMag => $row->{limiting_mag},
        obscode => $row->{obscode},
        timeStart => $row->{time_start},
        timeStop => $row->{time_stop},
        de => \@de_array,
        surveyMode => $row->{survey_mode},
        surveyId => $row->{survey_id},
        ocnum => $row->{ocnum},
        nn => $row->{nn},
    );

    return $field;
}


sub retrieve {
    # return field objects.  The use of "date" or "date_mjd" is deprecated since
    # the lazily rely on the coincidence that all observations from Hawaii
    # occur on the same MJD; when this is true it is safe to gang fields together by
    # int(mjd).  But since night number is really a local concept, the
    # correct method is to use the GMT offset to convert to local "night
    # numbers".
    my $pkg = shift;
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { 
        survey_id => 1,                 # which survey
        survey_mode => 0,               # survey mode selector

        all => 0,                       # retrieve all fields
        nn => 0,                        # single NN to insert
        start_nn => 0,                  # start of range
        end_nn => 0,                    # ending night number, inclusive
        start_mjd => 0,                 # specify MJD
        ending_mjd => 0,                # specify ending MJD, exact

        ocnum => 0,                     # single OC num to insert
        start_ocnum => 0,               # first day of OC num
        end_ocnum => 0,                 # ending OC num, inclusive

        extra_sql => 0,                 # hackish extra SQL for retrieve
    });

    my $gmt_offset_hours = $inst->gmt_offset_hours;
    my $row;
    my $base_sql = $selectall_str;
    my $sql;
    my $extra_sql = '';

    if ($args{extra_sql}) {
        $extra_sql = "and $args{extra_sql}";
    }

    if ($args{survey_mode}) {
        my $lc_survey_mode = lc($args{survey_mode});
        $base_sql .= <<"SQL";
and lower(survey_mode) like '%$lc_survey_mode%' $extra_sql
SQL
    }
   
    if ($args{all}) {
        my $sth = $dbh->prepare(<<"SQL") or croak "can't prepare SQL";
$base_sql $extra_sql
order by epoch_mjd
SQL
        $sth->execute($args{survey_id}) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    else {  
        # Some date specifier was used.  Convert to start and ending MJDs.
        my ($start_mjd, $end_mjd);

        if ($args{ocnum}) {
            $start_mjd = mopslib_ocnum2mjd($args{ocnum});
            $end_mjd = mopslib_ocnum2mjd($args{ocnum} + 1); # might be overridden later
        }
        elsif ($args{start_ocnum}) {    
            # only allow --start_ocnum=FOO --end_ocnum=BAR
            $start_mjd = mopslib_ocnum2mjd($args{start_ocnum});
            croak "end_ocnum is required" unless defined($args{end_ocnum});    # require end_ocnum
            $end_mjd = mopslib_ocnum2mjd($args{end_ocnum} + 1);
        }

        if ($args{nn}) {
            $start_mjd = mopslib_nn2mjd($args{nn}, $gmt_offset_hours);
            $end_mjd = $start_mjd + 1;
        }
        elsif ($args{start_nn}) {
            $start_mjd = mopslib_nn2mjd($args{start_nn}, $gmt_offset_hours);
            if ($args{end_nn}) {
                $end_mjd = mopslib_nn2mjd($args{end_nn} + 1, $gmt_offset_hours);
            }
            croak "can't determine end_mjd" unless defined($end_mjd);    # require end_nn
        }
        elsif ($args{start_mjd}) {
            # Require end_mjd
            $start_mjd = $args{start_mjd};
            if ($args{end_mjd}) {
                $end_mjd = $args{end_mjd};
            }
            croak "can't determine end_mjd" unless defined($end_mjd);    # require end_nn
        }

        croak "Can't determine starting and ending epochs for retrieval"
            unless $start_mjd and $end_mjd;

        my $sql = <<"SQL";
$base_sql $extra_sql
and epoch_mjd >= ? and epoch_mjd < ?
order by epoch_mjd
SQL

        my $sth = $dbh->prepare($sql) or croak $dbh->errstr;
        $sth->execute($args{survey_id}, $start_mjd, $end_mjd) or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref);
        });
    }
}


sub _insert {
    # Generic insert handler; accepts hashref from \%args or TemplateField object.
    my ($dbh, $stuff) = @_;
    my @args = @{$stuff}{qw(
        epoch ra dec raSigma decSigma
        filter limitingMag obscode 
        timeStart timeStop 
    )};

    # Tack on DEs.
    push @args, map { $stuff->{de}->[$_] } (0..9);

    # Rest of stuff.
    push @args, @{$stuff}{qw(
        surveyMode surveyId ocnum nn
    )};

    my $rv;
    $rv = $dbh->do(<<"SQL", undef, @args);
insert into template_fields (
epoch_mjd, ra_deg, dec_deg, ra_sigma, dec_sigma, 
filter_id, limiting_mag, obscode,
time_start, time_stop, 
de1, de2, de3, de4, de5, de6, de7, de8, de9, de10,
survey_mode, survey_id, ocnum, nn 
)
values (
?, ?, ?, ?, ?, 
?, ?, ?, 
?, ?,
?, ?, ?, ?, ?, ?, ?, ?, ?, ?,
?, ?, ?, ?
)
SQL
    if (!$rv) {
        croak "Couldn't insert field: " . $dbh->errstr;
    }
    return $rv;
}


# Methods
sub insert {
    my $self = shift;
    return _insert($self->{_inst}->dbh, $self); # call generic handler
}

1;

__END__

=head1 NAME

PS::MOPS::DC::TemplateField - Module for manipulating field template objects

=head1 SYNOPSIS

  use PS::MOPS::DC::TemplateField;
  my $template_field = PS::MOPS::DC::TemplateField->new(
    params here
  );
  $template_field->insert()

  $template_field_i = PS::MOPS::DC::TemplateField->retrieve(
    all => 1,
  )

  $template_field_i = PS::MOPS::DC::TemplateField->retrieve(
    nn => 54017,
  )

  $template_field_i = PS::MOPS::DC::TemplateField->retrieve(
    start_nn => 54017,
    end_nn => 54045
  )

  $template_field_i = PS::MOPS::DC::TemplateField->retrieve(
    ocnum => 95
    survey_mode => 'SS'
  )

  $template_field_i = PS::MOPS::DC::TemplateField->retrieve(
    start_ocnum => 86
    end_ocnum => 95
  )

=head1 DESCRIPTION

TemplateFields are used to build a library of MOPS DC Field-like objects
external to a MOPS simulation that can be used to generate ingest files
or can be directly inserted into a simulation.

Various selectors can be used to retrieve a range of template fields
from the templte field database:

=item all

When all => 1 is specified, all template fields are retrieved from
the database, subject to the survey_mode selector.

=item nn

nn specifies a single night number to load.

=item start_nn, end_nn

These specify a range (inclusive) of night numbers to load.

=item ocnum

ocnum specifies a single OC to load.

=item start_ocnum, end_ocnum

Specifieds a range (inclusive) of OC numbers to load.

=item survey_mode

Specifies a string that will be matched against the survey_modes in the
template field table.

=head1 FIELD ATTRIBUTES

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

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2008 Institute for Astronomy, University of Hawaii

=cut
