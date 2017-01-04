#! /usr/bin/env perl
package PS::MOPS::FITS::IPP2;

# Version 2 of IPP-MOPS export. Many smf-like additional columns.

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

my @cols = qw(RA RA_ERR DEC DEC_ERR MAG MAG_ERR EXT_SIGNIFICANCE FLAGS DIFF_SKYFILE_ID);
my %coldefs = (
    'RA' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1D1' ],
    'RA_ERR' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1D1' ],
    'DEC' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1D1' ],
    'DEC_ERR' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1D1' ],
    'MAG' => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    'MAG_ERR' => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    'EXT_SIGNIFICANCE' => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    'FLAGS' => [ Astro::FITS::CFITSIO::TULONG(), '1J' ],
    'DIFF_SKYFILE_ID' => [ Astro::FITS::CFITSIO::TLONG(), '1K1' ],
);

my @rawattr_cols = qw(
    PSF_CHI2 PSF_DOF
    CR_SIGNIFICANCE EXT_SIGNIFICANCE
    PSF_MAJOR PSF_MINOR PSF_THETA PSF_QUALITY PSF_NPIX
    MOMENTS_XX MOMENTS_XY MOMENTS_YY
    N_POS F_POS
    RATIO_BAD RATIO_MASK RATIO_ALL
    FLAGS
    IPP_IDET
    PSF_INST_FLUX PSF_INST_FLUX_SIG
    AP_MAG AP_MAG_RAW AP_MAG_RADIUS AP_FLUX AP_FLUX_SIG
    PEAK_FLUX_AS_MAG
    CAL_PSF_MAG CAL_PSF_MAG_SIG
    SKY SKY_SIGMA
    PSF_QF_PERFECT
    MOMENTS_R1 MOMENTS_RH
    KRON_FLUX KRON_FLUX_ERR KRON_FLUX_INNER KRON_FLUX_OUTER
    DIFF_R_P DIFF_SN_P DIFF_R_M DIFF_SN_M
    FLAGS2 N_FRAMES
);
my %rawattr_coldefs = (
    PSF_CHI2 => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],              # 0
    PSF_DOF => [ Astro::FITS::CFITSIO::TULONG(), '1J' ],
    CR_SIGNIFICANCE => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    EXT_SIGNIFICANCE => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    PSF_MAJOR => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    PSF_MINOR => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    PSF_THETA => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    PSF_QUALITY => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    PSF_NPIX => [ Astro::FITS::CFITSIO::TULONG(), '1J' ],
    MOMENTS_XX => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    MOMENTS_XY => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],            # 10
    MOMENTS_YY => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    N_POS => [ Astro::FITS::CFITSIO::TULONG(), '1J' ],
    F_POS => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    RATIO_BAD => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    RATIO_MASK => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    RATIO_ALL => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    FLAGS => [ Astro::FITS::CFITSIO::TULONG(), '1J' ],
    IPP_IDET => [ Astro::FITS::CFITSIO::TULONG(), '1J' ],
    PSF_INST_FLUX => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    PSF_INST_FLUX_SIG => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],     # 20
    AP_MAG => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    AP_MAG_RAW => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    AP_MAG_RADIUS => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    AP_FLUX => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    AP_FLUX_SIG => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    PEAK_FLUX_AS_MAG => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    CAL_PSF_MAG => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    CAL_PSF_MAG_SIG => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    SKY => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    SKY_SIGMA => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],             # 30
    PSF_QF_PERFECT => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    MOMENTS_R1 => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    MOMENTS_RH => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    KRON_FLUX => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    KRON_FLUX_ERR => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    KRON_FLUX_INNER => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    KRON_FLUX_OUTER => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    DIFF_R_P => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    DIFF_SN_P => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    DIFF_R_M => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],              # 40
    DIFF_SN_M => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
    FLAGS2 => [ Astro::FITS::CFITSIO::TSHORT(), '1I' ],
    N_FRAMES => [ Astro::FITS::CFITSIO::TSHORT(), '1I' ],
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

    my ( $self, $base_filename) = @_;
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
    my $col;

    foreach $col (@cols) {
        $status = 0;
        $self->{_fptr}->get_colnum(Astro::FITS::CFITSIO::CASEINSEN(), $col, $colnum, $status);
        if ($status == Astro::FITS::CFITSIO::COL_NOT_FOUND()) {
            die "can't find required column $col in $filename";
        }
        $self->{_col2num}->{$col} = $colnum;     # save FITS col for this column
    }

    # These columns are optional.
    if ($self->{_ncols} > 25) {
        foreach $col (@rawattr_cols) {
            $status = 0;
            $self->{_fptr}->get_colnum(Astro::FITS::CFITSIO::CASEINSEN(), $col, $colnum, $status);
            if ($status != Astro::FITS::CFITSIO::COL_NOT_FOUND()) {
                $self->{_col2num}->{$col} = $colnum;    # save FITS col for this column
                $self->{_have_rawattr_v2} = 1;              # yes, have dipole stats
            }
        }
        $self->{_have_rawattr_v2} = 1;
    }
    else {
        $self->{_have_rawattr_v2} = 0;
    }
}

