#! /usr/bin/env perl
package PS::MOPS::FITS::IPP;

use strict;
use warnings;
use Carp;
use Astro::FITS::CFITSIO;


our $VERSION = '0.01';

#
# TODO: These initial values should be combined into an initialization
# file.
#

###########################################################################
# Key definitions:
#     FITS data type
#     field width (only relevant for strings)
#     FITS comment string
#
# TODO: Add appropriate comment fields 
# TODO: String I/O should use field width
###########################################################################

my @keys = qw(
    EXP_NAME EXP_ID CHIP_ID CAM_ID FAKE_ID WARP_ID DIFF_ID DIFF_POS
    MJD-OBS RA DEC TEL_ALT TEL_AZ EXPTIME ROTANGLE FILTER AIRMASS OBSCODE SEEING MAGZP MAGZPERR ASTRORMS
);

# Main IPP FITS table definition, for full FPAs.
my %keydefs = (
    'EXP_NAME' => [ Astro::FITS::CFITSIO::TSTRING(), 20, 'Camera exposure ID' ],
    'EXP_ID' => [ Astro::FITS::CFITSIO::TLONG(), 0, 'IPP exposure ID' ],
    'CHIP_ID' => [ Astro::FITS::CFITSIO::TLONG(), 0, 'IPP chip stage ID' ],
    'CAM_ID' => [ Astro::FITS::CFITSIO::TLONG(), 0, 'IPP camera stage ID' ],
    'FAKE_ID' => [ Astro::FITS::CFITSIO::TLONG(), 0, 'IPP fake stage ID' ],
    'WARP_ID' => [ Astro::FITS::CFITSIO::TLONG(), 0, 'IPP warp stage ID' ],
    'DIFF_ID' => [ Astro::FITS::CFITSIO::TLONG(), 0, 'IPP diff stage ID' ],
#    'DIFF_POS' => [ Astro::FITS::CFITSIO::TBOOL(), 0, 'Diff sense of subtraction, T fwd F bkwd' ],
    'DIFF_POS' => [ Astro::FITS::CFITSIO::TSTRING(), 1, 'Diff sense of subtraction, T fwd F bkwd' ],

    'MJD-OBS' => [ Astro::FITS::CFITSIO::TDOUBLE(), 0, 'Midpoint time, TAI' ],
    'RA' => [ Astro::FITS::CFITSIO::TSTRING(), 12, 'Right ascension, sexagesimal hours' ],
    'DEC' => [ Astro::FITS::CFITSIO::TSTRING(), 9, 'Declination, sexagesimal degrees' ],
    'TEL_ALT' => [ Astro::FITS::CFITSIO::TDOUBLE(), 0, '' ],
    'TEL_AZ' => [ Astro::FITS::CFITSIO::TDOUBLE(), 0, '' ],
    'EXPTIME' => [ Astro::FITS::CFITSIO::TFLOAT(), 0, 'Exposure time, seconds' ],
    'ROTANGLE' => [ Astro::FITS::CFITSIO::TDOUBLE(), 0, 'Rotator angle, degrees' ],
    'FILTER' => [ Astro::FITS::CFITSIO::TSTRING(), 3, '' ],
    'AIRMASS' => [ Astro::FITS::CFITSIO::TFLOAT(), 0, '' ],
    'OBSCODE' => [ Astro::FITS::CFITSIO::TSTRING(), 5, ''],
    'SEEING' => [ Astro::FITS::CFITSIO::TFLOAT(), 0, 'Measured seeing at diff stage, arcsec' ],
    'MAGZP' => [ Astro::FITS::CFITSIO::TFLOAT(), 0, 'Magnitude zero point' ],
    'MAGZPERR' => [ Astro::FITS::CFITSIO::TFLOAT(), 0, '' ],
    'ASTRORMS' => [ Astro::FITS::CFITSIO::TFLOAT(), 0, 'RMS of astrometric fit, arcsec' ],
);

###########################################################################
# Column definitions: data type, data format
#
# The @cols array defines the order of the fields, so changing it
# will restructure the file.
###########################################################################