###################################################################
# Read a short from the FITS file.  This is just essential
# stuff for spatial filtering.
#
# $self - object
###################################################################

sub readrecordshort()
{
    my ($self) = @_;
    if ($self->{'_row'} > $self->{'_nrows'}) {
        return ();
    }
    my $status = 0;
    my @data = ($self->{_row});     # first element is row num

    # Read each column in turn, and display the corresponding value.  For
    # now, I just read the first row.

    my $ttype;
    my $colname;
    my $colnum;
    my @datum;
    my $col;

    for $col (@cols) {
        $colnum = $self->{_col2num}->{$col};
        if ($colnum > 0) {
            $ttype = $coldefs{$col}->[0];
            $self->{_fptr}->read_col($ttype, $colnum, $self->{_row}, 1, 1, 0, \@datum, my $anynul, $status);
            _check_status($status, "error reading column: $col");

            # Have to fix bogus OFF_CHIP flag, which is always set (as of 04 DEC 2011).
            if ($col eq 'FLAGS') {
                $datum[0] &= 0x7fffffff;
            }
            push @data, $datum[0];
        }
        else {
            die "can't find column $col";
        }
    }

    $self->{_row}++;
    return @data;
}

###################################################################
# Read a record from the FITS file.
#
# $self - object
###################################################################

sub readrecord()
{
    my ($self) = @_;
    if ($self->{'_row'} > $self->{'_nrows'}) {
        return ();
    }
    my $status = 0;
    my @data = ($self->{_row});     # first element is row num

    # Read each column in turn, and display the corresponding value.  For
    # now, I just read the first row.

    my $ttype;
    my $colname;
    my $colnum;
    my @datum;
    my $col;

    for $col (@cols) {
        $colnum = $self->{_col2num}->{$col};
        if ($colnum > 0) {
            $ttype = $coldefs{$col}->[0];
            $self->{_fptr}->read_col($ttype, $colnum, $self->{_row}, 1, 1, 0, \@datum, my $anynul, $status);
            _check_status($status, "error reading column: $col");

            # Have to fix bogus OFF_CHIP flag, which is always set (as of 04 DEC 2011).
            if ($col eq 'FLAGS') {
                $datum[0] &= 0x7fffffff;
            }
            push @data, $datum[0];
        }
        else {
            die "can't find column $col";
        }
    }


    # Read extended raw attributes.
    if ($self->{_have_rawattr_v2}) {
        my @rawattr_data = ();
        for $col (@rawattr_cols) {
            $colnum = $self->{_col2num}->{$col};
            if ($colnum) {
                $ttype = $rawattr_coldefs{$col}->[0];
                $self->{_fptr}->read_col($ttype, $colnum, $self->{_row}, 1, 1, 0, \@datum, my $anynul, $status);
                _check_status($status, "error reading column: $col");

                # Have to fix bogus OFF_CHIP flag, which is always set (as of 04 DEC 2011).
                if ($col eq 'FLAGS') {
                    $datum[0] &= 0x7fffffff;
                }
                push @rawattr_data, $datum[0];
            }
            else {
                die "bogus column $col";
            }
        }
        push @data, \@rawattr_data;     # ref to rawattr data is last item
    }

    $self->{_row}++;
    return @data;
}