my @cols = qw(RA RA_ERR DEC DEC_ERR MAG MAG_ERR EXT_SIGNIFICANCE ANGLE ANGLE_ERR LENGTH LENGTH_ERR FLAGS DIFF_SKYFILE_ID);
my %coldefs = (
    'RA' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1D1' ],
    'RA_ERR' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1D1' ],
    'DEC' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1D1' ],
    'DEC_ERR' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1D1' ],
    'MAG' => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    'MAG_ERR' => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    'EXT_SIGNIFICANCE' => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    'ANGLE' => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    'ANGLE_ERR' => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    'LENGTH' => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    'LENGTH_ERR' => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    'FLAGS' => [ Astro::FITS::CFITSIO::TULONG(), '1J' ],
    'DIFF_SKYFILE_ID' => [ Astro::FITS::CFITSIO::TLONG(), '1K1' ],
);

my @dipole_cols = qw(N_POS F_POS RATIO_BAD RATIO_MASK RATIO_ALL PSF_QUALITY);
my %dipole_coldefs = (
    'N_POS' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1J' ],
    'F_POS' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1E' ],
    'RATIO_BAD' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1E' ],
    'RATIO_MASK' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1E' ],
    'RATIO_ALL' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1E' ],
    'PSF_QUALITY' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1E' ],
);

# The flags column is replaced with this value if the dipole params
# evaluate to dipolish.
my $DIPOLE_FLAGS = 0xFFFF;

###################################################################
# Accessor method for keys.
#
# $self - object
# $key - Key to be set
# $value - value key is to be set to, if undefined, value is not set
#
# TODO: This should do a check for invalid parameters.
###################################################################

sub keyval {
    my ( $self, $key, $value ) = @_;

    $self->{'_keys'}{$key} = $value if defined($value);
    return $self->{'_keys'}{$key};
}


sub _check_status {
    # Private utility routine to just report the status and die
    # if the status is non-zero.
    my ($status, $msg) = @_;
    if ($status) {
        Astro::FITS::CFITSIO::fits_report_error('STDERR', $status);
        confess "FITS status: $status: $msg";
    }
}


###################################################################
# Open a FITS file, and read the header information
#
# $self - object
# $filename - name of file to be opened
###################################################################

sub openfile {
    ##############################################################
    # parameter checking and variable initialization
    ##############################################################

    my ( $self, $base_filename, $remove_dipoles ) = @_;
    if ( not defined $base_filename ) {
        die "must specify $base_filename"
    }

    # the field metadata is stored in the zeroth extention.  so read 
    # this hdu and extract the stuff from it.  then open the first 
    # extention, where we will find detectiions.
    my $status = 0;                         # cfitsio $status variable.
    my $filename;                           # general-purpose filename with fits hdu attached
    my $value;
    my $comment;

    # Open and read the fits file.
#   $filename = $base_filename . "[0]";
    $filename = $base_filename;
    $self->{_fptr} = Astro::FITS::CFITSIO::open_file($filename, 
        Astro::FITS::CFITSIO::READONLY(), $status);
    _check_status($status, "error while reading $filename");

    # Detections, etc.
    $self->{_fptr}->movrel_hdu(1, undef, $status);
    _check_status($status, "error moving to HDU 1 in $filename");
    $self->{_row} = 1;

    # Metadata keywords.
    $self->{_keys} = undef;
    for my $key (@keys) {
        $self->{_fptr}->read_keyword($key, $value, $comment, $status);

        if (!defined($value)) {
            die "Undefined keyword $key while reading $filename";
        }
        $value =~ s/^'//;
        $value =~ s/ *'$//;
        $self->{_keys}{$key} = $value;
    }

    # Read the extension name.
    $self->{_fptr}->read_keyword("EXTNAME", $value, $comment, $status);
    _check_status($status, "error reading keyword 'EXTNAME'");
    $self->{_extname} = $value;

    # Read the version name.
#   $self->{_fptr}->read_keyword("TABLEVER", $value, $comment, $status);
#   $self->{_tablever} = $value;
    $self->{_tablever} = 'UNKNOWN';
#    _check_status($status, "error getting TABLEVER");


    #  Get the number of rows
    $self->{_fptr}->get_num_rows($self->{'_nrows'}, $status);
    _check_status($status, "error getting number of rows");

    # Get the number of columns.
    $self->{_fptr}->get_num_cols($self->{'_ncols'}, $status);
    _check_status($status, "error getting number of columns");

    # Set up our column defs.
    $self->{_col2num} = {};
    my $colnum;
    my $template;
    for my $col (@cols) {
        $status = 0;
        $self->{_fptr}->get_colnum(Astro::FITS::CFITSIO::CASEINSEN(), $col, $colnum, $status);
        if ($status == Astro::FITS::CFITSIO::COL_NOT_FOUND()) {
            if ($col =~ /^(LENGTH|LENGTH_ERR|ANGLE|ANGLE_ERR)$/) {
                # Were removed in 28918.
                $self->{_col2num}->{$col} = 0;      # 0 means not present
            }
        };
        $self->{_col2num}->{$col} = $colnum;     # save FITS col for this column
    }

    # These columns are optional.
    for my $col (@dipole_cols) {
        $status = 0;
        $self->{_fptr}->get_colnum(Astro::FITS::CFITSIO::CASEINSEN(), $col, $colnum, $status);
        if ($status != Astro::FITS::CFITSIO::COL_NOT_FOUND()) {
            $self->{_col2num}->{$col} = $colnum;    # save FITS col for this column
            $self->{_have_dipole} = 1;              # yes, have dipole stats
        }
    }

    # Other flags.
    $self->{_remove_dipoles} = $remove_dipoles;
}

###################################################################
# Read a record from the FITS file.
#
# $self - object
###################################################################

sub readrecord()
{
    my ($self) = @_;

    my $status = 0;
    my @data = ();

    if ($self->{'_row'} > $self->{'_nrows'}) {
        return ();
    }

    # Read each column in turn, and display the corresponding value.  For
    # now, I just read the first row.

    my $ttype;
    my $colname;
    my $colnum;
    my @datum;
    for my $col (@cols) {
        $colnum = $self->{_col2num}->{$col};
        if ($colnum > 0) {
            $ttype = $coldefs{$col}->[0];
            $self->{_fptr}->read_col($ttype, $colnum, $self->{_row}, 1, 1, 0, \@datum, my $anynul, $status);
            _check_status($status, "error reading column: $col");
            push @data, $datum[0];
        }
        else {
            push @data, 0;      # for missing LENGTH/LENGTH_ERR/ANGLE/ANGLE_ERR cols
        }
    }

    # If we have dipole data, read the params && evaluate.  If this 
    # detection is dipolish, set flags to 0xffff.
    my $dratio;
    if ($self->{_remove_dipoles} and $self->{_have_dipole}) {
        my @dipole_data;

        for my $col (@dipole_cols) {
            $colnum = $self->{_col2num}->{$col};
            $ttype = $dipole_coldefs{$col}->[0];
            $self->{_fptr}->read_col($ttype, $colnum, $self->{_row}, 1, 1, 0, \@datum, my $anynul, $status);
            _check_status($status, "error reading column: $col");
            push @dipole_data, $datum[0];
        }

        # Now evaluate dipole data.
        unless (
#            $dipole_data[0] > 5             # NPOS > 5
            $dipole_data[1] > 0.7           # F_POS > 0.7
            and $dipole_data[2] > 0.7       # NRATIO_BAD > 0.7 (nPos / (nPos + nNeg))

            and $dipole_data[2] / $dipole_data[1] > .85 
            and $dipole_data[2] / $dipole_data[1] < 1.15

            and $dipole_data[5] > .75       # PSF_QUALITY
#            and $dipole_data[3] > 0.7       # NRATIO_MASK > 0.7 (nPos / (nPos + nMask))
#            and $dipole_data[4] > 0.5       # NRATIO_ALL > 0.5 (nPos / (nGood + nMask + nBad)
        ) {
            $data[11] = $DIPOLE_FLAGS; # BAD!
        }
    }

    $self->{_row}++;
    return @data;
}


###################################################################
# Write values to a FITS file.  Dies upon error.
#
# $self - object
# $filename - file to be created
# %keys - keys to be stored in the FITS header
###################################################################

sub rejectrecord() {
    # Use our knowledge of the FITS table to decide whether
    # to reject this detetion (dipole filtering, etc).
    
    return 0;       # default for now
}

###################################################################
# Write values to a FITS file.  Dies upon error.
#
# $self - object
# $filename - file to be created
# %keys - keys to be stored in the FITS header
###################################################################