###################################################################
# Return a flag if the detection should be rejected for various 
# reasons:
#
# $self - object
# $row - arrayref to row retrieved from readrecord()
#
# 0   ROWNUM
# 1   'RA' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1D1' ],
# 2   'RA_ERR' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1D1' ],
# 3   'DEC' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1D1' ],
# 4   'DEC_ERR' => [ Astro::FITS::CFITSIO::TDOUBLE(), '1D1' ],
# 5   'MAG' => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
# 6   'MAG_ERR' => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
# 7   'EXT_SIGNIFICANCE' => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
# 8   'DIFF_SKYFILE_ID' => [ Astro::FITS::CFITSIO::TLONG(), '1K1' ],
# 9   'RAWATTR_V2' => [
#   0   PSF_CHI2 => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   1   PSF_DOF => [ Astro::FITS::CFITSIO::TULONG(), '1J' ],
#   2   CR_SIGNIFICANCE => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   3   EXT_SIGNIFICANCE => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   4   PSF_MAJOR => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   5   PSF_MINOR => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   6   PSF_THETA => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   7   PSF_QUALITY => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   8   PSF_NPIX => [ Astro::FITS::CFITSIO::TULONG(), '1J' ],
#   9   MOMENTS_XX => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   10  MOMENTS_XY => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   11  MOMENTS_YY => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   12  N_POS => [ Astro::FITS::CFITSIO::TULONG(), '1J' ],
#   13  F_POS => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   14  RATIO_BAD => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   15  RATIO_MASK => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   16  RATIO_ALL => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   17  FLAGS => [ Astro::FITS::CFITSIO::TULONG(), '1J' ],
#   18  IPP_IDET => [ Astro::FITS::CFITSIO::TULONG(), '1J' ],
#   19  PSF_INST_FLUX => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   20  PSF_INST_FLUX_SIG => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   21  AP_MAG => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   22  AP_MAG_RAW => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   23  AP_MAG_RADIUS => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   24  AP_FLUX => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   25  AP_FLUX_SIG => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   26  PEAK_FLUX_AS_MAG => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   27  CAL_PSF_MAG => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   28  CAL_PSF_MAG_SIG => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   29  SKY => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   30  SKY_SIGMA => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   31  PSF_QF_PERFECT => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   32  MOMENTS_R1 => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   33  MOMENTS_RH => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   34  KRON_FLUX => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   35  KRON_FLUX_ERR => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   36  KRON_FLUX_INNER => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   37  KRON_FLUX_OUTER => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   38  DIFF_R_P => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   39  DIFF_SN_P => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   40  DIFF_R_M => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   41  DIFF_SN_M => [ Astro::FITS::CFITSIO::TFLOAT(), '1E1' ],
#   42  FLAGS2 => [ Astro::FITS::CFITSIO::TSHORT(), '1I' ],
#   43  N_FRAMES => [ Astro::FITS::CFITSIO::TSHORT(), '1I' ],
#   ]
###################################################################


# Move these to table obj.
our $photo_min_mag = 0;
our $photo_max_mag = 40;

sub rejectrecord {
    my ($self, $row) = @_;
    my ($mag, $ra_sigma_deg, $dec_sigma_deg, $mag_sigma, $flags, $rawattr_v2) = @{$row}[5, 2, 4, 6, 8, 10];
    my $silly;
    my $dipole;

    # Filter on flags.  If the detection fails due to dipole parameters, the failure
    # is handled in readrecord() and $flags ($data[11]) is set.
    # 0x3888 is IPP-recommended flag.
    $silly = ($flags & 0x3888) ||
        # Toss stuff with strange sigmas or bad photometry.
        ($ra_sigma_deg eq 'nan' || $dec_sigma_deg eq 'nan' || $mag_sigma eq 'nan') ||
        ($ra_sigma_deg <= 0 || $dec_sigma_deg <= 0 || $mag_sigma <= 0) ||
        (($mag < $photo_min_mag) || ($mag > $photo_max_mag));

#exclude PSF_QF < 0.9
#exclude PSF_QF_PERFECT < 0.9 (or maybe 0.95)
#exclude (DIFF_R_P < 20) && finite
#exclude (DIFF_R_M < 20) && finite

    if ($rawattr_v2) {
        my ($psf_quality, $psf_qf_perfect, $diff_r_p, $diff_r_m) = @{$rawattr_v2}[7, 31, 38, 40];
        $dipole = ($psf_quality < 0.9) || 
            ($psf_qf_perfect ne 'nan' && $psf_qf_perfect < 0.9) ||
            ($diff_r_m ne 'nan' && $diff_r_m < 20) ||
            ($diff_r_p ne 'nan' && $diff_r_p < 20);
    }

    return ($silly or $dipole);
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