sub createfile()
{
    my ($self, $filename, $force, %keys) = @_;
    my @null_ttype = ();
    my @null_tform = ();
    my $key;

    for $key (@keys) {
        $self->{'_keys'}{$key} = $keys{$key};
    }

    # Parameter checking and variable initializing
    if ( not defined $filename ) {
        die "Must specify $filename"
    }

    # The cfitsio status variable.
    my $status = 0;

    # Open the file
    if ($force) {
        $self->{_fptr} = Astro::FITS::CFITSIO::create_file('!' . $filename,
            $status);
    } else {
        $self->{_fptr} = Astro::FITS::CFITSIO::create_file($filename,
            $status);
    }
    _check_status($status, "cannot create file $filename");

    $self->{_fptr}->create_img(16, 0, undef, $status);
    _check_status($status, "cannot create img $filename");

    ################################################################
    # Create the table (currently empty).
    ################################################################

    $self->{_fptr}->create_tbl(Astro::FITS::CFITSIO::BINARY_TBL(),
        0, 0, \@null_ttype, \@null_tform, 0, $self->{_extname}, $status);
    _check_status($status, "cannot create table");

    $self->{_row} = 1;

    for $key (@keys) {
        my $type = $keydefs{$key}[0];
        my $width = $keydefs{$key}[1];
        my $comment = $keydefs{$key}[2];
        my $value = $self->{'_keys'}{$key};

        $self->{_fptr}->write_key($type, $key, $value, $comment, $status);
        _check_status($status, "cannot write key $key to file $filename");
    }

    $self->{_fptr}->write_key(Astro::FITS::CFITSIO::TSTRING(),
        'TABLEVER', $self->{_tablever}, 'Table Version', $status);
    _check_status($status, "cannot write key 'TABLEVER'");

    ########################################################################
    # Create the columns of the table.  For some reason, I get a segfault
    # when I try to create a multi-column table with create_tbl.
    ########################################################################

    # Create the matching format string.
    $self->{_col2num} = {};
    my @tform = ();
    my $i = 1;
    for $key (@cols) {
        @tform = ( @tform, $coldefs{$key}[1] );
        $self->{_col2num}->{$key} = $i;
        $i++;
    }

    # Now create the columns themselves
    $self->{_fptr}->insert_cols(1, @cols + 0, \@cols, \@tform, $status);
    _check_status($status, "cannot insert columns");

    # Create our column mapping table.
}

###################################################################
# Write a single record of data
#
# $self - object
# @data - array of data to be written (RA_DEG, DEC_DEG, RA_SIG, DEC_SIG,
#     MAG, MAG_SIG, FLGS, STARPSF, ANG, ANG_SIG, LEN, LEN_SIG, PROC_ID)
###################################################################

sub writerecord {
    my ( $self, @data ) = @_;

    my $status = 0;

    #
    # Write the data values for each column.  For now, I just write
    # a 1 value in the first row, for testing.
    #

    my $ttype;
    my $i = 0;
    for my $col (@cols) {
        my @datum = ($data[$i]);
        $ttype = $coldefs{$col}->[0];
        $self->{_fptr}->write_col($ttype, $self->{_col2num}->{$col}, 
            $self->{_row}, 1, 1, \@datum, $status);
        _check_status($status, "error writing column");
        $i++;
    }

    $self->{_row}++;
}

###################################################################
# Close the file
#
# $self - object
###################################################################

sub closefile {
    my ( $self ) = @_;

    my $status = 0;

    $self->{_fptr}->close_file($status);
    _check_status($status, "cannot close file");

    $self->{_fptr} = undef;
    $self->{_row} = undef;
    $self->{_nrows} = undef;
}

###################################################################
# Print all keys and other table information
#
# $self - object
###################################################################

sub printkeys {
    my ( $self ) = @_;

    printf "%8s = %s\n", "EXTNAME", $self->{_extname};
    printf "%8s = %s\n", "TABLEVER", $self->{_tablever};

    for my $key (keys %keydefs) {
        printf("%8s = %s\n", $key, $self->{'_keys'}{$key});
    }
}

###################################################################
# constructor
###################################################################

sub new {
    my ($class) = @_;
    my $self = {
        _extname => 'MOPS_TRANSIENT_DETECTIONS',
        _tablever => 'UNKNOWN',
        _keys => undef,
        _data => undef,
        _fptr => undef,
        _row => undef,
        _nrows => undef,
    };

    bless $self, $class;
    return $self;
}

1;

__END__

=head1 NAME

PS::MOPS::DataStore::table.pm - Manipulate a FITS table file.

=head1 SYNOPSIS

use PS::MOPS::DataStore::table;

=head1 DESCRIPTION

Routines for manipulating FITS tables.  (See IPP-MOPS ICD).

=head1 PUBLIC FUNCTIONS

=head2 keyval

Accessor method for keys.

=head2 openfile

Open a file for reading.

=head2 readrecord

Read a single record from the file.

=head2 createfile

Open a file for writing.

=head2 writerecord

Write a single record to the file.

=head2 closefile

Close the file.

=head2 printkeys

Print the information about the file.

=head2 new

Create a new object.

=cut
