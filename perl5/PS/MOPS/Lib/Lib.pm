package PS::MOPS::Lib;

use 5.008;
use strict;
use warnings;

use base qw(Exporter);

use Math::Trig;
use Math::Random;
use Astro::SLA;
use Astro::Time qw(cal2mjd);
use Carp;
use Params::Validate qw(:all);
use PS::MOPS::Constants qw(:all);

=head1 NAME

PS::MOPS::Lib - MOPS support library for Perl

=head1 SYNOPSIS

  use PS::MOPS::Lib qw(:all);

  # calculate OC number from MJD.
  $ocnum = mopslib_mjd2ocnum($mjd);      

  # calculate difference between two angles.
  $d_ang = mopslib_dang($ang1_deg, $ang2_deg);

  # calculate average and stdev
  ($avg, $std) = mopslib_stdev(@array);
  warn "couldn't calculate average" unless defined($avg);

=head1 DESCRIPTION

This module provides various support routines for MOPS code.

=cut

our %EXPORT_TAGS = ( 'all' => [ qw(
    mopslib_rndGaussian
    mopslib_normalizeRADEC
    mopslib_dang
    mopslib_calut2mjd
    mopslib_jd2mjd
    mopslib_mjd2jd
    mopslib_stdev
    mopslib_histogram
    mopslib_inSquareField
    mopslib_inField
    mopslib_toB62
    mopslib_fromB62
    mopslib_jd2ocnum
    mopslib_ocnum2jd
    mopslib_ocnum2mjd
    mopslib_mjd2ocnum
    mopslib_getOCNum
    mopslib_photoS2N
    mopslib_photoS2Nx
    mopslib_photoS2N4
    mopslib_photoS2N5
    mopslib_astroError
    mopslib_photoError
    mopslib_fuzzRDM
    mopslib_fuzzRDM2
    mopslib_fuzzRDM3
    mopslib_fuzzRDM4
    mopslib_fuzzRDM5
    mopslib_mutateControlObject
    mopslib_mjd2nn
    mopslib_nn2mjd
    mopslib_mjd2utctimestr
    mopslib_mjd2localtimestr
    mopslib_calculateDCriterion
    mopslib_V2filt
    mopslib_filt2V
    mopslib_effClassifyObjectNames
    mopslib_classifyObject
    mopslib_hsn2flux
    mopslib_flux2hsn
    mopslib_mag2fluxsn
    mopslib_parseOrbfitOptsFile
    mopslib_validOrbit
    mopslib_formatTimingMsg
    mopslib_computeXYIdx
    mopslib_computeSolarElongation
    mopslib_computeEclipticLatitude
    mopslib_computeGalacticLatitude
    mopslib_assembleTTITuples
    mopslib_fitsVersion
    log10
) ] );
our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );
our @EXPORT = qw(
    mopslib_rndGaussian
    mopslib_normalizeRADEC
    mopslib_dang
    mopslib_calut2mjd	
    mopslib_jd2mjd
    mopslib_mjd2jd
    mopslib_stdev
    mopslib_histogram
    mopslib_inSquareField
    mopslib_inField
    mopslib_toB62
    mopslib_fromB62
    mopslib_jd2ocnum
    mopslib_ocnum2jd
    mopslib_ocnum2mjd
    mopslib_mjd2ocnum
    mopslib_getOCNum
    mopslib_photoS2N
    mopslib_photoS2Nx
    mopslib_photoS2N4
    mopslib_photoS2N5
    mopslib_astroError
    mopslib_photoError
    mopslib_fuzzRDM
    mopslib_fuzzRDM2
    mopslib_fuzzRDM3
    mopslib_fuzzRDM4
    mopslib_fuzzRDM5
    mopslib_mutateControlObject
    mopslib_mjd2nn
    mopslib_nn2mjd
    mopslib_mjd2utctimestr
    mopslib_mjd2localtimestr
    mopslib_calculateDCriterion
    mopslib_V2filt
    mopslib_filt2V
    mopslib_effClassifyObjectNames
    mopslib_classifyObject
    mopslib_hsn2flux
    mopslib_flux2hsn
    mopslib_mag2fluxsn
    mopslib_parseOrbfitOptsFile
    mopslib_validOrbit
    mopslib_formatTimingMsg
    mopslib_computeXYIdx
    mopslib_computeSolarElongation
    mopslib_computeEclipticLatitude
    mopslib_computeGalacticLatitude
    mopslib_assembleTTITuples
    mopslib_fitsVersion
    log10
);

our $VERSION = '0.01';

sub mopslib_jd2mjd {

=item mopslib_jd2mjd

Convert JD to MJD; just subtract offset.

=cut
    return $_[0] - $MJD2JD_OFFSET;
}


sub mopslib_mjd2jd {

=item mopslib_mjd2jd

Convert MJD to JD; just add offset.

=cut

    return $_[0] + $MJD2JD_OFFSET;
}


my ($dz1, $dz2);
my $bSecondValueAvailable;

sub mopslib_rndGaussian {

=item mopslib_rndGaussian

Generate a value from a Gaussian random variable with specified sigma.

=head1 MORE INFO

This is the original C implementation of RJ's Gaussian error routine.
Return a random variable from a Gaussian distribution.  This algorithm
generates values in pairs, and module-wide globals are used to indicate
that a second value is available.

double rnddGaussian ( 
  double dMean,    /* @parm The mean  of  the desired gaussian distribution */
  double dSigma )  /* @parm The sigma for the desired gaussian distribution */
{
  double dFactor;
  double du1, du2;
  double dv1, dv2;
  double dr2 = 100.0;

  static double dz1, dz2;
  static BOOL bSecondValueAvailable = FALSE;

  if ( bSecondValueAvailable )
    {
      bSecondValueAvailable = FALSE;
      return dMean +  dz2 * dSigma;
    }

  while ( dr2 > 1.0 )
    {
      du1 = rndDouble ( 0.0, 1.0 );
      du2 = rndDouble ( 0.0, 1.0 );

      dv1 = 2.0 * du1 - 1.0;
      dv2 = 2.0 * du2 - 1.0;

      dr2 = dv1 * dv1 + dv2 * dv2;
    }

  dFactor = (double) sqrt ( -2.0 * log(dr2) / dr2 );

  dz1 = dv1 * dFactor;
  dz2 = dv2 * dFactor;

  bSecondValueAvailable = TRUE;

  return dMean + dz1 * dSigma;
}

=cut

    my ($dMean, $dSigma) = @_;
    my $dFactor;
    my ($du1, $du2);
    my ($dv1, $dv2);
    my $dr2 = 100.0;

    if ($bSecondValueAvailable) {
        $bSecondValueAvailable = 0;
        return $dMean + $dz2 * $dSigma;
    }

    while ($dr2 > 1 || $dr2 == 0) {
        $du1 = rand(1);
        $du2 = rand(1);
        $dv1 = 2 * $du1 - 1;
        $dv2 = 2 * $du2 - 1;
        $dr2 = $dv1 * $dv1 + $dv2 * $dv2;
    }

    $dFactor = sqrt(-2 * log($dr2) / $dr2);
    $dz1 = $dv1 * $dFactor;
    $dz2 = $dv2 * $dFactor;

    $bSecondValueAvailable = 1;
    return $dMean + $dz1 * $dSigma;
}


sub _fmod {
    my ($a, $b) = @_;
    if ($a < 0) {
        my $div = (-$a - int(-$a / $b) * $b);
        return $div != 0 ? $b - $div : 0;
    }
    else {
        return $a - int($a / $b) * $b;
    }
}

=head1 FUNCTIONS

=cut


sub mopslib_normalizeRADEC {

=item mopslib_normalizeRADEC

Normalize a given RA/DEC by wrapping the RA in [0, 360) and the
DEC in [-90, 90].  If the DEC exceeds 90 or -90 then correct the
RA appropriately.

=cut

    my ($ra_deg, $dec_deg) = @_;

    # First fold the DEC into (-180, +180).  If the dec is greater than +90
    # or less than -90 the RA changes by 180.
##    $dec_deg = ($dec_deg + 180) % 360 - 180;    # fold into (-180, +180)
    $dec_deg = _fmod($dec_deg + 180, 360) - 180;
    if ($dec_deg > 90) {
        $dec_deg = 180 - $dec_deg;
        $ra_deg += 180;
    }
    elsif ($dec_deg < -90) {
        $dec_deg = -180 - $dec_deg;
        $ra_deg += 180;
    }

##    return ($ra_deg % 360.0, $dec_deg);
    return (_fmod($ra_deg, 360), $dec_deg);
}


sub mopslib_stdev {
=item mopslib_stdev

Return average and standard deviation of a list of numbers.

=cut

    my $n;
    my $sum;
    my $avg;
    my $std;

    # Avg first.
    $n = scalar @_;     # num elems in list
    return undef unless $n > 0;
    $sum = 0;
    $sum += $_ foreach (@_);
    $avg = $sum / $n;   # average

    # Stdev.
    $sum = 0;
    $sum += ($_ - $avg) * ($_ - $avg) foreach (@_);
    $std = sqrt($sum / $n);

    return ($avg, $std);
}


sub mopslib_histogram {

=item mopslib_histogram

Calculate a histogram of input data.  Return a table as follows:

    * min => min of data
    * max => max of data
    * histogram => histogram data
    * binsize => bin size (width)

Inputs are

    * data => arrayref of data
    * bins => number of bins
    * binsize => size of bins
    * hmax => max histogram value; place all values > hmax in last bin

=cut

    my %args = validate(@_, {
        data => 1,
        bins => 0,
        binsize => 0,
        hmax => 0,
    });

    # Calculate a histogram of input data.  Return a table as follows:
    # {
    #   min => min of data
    #   max => max of data
    #   histogram => histogram data
    #   binsize => bin size (width)
    # }

    # Input should be arrayref and number of bins

    # This routine requires two passes: one to find min/max/stdev of data,
    # second to insert into bins.
    my $dmin;
    my $dmax;
    my $hmax;
    my $sum = 0;
    my $nd = scalar @{$args{data}};
    foreach my $d (@{$args{data}}) {
        if (!defined($dmin) or $d < $dmin) {
            $dmin = $d;
        }
        if (!defined($dmax) or $d > $dmax) { 
            $dmax = $d;
        }
    }

    $hmax = defined($args{hmax}) ? $args{hmax} : $dmax; # set max hist value
    my $r = $hmax - $dmin;                        # data range
    if ($r == 0) {
        return {
            numpts => $nd,
            min => $dmin,
            max => $hmax,
            hist => [ $nd ], # single-valued
            binsize => 0,
        };
    }

    my @hist;
    my $b;
    if ($args{binsize}) {
        $b = int($r / $args{binsize}) - 1;
        $b = 9 if $b < 9;
        $hist[$b] = 0;     # auto-vivify last element
        foreach my $d (@{$args{data}}) {
            $d = $hmax if ($d > $hmax);
            $hist[int(($d - $dmin) / $r * $b + 0.99999999)]++;
        }
    }
    else {
        $b = $args{bins} - 1;
        $hist[$b] = 0;     # auto-vivify last element
        foreach my $d (@{$args{data}}) {
            $d = $hmax if ($d > $hmax);
            $hist[int(($d - $dmin) / $r * $b + 0.99999999)]++;
        }
    }

    # Set untouched hist positions to zero (otherwise they're undef).
    @hist = map { !defined($_) ? 0 : $_ } @hist;

    return {
        min => $dmin,
        max => $hmax,
        hist => \@hist,
        bins => $b + 1,
        binsize => ($hmax - $dmin) / $b,
        numpts => $nd,
    };
}


sub mopslib_inSquareField {

=item mopslib_inSquareField

Return whether a point is inside a square field, accounting for spherical
curvature.  All inputs in degrees, converted to rad on the fly.

=cut

    # fc == 'field center'
    # fov = 'field of view' (length of side of field)
    # pt = point to test
    my ($fc_ra, $fc_dec, $fov, $pt_ra, $pt_dec) = map { $_ / $DEG_PER_RAD } @_; # convert deg to rad

    my $dra;        # delta RA
    my $cos_dra;    # cos delta RA
    my $num1;
    my $num2;
    my $den;
    my $in;
    my $eta;
    my $xi;

    if (slaDsep($fc_ra, $fc_dec, $pt_ra, $pt_dec) < $PI / 2) {
        $dra = $fc_ra - $pt_ra;
        $cos_dra = cos($dra);
        $num1 = cos($pt_dec) * sin($dra);
        $num2 = cos($fc_dec) * sin($pt_dec) - sin($fc_dec) * cos($pt_dec) * $cos_dra;
        $den = sin($fc_dec) * sin($pt_dec) + cos($fc_dec) * cos($pt_dec) * $cos_dra;

        if ($den > 0) {
            $eta = $num1 / $den;
            $xi = $num2 / $den;

            if (abs($xi) < $fov / 2 and abs($eta) < $fov / 2) {
                $in = 1;
            }
        }
    }

    return $in;
}


sub mopslib_inField {

=item mopslib_inField

Return whether a point is inside a circular field.  All inputs are in degrees.

=cut

    # fc == 'field center'
    # fov = 'field of view' (diameter of field)
    # pt = point to test
    my ($fc_ra, $fc_dec, $fov, $pt_ra, $pt_dec) = map { $_ / $DEG_PER_RAD } @_; # convert deg to rad
    return slaDsep($fc_ra, $fc_dec, $pt_ra, $pt_dec) < $fov / 2;
}


sub mopslib_dang {

=item mopslib_dang

Calculate the difference ($a1 - $a2) between two angles, handling wraparound at 0/360.
Inputs and outputs are degrees.  Angles are folded into (-180, 180) before calculating
difference.

If the difference is larger than 180 degrees, 360 is subtracted from it.
If the difference is less than -180 degrees, 360 is added to it.

We should probably use a SLALIB routine now instead.

=cut

    my ($a1, $a2) = @_;

    # Fold out-of-range values.
    ($a1, $a2) = map {
        if ($_ > 180) {
            $_ -= (int($_ / 360 + 0.5)) * 360;
        }
        elsif ($_ < -180) {
            $_ += (int(-$_ / 360 + 0.5)) * 360;
        }
        $_;
    } ($a1, $a2);

    my $d = $a1 - $a2;
    if ($d > 180) {
        $d -= 360;
    }
    elsif ($d < -180) {
        
    }
    return $d;
}


sub mopslib_toB62 {

=item mopslib_toB62

Convert an integer value to base-62.  Inputs are an integer to convert
and a template of the form 'BAZ0000000'.  The converted base-62 value
is overlaid into the zeros.

=cut

    my ($num, $template) = @_;
    my $prefix;
    my $len;
    my $res;
    if ($template =~ /^([^0]*)(0+)$/) {
        $prefix = $1;
        $len = length($2);

        my $m = '';
        my $base = 62;
        while ($num) {
            $m .= substr($B62_CHARS, $num % $base, 1);
            $num = int($num / $base);
        };

        $res = $prefix . ('0' x ($len - length($m))) . (scalar reverse $m);
    }
    return $res;
}


sub mopslib_fromB62 {

=item mopslib_fromB62

Convert a base-62 string to an integer.  We use this routine
to perform conversions to code that has a 32-bit ID limit,
e.g. orbfit.

OK, we defined our original B62 conversion stupidly, should have used a
continous ASCII sequence, but instead we chopped it up.  So handle
the three ranges separately:

0-9
a-z
A-Z

(we should have done)

0-9
A-Z
a-z

=cut

    my ($b62) = @_;
    my @chars = split //, $b62;
    my $c;
    my $num = 0;
    my $power = 1;
    my $idx;

    while (defined($c = pop @chars)) {
        $idx = ord($c);
        # check if in '0'..'9' (48..57)
        if ($idx >= 48 and $idx <= 57) {
            $idx -= 48;
        }
        # check if in 'a'..'z' (97..122)
        elsif ($idx >= 97 and $idx <= 122) {
            $idx = $idx - 97 + 10;
        }
        # check if in 'A'..'Z' (65..90)
        elsif ($idx >= 65 and $idx <= 90) {
            $idx = $idx - 65 + 36;
        }
        else {
            die "strange character in base-62 string: $c";
        }

        $num += $idx * $power;
        $power *= 62;
    }

    return $num;
}


sub _round {
    my ($number) = shift;
    return int($number + 0.5 * ($number <=> 0));
}


sub mopslib_jd2ocnum {

=item mopslib_jd2ocnum

Compute the MOPS OC number for a given JD.

=cut

    my $t = shift;
    return _round((((int($t - 0.5) + 0.5) - $OC_TREF_0HUT_JD) / $OC_SYNODIC_PERIOD) - 0.49999999999); 
    # round(x - 0.5) == floor(x)
}


sub mopslib_ocnum2jd {
    # Convert an integer OC number to the JD at which the OC begins.  Note that OC number
    # is defined to change at UT=0h.
    my $ocnum = int(shift);     # only accept integer OC nums
    my $full_moon_jd = $OC_TREF_0HUT_JD + $OC_SYNODIC_PERIOD * $ocnum;    # time of full moon
    return int($full_moon_jd);
}


sub mopslib_ocnum2mjd {
    my $ocnum = shift;
    return mopslib_jd2mjd(mopslib_ocnum2jd($ocnum));
}


sub mopslib_mjd2ocnum {

=item mopslib_mjd2ocnum

Compute the MOPS OC number for a given MJD.

=cut

    return mopslib_jd2ocnum(map { $_ + $MJD2JD_OFFSET } @_);
}


sub mopslib_getOCNum {

=item mopslib_getOCNum

Return the OC number of the date specified, or the current date
if not date is specified.

=cut

    my ($mjd) = shift;
    my $ocnum;
    $mjd ||= now2mjd();                 # use Astro::Time::now2mjd() if unspecified
    return mopslib_mjd2ocnum($mjd);
}


# Some constants for S2N calculation.  See PS1 DRM, Table 1.
my $PS = 2.0;                   # sqrt(4)
my $C = 1.00017;                # multiplicative constant

# M1 + MU changing per RJ/PP/TG discussion 4/2006.
#my ($M1, $MU) = (0, 45.6);      # from PS1 DRM
my ($M1, $MU) = (0, 46.2);      # from PS1 DRM
my $FWHM_as = 0.6;              # FWHM of PSF in arcsecs

sub mopslib_photoS2N {

=item mopslib_photoS2N

Returns photometric signal-to-noise as a function of apparent magnitude
and exposure time.

=cut

    my ($M, $expTime_s) = @_;           # get input apparent magnitude, exposure time
    $expTime_s ||= 30;                  # default if unspecified
    my $S2N = 
        $PS * $C * 10 ** (-0.2 * (2 * $M - $MU - $M1)) *
        sqrt($expTime_s / $PI / ($FWHM_as ** 2));
    return $S2N;
}


sub mopslib_photoS2Nx {
    # Returns photometric signal-to-noise as a function of apparent magnitude,
    # filter selection, and input filter configuration.
    my ($M, $filter, $s2n_config_hash) = @_;            # get input apparent magnitude, exposure time

    my $expTime_s = $s2n_config_hash->{$filter}->{exposure_time_sec};
    die "can't get exposure_time_sec for filter $filter" unless $expTime_s;     # fatal
    my $M1plusMU = $s2n_config_hash->{$filter}->{M1plusMU};
    die "can't get M1plusMU for filter $filter" unless $M1plusMU;               # also fatal
    my $PS_factor = $s2n_config_hash->{PS};
    die "can't get PS_factor" unless $PS_factor;

    my $S2N =
        $PS_factor * $C * 10 ** (-0.2 * (2 * $M - $M1plusMU)) *
        sqrt($expTime_s / $PI / ($FWHM_as ** 2));
    return $S2N;
}


sub mopslib_photoS2N4 {

=item mopslib_photoS2N4    

Returns photometric signal-to-noise as a function of apparent magnitude,
filter selection, and input filter configuration.

=cut    
    my ($M, $filter, $exposure_time_s, $fwhm_arcsec, $diff_mag, $s2n_config_hash) = @_;            

    my $M1plusMU = $s2n_config_hash->{$filter}->{M1plusMU};
    die "can't get M1plusMU for filter $filter" unless $M1plusMU;               # also fatal
    my $PS_factor = $s2n_config_hash->{PS};
    die "can't get PS_factor" unless $PS_factor;

    my $S2N =
        $PS_factor * $C * 10 ** (-0.2 * (2 * $M - $M1plusMU + $diff_mag)) *
        sqrt($exposure_time_s / $PI / ($fwhm_arcsec ** 2));
    return $S2N;
}


sub log10 {
    my $n = shift;
    return log($n)/log(10);
}
    

sub mopslib_photoS2N5 {

=item mopslib_photoS2N5    

Returns photometric signal-to-noise as a function of apparent magnitude.
Signal-to-noise will be calculated on a per-exposure basis using information 
provided by IPP.

For a detailed description on how the signal to noise value is calculated see
the MOPS wiki at 
http://ps1sc.ifa.hawaii.edu/PS1wiki/images/IPP_Synthetic_S_N_investigations.pdf
=cut 
    # Variables
    #   $zero_point   - Zero point magnitude.
    #   $read_noise   - Read noise reported as a count.
    #   $mag          - Object brightness reported as a magnitude.
    #   $sky          - Average sky background count of exposure.
    #   $seeing       - Measured seeing (fwhm) at diff stage in pixels
    #   $scale_factor - Scaling factor to use to adjust the calculated signal to
    #                   noise. If not specified then it will default to 1
    # Process parameters
    my %args = validate(@_,  {
        mag => 1,         
        read_noise => 0,    
        zero_point => 1,
        sky =>1,
        seeing =>1,
        scale_factor => 0,
    });
    my $zp = $args{zero_point};
    my $mag = $args{mag};
    my $sky = $args{sky};
    my $rn = $args{read_noise};
    # Set scale factor to 1 if it is not specified.
    my $scale_factor = defined($args{scale_factor}) ? $args{scale_factor} : 1;
    
    # Randomly generate read noise assuming a normal distrubution with a mean 
    # of 5.44 and a standard deviation of 0.56.
    my $RN_MEAN = 5.55;
    my $RN_STDEV = 0.56;
    unless ($rn) {
        while (1) {
            $rn = random_normal(1, $RN_MEAN, $RN_STDEV);
            # Break out of loop if randomly generated read noise is within 3  
            # standard deviation of the read noise mean.
            last if ($rn >=($RN_MEAN - 3 * $RN_STDEV) && $rn <= ($RN_MEAN + 3 * $RN_STDEV));
        }
    }
    my $seeing = $args{seeing};
    # Ensure that a valid seeing value is specified to avoid a divide by zero error.
    die "Seeing (fwhm) for field cannot be zero or negative!" if ($seeing <= 0);
    
    my $sigma_sq = ($seeing ** 2) / 5.5452;
    my $flux = (10 ** (0.4 * ($zp - $mag))) * (1 - exp(-1 / (2 * $sigma_sq)));       
        
    # Calculate the signal to noise
    my $s2n = $scale_factor * ($flux / sqrt($flux + 2 * Math::Trig::pi * ($sky + ($rn ** 2) + 0.289)));   
        
    return $s2n;
}


sub mopslib_astroError {

=item mopslib_astroError

Returns astrometric error in arcseconds as a function of apparent magnitude.

=cut

    my ($M, $S2N) = @_;
    $S2N ||= mopslib_photoS2N($M);     # get signal-to-noise at this mag (if not specified)
    if ($S2N <= 1.5) {
#        warn "got S/N < 0: $M, $S2N";
        return 1.5;
    }

    my $astroError = 0.070 * ($FWHM_as / 0.6) * (5.0 / $S2N);
    return $astroError;
}


my $_log10 = log(10);
sub mopslib_photoError {

=item mopslib_photoError

Returns photometric error in arcseconds as a function of apparent magnitude.

=cut

    my ($M, $S2N) = @_;
    $S2N ||= mopslib_photoS2N($M);
    if ($S2N <= 1.5) {
#        warn "got S/N < 0: $M, $S2N";
        $S2N = 1.5;       # something very low
    }

    my $arg = 1 - 1.0 / $S2N;
    if ($arg < 0) {
#        warn "S2N < 1: $S2N";
        return 0;
    }

    my $photoError = -2.5 * log($arg) / $_log10;  # log10(x) = log(x) / log(10)
    return $photoError; 
}


sub mopslib_fuzzRDM {

=item mopslib_fuzzRDM

Fuzz an RA/DEC/Mag triplet using the PS MOPS error model.  RA and
Dec must be specified in degrees.

=cut

    my ($in_ra_deg, $in_dec_deg, $M, $bias_arcsec) = @_;    # RA, DEC, mag
    $bias_arcsec ||= 0;                                     # 0 if not specified
    my $S2N = mopslib_photoS2N($M);
    my $astro_error_arcsec = mopslib_astroError($M, $S2N) + $bias_arcsec;
    my $out_ra_deg;
    my $out_dec_deg;

    if ($in_dec_deg == 90) {
        $out_ra_deg = $in_ra_deg;       # does not compute
    }
    else {
        $out_ra_deg = $in_ra_deg + mopslib_rndGaussian(0, $astro_error_arcsec / 3600) / 
            cos($in_dec_deg / $DEG_PER_RAD);
    }
    $out_dec_deg = $in_dec_deg + mopslib_rndGaussian(0, $astro_error_arcsec / 3600);

    my $photo_error = mopslib_photoError($M, $S2N);
    my $new_M = $M + mopslib_rndGaussian(0, $photo_error);
    return mopslib_normalizeRADEC($out_ra_deg, $out_dec_deg), $new_M;
#    return ($out_ra_deg, $out_dec_deg);
}


sub mopslib_fuzzRDM2 {

=item mopslib_fuzzRDM2

Fuzz an RA/DEC/Mag triplet using the PS MOPS error model.  RA and
Dec must be specified in degrees.  Return fuzzed RA, Dec and mag,
and sigmas.

=cut

    my ($in_ra_deg, $in_dec_deg, $M, $bias_arcsec) = @_;    # RA, DEC, mag
    $bias_arcsec ||= 0;                                     # 0 if not specified

    # Photometry.
    my $S2N = mopslib_photoS2N($M);                         # compute nominal S2N from perfect mag
    my $photo_error = mopslib_photoError($M, $S2N);         # photometric error at this S2N
    my $new_M = $M + mopslib_rndGaussian(0, $photo_error);  # randomize mag from photo error
    my $new_S2N = mopslib_photoS2N($new_M);                 # back-out new S2N from fuzzed mag

    # Astrometry.
    my $astro_error_arcsec = mopslib_astroError($new_M, $new_S2N) + $bias_arcsec;
    my $out_ra_deg;
    my $out_dec_deg;
    my $ra_sigma_deg;
    my $dec_sigma_deg;

    if ($in_dec_deg == 90) {
        $ra_sigma_deg = 0;
        $out_ra_deg = $in_ra_deg;       # does not compute
    }
    else {
        $ra_sigma_deg = $astro_error_arcsec / 3600;
        $out_ra_deg = $in_ra_deg + mopslib_rndGaussian(0, $ra_sigma_deg) / 
            cos($in_dec_deg / $DEG_PER_RAD);
    }

    $dec_sigma_deg = $astro_error_arcsec / 3600;
    $out_dec_deg = $in_dec_deg + mopslib_rndGaussian(0, $dec_sigma_deg);

    my ($new_ra_deg, $new_dec_deg) = mopslib_normalizeRADEC($out_ra_deg, $out_dec_deg);
    return { 
        RA_DEG => $new_ra_deg,
        DEC_DEG => $new_dec_deg,
        MAG => $new_M,
        RA_SIGMA_DEG => $ra_sigma_deg,
        DEC_SIGMA_DEG => $dec_sigma_deg,
        MAG_SIGMA => $photo_error,
        S2N => $new_S2N,
    };
}


sub mopslib_fuzzRDM3 {

=item mopslib_fuzzRDM3

Fuzz an RA/DEC/Mag triplet using the PS MOPS error model given a S/N
and PS filter configuration.  RA and Dec must be specified in degrees.
Return fuzzed RA, Dec and mag, and sigmas, and a synthetic S/N.

=cut

    # ideal RA, DEC, mag; bias (min possible error); filter, s2n_config
    my ($in_ra_deg, $in_dec_deg, $in_mag, $bias_arcsec, $filter, $s2n_config_href) = @_;    

    # Photometry.
    my $S2N = mopslib_photoS2Nx($in_mag, $filter, $s2n_config_href);         # compute nominal S2N from perfect mag
    my $photo_error = mopslib_photoError($in_mag, $S2N);                    # photometric error at this S2N
    my $new_mag = $in_mag + mopslib_rndGaussian(0, $photo_error);           # randomize mag from photo error
    my $new_S2N = mopslib_photoS2Nx($new_mag, $filter, $s2n_config_href);    # back-out new S2N from fuzzed mag

    # Astrometry.
    my $astro_error_arcsec = mopslib_astroError($new_mag, $new_S2N) + $bias_arcsec;
    my $out_ra_deg;
    my $out_dec_deg;
    my $ra_sigma_deg;
    my $dec_sigma_deg;

    if ($in_dec_deg == 90) {
        $ra_sigma_deg = 0;
        $out_ra_deg = $in_ra_deg;       # does not compute
    }
    else {
        $ra_sigma_deg = $astro_error_arcsec / 3600;
        $out_ra_deg = $in_ra_deg + mopslib_rndGaussian(0, $ra_sigma_deg) / 
            cos($in_dec_deg / $DEG_PER_RAD);
    }

    $dec_sigma_deg = $astro_error_arcsec / 3600;
    $out_dec_deg = $in_dec_deg + mopslib_rndGaussian(0, $dec_sigma_deg);

    my ($new_ra_deg, $new_dec_deg) = mopslib_normalizeRADEC($out_ra_deg, $out_dec_deg);
    return { 
        RA_DEG => $new_ra_deg,
        DEC_DEG => $new_dec_deg,
        MAG => $new_mag,
        RA_SIGMA_DEG => $ra_sigma_deg,
        DEC_SIGMA_DEG => $dec_sigma_deg,
        MAG_SIGMA => $photo_error,
        S2N => $new_S2N,
    };
}


sub mopslib_fuzzRDM4 {

=item mopslib_fuzzRDM4

Fuzz an RA/DEC/Mag triplet using the PS MOPS error model given RA, DEC,
filter, observed mag, exposure time, filter, FWHM, intrinsic astrometric
error (bias), diff noise, pixel scale (arcsec/px) and filter S/N config.
Return fuzzed RA, Dec and mag, and sigmas, and a synthetic S/N.

=cut

    # ideal RA, DEC, mag; bias (min possible error); filter, s2n_config
    my ($in_ra_deg, $in_dec_deg, $in_mag, $filter, $exposure_time_s, $bias_arcsec, $fwhm_arcsec, $diff_mag, $pixel_scale, $s2n_config_href) = @_;    

    # Photometry.
    #
    # compute nominal S2N from perfect mag
    my $S2N = mopslib_photoS2N4($in_mag, $filter, $exposure_time_s, $fwhm_arcsec, $diff_mag, $s2n_config_href);        
    my $photo_error = mopslib_photoError($in_mag, $S2N);                    # photometric error at this S2N
    my $new_mag = $in_mag + mopslib_rndGaussian(0, $photo_error);           # randomize mag from photo error

    # back-out new S2N from fuzzed mag
    my $new_S2N = mopslib_photoS2N4($new_mag, $filter, $exposure_time_s, $fwhm_arcsec, $diff_mag, $s2n_config_href);   

    # Astrometry.
    my $astro_error_arcsec = mopslib_astroError4($new_S2N, $bias_arcsec, $pixel_scale);
    my $out_ra_deg;
    my $out_dec_deg;
    my $ra_sigma_deg;
    my $dec_sigma_deg;

    if ($in_dec_deg == 90) {
        $ra_sigma_deg = 0;
        $out_ra_deg = $in_ra_deg;       # does not compute
    }
    else {
        $ra_sigma_deg = $astro_error_arcsec / 3600;
        $out_ra_deg = $in_ra_deg + mopslib_rndGaussian(0, $ra_sigma_deg) / 
            cos($in_dec_deg / $DEG_PER_RAD);
    }

    $dec_sigma_deg = $astro_error_arcsec / 3600;
    $out_dec_deg = $in_dec_deg + mopslib_rndGaussian(0, $dec_sigma_deg);

    my ($new_ra_deg, $new_dec_deg) = mopslib_normalizeRADEC($out_ra_deg, $out_dec_deg);
    return { 
        RA_DEG => $new_ra_deg,
        DEC_DEG => $new_dec_deg,
        MAG => $new_mag,
        RA_SIGMA_DEG => $ra_sigma_deg,
        DEC_SIGMA_DEG => $dec_sigma_deg,
        MAG_SIGMA => $photo_error,
        S2N => $new_S2N,
    };
}

sub mopslib_fuzzRDM5 {

=item mopslib_fuzzRDM5

Fuzz an RA/DEC/Mag triplet using the PS MOPS error model given RA, DEC,
observed mag, exposure time, intrinsic astrometric error (bias), diff noise, 
and pixel scale (arcsec/px).
Return fuzzed RA, Dec and mag, and sigmas, and a synthetic S/N.

=cut
    my %args = validate(@_,  {
        ra => 1,          # right ascension
        dec => 1,         # declination
        mag => 1,         # magnitude
        bias => 1,        # bias in arc seconds
        pixel_scale => 1, # pixel scale (arcsec/pixel)
        seeing => 1,      # measured seeing at diff stage in arcsec
        zero_point => 1,  # zero point as a magnitude
        sky => 1,         # average sky background count of exposure
        scale_factor => 0,# S2N scaling factor 
    });

    my $in_ra_deg = $args{ra};
    my $in_dec_deg = $args{dec};
    my $in_mag = $args{mag};
    my $bias_arcsec = $args{bias};
    my $pixel_scale = $args{pixel_scale};
    my $zero_point = $args{zero_point};
    my $exp_time = $args{exp_time};
    my $sky = $args{sky};
    my $seeing = $args{seeing};
    my $scale_factor = $args{scale_factor};
    
    # Photometry.
    #
    # compute nominal S2N from perfect mag
    my $S2N = mopslib_photoS2N5(mag => $in_mag, zero_point => $zero_point, 
              seeing => $seeing, sky => $sky, scale_factor => $scale_factor);
    my $photo_error = mopslib_photoError($in_mag, $S2N);          # photometric error at this S2N
    my $new_mag = $in_mag + mopslib_rndGaussian(0, $photo_error); # randomize mag from photo error

    # back-out new S2N from fuzzed mag
    my $new_S2N = mopslib_photoS2N5(mag => $new_mag, zero_point => $zero_point, 
                  seeing => $seeing, sky => $sky, scale_factor => $scale_factor);  

    # Astrometry.
    my $astro_error_arcsec = mopslib_astroError4($new_S2N, $bias_arcsec, $pixel_scale);
    my $out_ra_deg;
    my $out_dec_deg;
    my $ra_sigma_deg;
    my $dec_sigma_deg;

    if ($in_dec_deg == 90) {
        $ra_sigma_deg = 0;
        $out_ra_deg = $in_ra_deg;       # does not compute
    }
    else {
        $ra_sigma_deg = $astro_error_arcsec / 3600;
        $out_ra_deg = $in_ra_deg + mopslib_rndGaussian(0, $ra_sigma_deg) / 
            cos($in_dec_deg / $DEG_PER_RAD);
    }

    $dec_sigma_deg = $astro_error_arcsec / 3600;
    $out_dec_deg = $in_dec_deg + mopslib_rndGaussian(0, $dec_sigma_deg);

    my ($new_ra_deg, $new_dec_deg) = mopslib_normalizeRADEC($out_ra_deg, $out_dec_deg);
    return { 
        RA_DEG => $new_ra_deg,
        DEC_DEG => $new_dec_deg,
        MAG => $new_mag,
        RA_SIGMA_DEG => $ra_sigma_deg,
        DEC_SIGMA_DEG => $dec_sigma_deg,
        MAG_SIGMA => $photo_error,
        S2N => $new_S2N,
    };
}
sub mopslib_astroError4 {

=item mopslib_astroError

Returns astrometric error in arcseconds as a function of baseline 1-sigma astrometry and S/N.
Pixel scale should be in arcsec/px.  The constant .26923076923076923076 comes from the
ratio of PS1 pixel scale (0.25 arcsec/px) to 0.070.

=cut

    my ($S2N, $bias_as, $pixel_scale) = @_;
    # Pixel scale may be negative for a flipped array so take the absolute value.
    $pixel_scale = abs($pixel_scale);
    my $astroError = sqrt($bias_as ** 2 + ($pixel_scale * .269230 * (5.0 / $S2N)) ** 2);     # add in quadrature
    return $astroError;
}


sub mopslib_mutateControlObject {
=item mopslib_mutateControlObject

Mutate a detection by specific rules for a synthetic detection from a
MOPS control SSM object.  This function applies specific alterations to
synthetic detections so that they may be monitored by the MOPS software.
The object name prefix will indicate the mutating action to apply.
Rules are:

SM8 : rotate the direction vector 180 degrees if the current time in
seconds is odd.  Effect: should result in unlinkable tracklets.

Everything else: do nothing
=cut

    my ($det, $time) = @_;
    if ($det->{objectName} =~ /^SM0/ and defined($det->{orient_deg}) and ($time || time) % 2) {
        my $new_orient_deg = $det->{orient_deg};
        $new_orient_deg += 180;                             # rotate 180 deg
        $new_orient_deg -= 360 if $new_orient_deg > 180;    # fold into (-180, 180]
        $det->{orient_deg} = $new_orient_deg;               # assign new rot
    }
}


sub mopslib_mjd2nn {

=item mopslib_mjd2nn

Convert an MJD and local time offset to a night number, the integer
MJD at the trailing edge of local noon.

=cut

    my ($mjd, $local_offset_hours) = @_;
    croak('local_offset_hours unspecified') unless defined($local_offset_hours);
    return int($mjd - 0.5 + $local_offset_hours / 24);
}


sub mopslib_nn2mjd {

=item mopslib_nn2mjd

Convert a night number to an MJD.  The time range MJD to MJD+1
is noon to noon localtime; all observations for a single night
occur in this range.

=cut

    my ($nn, $local_offset_hours) = @_;
    croak('local_offset_hours unspecified') unless defined($local_offset_hours);
    return $nn + 0.5 - $local_offset_hours / 24;
}

sub mopslib_calut2mjd {

=item mopslib_calut2mjd

Convert a calendar date and time (in UTC) to a floating-point MJD.
=cut
my ($yr, $mon, $day, $hr, $min, $sec) = @_;

# Convert time to a day fraction
my $SECONDS_IN_DAY = 86400;
my $seconds = (($hr * 60 + $min) * 60) + $sec;
my $day_frac = $seconds / $SECONDS_IN_DAY;

return Astro::Time::cal2mjd($day, $mon, $yr, $day_frac);

}

sub mopslib_mjd2utctimestr {

=item mopslib_mjd2utctimestr

Convert a floating-point MJD to UTC time string.
=cut

    my ($mjd, $sep) = @_;
    $sep ||= 'T';       # default separator between date and time ("-" to keep as one word)
    my ($year, $month, $day, $ut, $err);
    slaDjcl($mjd, $year, $month, $day, $ut, $err);
    my (@hmsf);
    my $sign = '+';
    slaCd2tf(0, $ut, $sign, @hmsf);
    return sprintf "%4d-%02d-%02d$sep%02d:%02d:%04.1fZ", $year, $month, $day, @hmsf[0..2];
}


sub mopslib_mjd2localtimestr {

=item mopslib_mjd2localtimestr

Convert a floating-point MJD to local time string.
=cut

    my ($mjd, $local_offset_hours, $sep) = @_;
    croak('local_offset_hours unspecified') unless defined($local_offset_hours);
    $sep ||= 'T';       # default separator between date and time ("-" to keep as one word)
    my ($year, $month, $day, $ut, $err);
    slaDjcl($mjd + $local_offset_hours / 24, $year, $month, $day, $ut, $err);
    my (@hmsf);
    my $sign = '+';
    slaCd2tf(0, $ut, $sign, @hmsf);
    return sprintf "%4d-%02d-%02d$sep%02d:%02d:%04.1f", $year, $month, $day, @hmsf[0..2];
}


sub mopslib_calculateDCriterion {

=item mopslib_calculateDCriterion

Calculate 3- and 4-element D-critera from two input orbits.  Reference:
Jedicke D-criterion email, Southworth and Hawkins (1963), Drummond
(2000).

=cut

    my ($orb1, $orb2) = @_;
    my $D3;         # 3-element D-crietrion
    my $D4;         # 4-element D-crietrion
    my $i1_rad = $orb1->i / $DEG_PER_RAD;
    my $i2_rad = $orb2->i / $DEG_PER_RAD;
    my $omega1_rad = $orb1->node / $DEG_PER_RAD;
    my $omega2_rad = $orb2->node / $DEG_PER_RAD;
    my $arg;

    my $d1 = ($orb1->q - $orb2->q); 
    $d1 *= $d1;

    my $d2 = ($orb1->e - $orb2->e); 
    $d2 *= $d2;

    $arg = cos($i1_rad) * cos($i2_rad) + 
        sin($i1_rad) * sin($i2_rad) * cos($omega1_rad - $omega2_rad);
    if ($arg > 1) {
        $arg = 1;
    }
    elsif ($arg < -1) {
        $arg = -1;
    }
    my $I_rad = acos($arg);

    $arg = cos(($i1_rad + $i2_rad) / 2) 
            * sin(($omega1_rad - $omega2_rad) / 2) 
            * sec($I_rad / 2);
    if ($arg > 1) {
        $arg = 1;
    }
    elsif ($arg < -1) {
        $arg = -1;
    }
    my $II_rad = ($orb1->argPeri - $orb2->argPeri) / $DEG_PER_RAD 
        + 2 * asin($arg);

    my $d3 = 2 * sin($I_rad / 2); 
    $d3 *= $d3;

    my $d4 = ($orb1->e + $orb2->e) * sin($II_rad / 2);
    $d4 *= $d4;

    $D3 = sqrt($d1 + $d2 + $d3);
    $D4 = sqrt($d1 + $d2 + $d3 + $d4);

    return ($D3, $D4);
}


# Conversion from Johnson mags to V-band.
our %_v2filt = (
    'g' =>  0.01,
    'r' =>  0.23,
    'i' =>  1.22,
    'z' =>  0.70,
    'U' =>  1.10,
    'J' =>  1.10,
    ' ' => -0.80,
    'B' => -0.80,
    'R' =>  0.40,
    'I' =>  0.80,
    'C' =>  0.40,
);


# Conversions from PS1/Sloan AB mags to Johnson mags.  These contants must be SUBTRACTED
# to the AB mag to get the Johnson mag.
our %_AB2filt = (
    'u' => 0.981,
    'g' => -0.093,
    'r' => 0.166,
    'i' => 0.397,
    'z' => 0.572,
    'y' => 0.0,         # unknown so far
);


# Filter corrections from PS1 mags to V based on work from Fitzsimmons.
# http://ps1sc.ifa.hawaii.edu/PS1wiki/images/Fitzsimmons.TheoreticalAsteroidColours-v3.pdf
# pg. 3 Johnson-PS1 Colours
# E.g., given a reported g mag, the corresponding V is g + 0.28.
our %_ps1v2filt = (
    'g' => -0.28,
    'r' => 0.23,
    'i' => 0.39,
    'z' => 0.37,
    'y' => 0.36,
    'w' => 0.16,        # same as g for now
);

=item mopslib_V2filt, mopslib_filt2V

Convert a magnitude using the specified filter to V-band, using various
simplistic conversion rules.  If there is no match in the lookup table,
use zero.

We now accept an optional $cvt hashref that contains the filter
conversions.

Conversion to V from:

  g : +0.01
  r : +0.23
  U : +1.1
  J : +1.1
  ' ' : -0.8
  B : -0.8
  R : +0.4
  I : +0.8
  C : +0.4
  z : +0.7
  i : +1.22

=cut

sub mopslib_V2filt {
    my ($mag, $filt, $cvt) = @_;
    $cvt ||= \%_ps1v2filt;

    # Old Johnson+AB conversion.
    #return $mag + ($_AB2filt{$filt} || 0.0) - ($_v2filt{$filt} || 0.0);

    # PS1 conversion (preliminary).
    return $mag - ($cvt->{$filt} || 0.0);
}


sub mopslib_filt2V {
    my ($mag, $filt, $cvt) = @_;
    $cvt ||= \%_ps1v2filt;

    # Old Johnson+AB conversion.
    #return $mag - ($_AB2filt{$filt} || 0.0) + ($_v2filt{$filt} || 0.0);

    # PS1 conversion (preliminary).
    return $mag + ($cvt->{$filt} || 0.0);
}


sub mopslib_effClassifyObjectNames {
    my (@names) = @_;
    # Given a list of object names, classify the list as CLEAN, MIXED, BAD, or NONSYNTHETIC.
    # Names beginning with 'S' are Pan-STARRS synthetic objects; everything else is non-synthetic.  
    my %synth;              # track synth objs
    my $nonsynth = 0;       # count nonsynth
    my $name;
    foreach $name (@names) {
        if ($name and $name =~ /^S/) {
            $synth{$name}++;       # count num dets for this obj
        }
        else {
            $nonsynth++;
        }
    }

    # See what we've got.
    if (scalar keys %synth == 0) {
        return $MOPS_EFF_NONSYNTHETIC;          # no synthetic dets
    }
    elsif (scalar keys %synth == 1 and $nonsynth == 0) {
        return $MOPS_EFF_CLEAN;
    }
    elsif (scalar keys %synth > 1 and $nonsynth == 0) {
        return $MOPS_EFF_MIXED;                 # num synth > 1, no nonsynth, no mixed
    }
    else {
        return $MOPS_EFF_BAD;                   # must be bad (synth + nonsynth)
    }
}


sub mopslib_classifyObject {

=item mopslib_classifyObject

Generate a three-charactor (or fewer) string classifying on orbit based on
orbital elements and Tisserand parameter, as follows:

PHO (potentially hazardous object) : MOID <= 0.05 AU

NEO (near-Earth object) : p < 1.30 AU

MC (Mars-crosser) : 1.30 AU < p <= 1.67 AU

MB (main belt) : 1.67 AU < a <= 5.00 AU

TRO (Jupiter) : 5 AU < a <= 5.5 AU

CEN (Centaur) : 5.5 AU < a <= 35.0 AU

TNO (trans-Neptunian) : 35.0 AU < a <= 50.0 AU

SDO (scattered disk) : a > 50.0 AU

SPC (short-period comet) : 2 < T < 3

HFC (Halley-family comet) : T <= 2 and a <= 34.2 AU

LPC (long-period comet) : T <= 2 and a > 34.2 AU

UNK (unknown) : all others

Note that this is not the same as a MOPS efficiency classification --
it is a scientific classification.

=cut

    my ($orbit) = @_;
    my $a;          # semi-major axis
    my ($q, $e, $i) = ($orbit->{q}, $orbit->{e}, $orbit->{i});

    die "bogus orbit: q < 0 ($q)" if ($q < 0);

    if ($e > 0 and $e < 1) {
        my $aJ = 5.20;  # Jupiter's semi-major axis, AU
        $a = $q / (1 - $e);

        return 'PHO' if ( (defined($orbit->{moid_1}) and $orbit->{moid_1} < 0.05) 
            or (defined($orbit->{moid_2}) and $orbit->{moid_2} < 0.05) );

        return 'NEO' if ($q < 1.30);
        return 'MC' if (1.30 < $q and $q <= 1.67);
        return 'MB' if (1.67 < $a and $a <= 5.00);
        return 'TRO' if (5.00 < $a and $a <= 5.5);
        return 'CEN' if (5.00 < $a and $a <= 35.0);
        return 'TNO' if (35.0 < $a and $a <= 50.0);
        return 'SDO' if (50.0 < $a);

        # Calculate cometary parameters.  $TJ is Tisserand paramater; see
        # http://www.ifa.hawaii.edu/~jewitt/tisserand.html
        my $TJ = $aJ / $a + 2 * sqrt((1 - $e * $e) * $a / $aJ) * cos($i / $DEG_PER_RAD);

        return 'SPC' if (2.0 < $TJ and $TJ < 3.0);
        return 'HFC' if ($TJ < 2.0 and $a <= 34.2);
        return 'LPC' if ($TJ < 2.0 and $a > 34.2);
    }
    elsif ($e > 1) {
        return 'HYP';
    }
    return 'UNK';
}


# it's a good point, and it partly depends on what accuracy you need.
# The relationship between a flux and a magnitude is:
#
# mag = -2.5*log10(flux) + Co
#
# where Co is the zero point relationship between the magnitude system
# and flux system.  The flux that we measure directly off of an image is
# in counts / second, so we need to determine an appropriate zero point
# to bring the magnitudes into our calibrated system.  In theory, every
# image will have a unique, and uniquely determined, zero point which we
# would supply with the metadata for that image.  However, in practice,
# that zero point might not very well measured, or measured at all,
# until after a calibration analysis.  Thus, we will initially have to
# send a nominal zero point measured in general for the instrument, but
# which may be somewhat in error for given specific image.  (For
# example, there may be some haze or clouds or other atmospheric effect
# attenuating the signal; alternatively gain changes could increase the
# apparent signal).
our %C0_table = (
    g => 24.9,
    r => 25.1,
    i => 25.0,
    z => 24.6,
    y => 23.6,
);
our $ln10 = log(10);


sub mopslib_hsn2flux {
    # Convert a mag and S/N to flux and flux error under the PS1 regime.
    # This routine normally will be used only to synthesize flux and flux error
    # values when creating synthetic false detections.
    my ($mag, $s2n, $filt) = @_;
    croak "unknown filter: $filt" unless exists($C0_table{$filt});
    croak "zero S/N" unless $s2n > 0;
    my $flux = 10 ** (($mag - $C0_table{$filt}) / -2.5);
    my $flux_sigma = $flux / $s2n;

    return ($flux, $flux_sigma);
}


sub mopslib_flux2hsn {
    # Convert flux and flux error to mag, mag_sigma and S/N.
    my ($flux, $flux_sigma, $filt) = @_;
    croak "unknown filter: $filt" unless exists($C0_table{$filt});
    croak "flux is zero" unless $flux > 0;
    croak "flux_sigma is zero" unless $flux_sigma > 0;
    my $mag = -2.5 * log($flux) / $ln10 + $C0_table{$filt};
    my $mag_sigma = abs(-2.5 * $flux_sigma / ($flux * $ln10));
    my $s2n = $flux / $flux_sigma;
    return ($mag, $mag_sigma, $s2n);
}


sub mopslib_mag2fluxsn {
    # Convert mag and mag error to flux, flux error and S/N.
    my ($mag, $mag_sigma, $filt) = @_;
    croak "unknown filter: $filt" unless exists($C0_table{$filt});
    croak "mag_sigma is zero" unless $mag_sigma > 0;
    my $flux = exp($ln10 * ($mag - $C0_table{$filt}) / -2.5);
    my $flux_sigma = abs($mag_sigma * $flux * $ln10 / -2.5);
    my $s2n = $flux / $flux_sigma;

    return ($flux, $flux_sigma, $s2n);
}


sub mopslib_parseOrbfitOptsFile {

=item mopslib_parseOrbfitOptsFile

Extract a section from an OrbFit options file in which sections are
demarcated using '!!'.  For a given named section, scan the file until
the section is reached, then emit all lines until the start of the
next section.  Optionally perform substitutions of $VARIABLES using a
supplied hash.

=cut

    my ($filename, $section, $argsref) = @_;
    my $fh = new FileHandle $filename;
    my @lines;
    while (<$fh>) {
        next unless /^!!$section/.../^!!/;          # toss everything unless between !!$section and next !!
        push @lines, $_ unless /^!!/;
    }
    $fh->close();
    
    my $stuff = join('', @lines);
    if ($argsref) {
        foreach my $key (keys %{$argsref}) {        # for each listed var
            $stuff =~ s/\$$key/${$argsref}{$key}/mg; # global sub in string
        }
    }

    return $stuff;
}


sub mopslib_validOrbit {
=item mopslib_validOrbit

Perform a sanity check on the orbit, return a true value if the
orbit passes muster.  We use this routine to reject orbits that are
non-physical or lie outside what can be handled in the MOPS phase space
(usually very-high eccentricity orbits).

=cut
    my ($orbit) = @_;
    my ($q, $e, $i, $node, $argPeri) = @{$orbit}{qw(q e i node argPeri)};
    if ( $q <= 0
        or $e < 0
        or $i > 180
        or $i < 0
        or $node < 0 or $node > 360
        or $argPeri < 0 or $argPeri > 360
        or ($e > 10 and $e / $q > 1e7)      # nonphysical hyperbolic
    ) {
        return 0;
    }

    return 1;
}


sub mopslib_formatTimingMsg {
=item mopslib_formatTimingMsg

Format the specified input values (subsystem, subsubsystem, time, comment) for
insertion into a log file.  We provide a library routine to do this so that
we have consistency across subsystems in reporting timing values.

=cut
    my %args = validate(@_, {
        subsystem => 1,
        subsubsystem => 0,
        nn => 1,
        time_sec => 1,
        comment => 0,
    });

    my $msg = 'TIMING ' . $args{subsystem};
    $msg .= "/$args{subsubsystem}" if $args{subsubsystem};
    $msg .= sprintf " %.3f %d", $args{time_sec}, $args{nn};
    $msg .= " # $args{comment}" if $args{comment};

    return $msg;
}


sub mopslib_computeXYIdx {
=item mopslib_computeXYIdx

Given a field and detection, compute an integer index describing the X/Y
numbered position of a rectangle on the field's tangent-plane projection
where the detection can be found.  The rectangle will always be oriented
toward the north pole.  If the field center is exactly on the north pole,
orient the field so that "up" is along the 12h RA line, and along the
0h line for the south pole.

=cut
    my ($field, $det, $fov_deg) = @_;
    $fov_deg ||= $field->{FOV_deg};
    my $pa_deg = $field->{pa_deg} || 0;                     # Default to 0 if not specified.
    my $fov_rad = ($fov_deg / $DEG_PER_RAD);                # FOV size in radians
    my $half_fov_rad = ($fov_deg / 2 / $DEG_PER_RAD);       # half FOV size in radians
    my $size = $field->{xyidxSize};

    # Compute the SLA tangent-plane projection at the field center to
    # the detection's location.
    my ($xi_rad, $eta_rad, $j);
    slaDs2tp(
        $det->{ra} / $DEG_PER_RAD, $det->{dec} / $DEG_PER_RAD,
        $field->{ra} / $DEG_PER_RAD, $field->{dec} / $DEG_PER_RAD, 
        $xi_rad, $eta_rad, $j
    );

    if ($j != 0) {
        # Error in slaDs2tp()
        return int($size * $size / 2);      # center bin, on field center
    }

    # slaDs2tp returns co-ordinates on the tangent plane which is always 
    # oriented the same way regardless of camera rotation. Camera co-ordinates
    # and tangent plane co-ordintaes are the same only when the orientation of
    # the camera is the same as the orientation of the tangent plane. To account
    # for the times when the orientations are not the same we must convert from
    # tangent plane co-ordinates to camera co-ordinates before computing the X/Y
    # index. This is done by applying a rotation to the tangent plane 
    # co-ordinates which will take into account any camera rotation that 
    # occurred when taking the image. This rotation is opposite of the direction 
    # of the camera rotation as all rotations are described relative to a 
    # particular frame of reference. In general for any orthogonal transformation 
    # on a body in a coordinate system there is an inverse transformation which 
    # if applied to the frame of reference results in the body being at the same 
    # coordinates. For example in two dimensions rotating a point clockwise about 
    # a fixed axes (in this case the camera axes) is equivalent to rotating the 
    # axes counterclockwise about the same point.
    my $pa_rad = $pa_deg / $DEG_PER_RAD;
    $xi_rad = $xi_rad * cos(-$pa_rad) - $eta_rad * sin(-$pa_rad);
    $eta_rad = $xi_rad * sin(-$pa_rad) + $eta_rad * cos(-$pa_rad);
    
    # Calculate the XY index of the rotated detection.
    my $xb = int(($xi_rad + $half_fov_rad) / $fov_rad * $size);
    my $yb = int(($eta_rad + $half_fov_rad) / $fov_rad * $size);
    if ($xb >= $size) {
        $xb = $size - 1;
    }
    elsif ($xb < 0) {
        $xb = 0;
    }
    if ($yb >= $size) {
        $yb = $size - 1;
    }
    elsif ($yb < 0) {
        $yb = 0;
    }
    return $yb * $size + $xb;
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

sub mopslib_fitsVersion {
=item mopslib_fitsVersion

Retrieve the IPP version of the fits file.

=cut
    ##############################################################
    # parameter checking and variable initialization
    ##############################################################

    my $base_filename = shift;
    unless (-e $base_filename) {
        die "$base_filename is not a valid file."
    }

    # the field metadata is stored in the zeroth extention.  so read 
    # this hdu and extract the stuff from it.  then open the first 
    # extention, where we will find detectiions.
    my $status = 0;                         # cfitsio $status variable.
    my $filename;                           # general-purpose filename with fits hdu attached
    my $value;
    my $comment;

    # Open and read the fits file.
    $filename = $base_filename;
    my $fptr = Astro::FITS::CFITSIO::open_file($filename, 
        Astro::FITS::CFITSIO::READONLY(), $status);
    _check_status($status, "error while reading $filename");

    # Detections, etc.
    $fptr->movrel_hdu(1, undef, $status);
    _check_status($status, "error moving to HDU 1 in $filename");

    # Read the version name.
    $fptr->read_keyword("HIERARCH CMFVERSION", $value, $comment, $status);
    _check_status($status, "error getting HIERARCH CMFVERSION");
    $value =~ s/^'|'$//g;        # Trim leading and trailing singe quote
    $value =~ s/^\s+|\s+$//g;    # Trim leading and trailing spaces
    return $value;
}


sub mopslib_computeSolarElongation {
=item mopslib_computeSolarElongation

Given MJD, RA and dec on the sky and topocentric position, compute 
the solar elongation of the specified position.

=cut
    my ($mjd, $ra_deg, $dec_deg, $long_deg, $lat_deg) = @_;
    if (!defined($long_deg)) { $long_deg = 0; }
    if (!defined($lat_deg)) { $lat_deg = 0; }
    my $SUN = 0;
    my ($alpha_rad, $delta_rad, $diam_rad);
    slaRdplan(
        $mjd, 
        $SUN, 
        $long_deg / $DEG_PER_RAD,
        $lat_deg / $DEG_PER_RAD,
        $alpha_rad, $delta_rad, $diam_rad
    );
#    return (slaDsep($ra_deg / $DEG_PER_RAD, $dec_deg / $DEG_PER_RAD, $alpha_rad, $delta_rad) - $diam_rad / 2) * $DEG_PER_RAD;
    return slaDsep($ra_deg / $DEG_PER_RAD, $dec_deg / $DEG_PER_RAD, $alpha_rad, $delta_rad) * $DEG_PER_RAD;
}


sub mopslib_computeEclipticLatitude {
=item mopslib_computeEclipticLatitude

Given MJD, RA and dec on the sky and topocentric position, compute 
the ecliptic latitude using slaEqecl().

=cut
    my ($epoch_mjd, $ra_deg, $dec_deg) = @_;
    my ($l_rad, $b_rad);
    
    slaEqecl($ra_deg / $DEG_PER_RAD, $dec_deg / $DEG_PER_RAD, $epoch_mjd, $l_rad, $b_rad);
    return $b_rad * $DEG_PER_RAD;
}


sub mopslib_computeGalacticLatitude {
=item mopslib_computeGalacticLatitude

Given MJD, RA and dec on the sky and topocentric position, compute 
the galactic latitude using slaEqgal().

=cut
    my ($ra_deg, $dec_deg) = @_;
    my ($l_rad, $b_rad);
    slaEqgal(
        $ra_deg / $DEG_PER_RAD, $dec_deg / $DEG_PER_RAD,
        $l_rad, $b_rad,
    );
    return $b_rad * $DEG_PER_RAD;
}


sub _sep_rad {
    # Support routine for assembleTTITuples.
    my ($f1, $f2) = @_;
    return slaDsep(
        $f1->{ra} / $DEG_PER_RAD, $f1->{dec} / $DEG_PER_RAD,
        $f2->{ra} / $DEG_PER_RAD, $f2->{dec} / $DEG_PER_RAD,
    );
}


sub mopslib_assembleTTITuples {

=item mopslib_assembleTTITuples

Given a list of fields, group them into TTI stacks and return a data structure
like the following:

$res = {
    TTI_TUPLES => {
        FIELD_ID_A => [ $field_1, $field_2, $field_A ],
        FIELD_ID_B => [ $field_3, $field_4, $field_B ],
    },
    DEEPSTACKS => {
        FIELD_ID_C => [ $field_5, $field_6, $field_C ],
        FIELD_ID_D => [ $field_7, $field_8, $field_D ],
    },
    ORPHANS => [ $field_10, $field_11, ... ],
}

=cut

    my %args = validate(@_,  {
        fields => 1,                # field list ref
        min_fields => 1,            # min number of fields required for tuple
        max_fields => 1,            # max number of fields required for tuple
        min_tti_min => 1,           # min time between successive fields
        max_tti_min => 1,           # max time between succesive fields
        max_time => 0,              # max time for complete tuple
        max_deep_tuple => 0,        # max num exposures in deep stack
        any_filter => 0,            # allow any filter in TTI (default no => require same filter)
        no_deep_stacks => 0,        # disallow deep stacks (> min_fields)
    });
    my @inp_fields = sort { $a->{epoch} <=> $b->{epoch} } @{$args{fields}};      # copy fields, sort by field epoch
    my $min_fields = $args{min_fields};

    my %tuples;
    my %deep_stacks;
    my @orphans;

    my $res = {
        TTI_TUPLES => \%tuples,         # normal TTI pairs/tuples
        DEEP_STACKS => \%deep_stacks,   # deep stacks (MD, LSST deep stack)
        ORPHANS => [],                  # unmatchable fields
    };


    my ($f, $f1, $idx);     # field objects

    my $min_tti_span_mjd = $args{min_tti_min} / $MINUTES_PER_DAY;
    my $max_tti_span_mjd = $args{max_tti_min} / $MINUTES_PER_DAY;
    my $MAX_SEP_RAD = 0.1 / $DEG_PER_RAD;                               # .1 degree max spatial separation of TTI pair
    my $max_deep_tuple = $args{max_deep_tuple} || 8;    

    # These will keep track of the range of times for the current tuple of fields.
    my $current_tuple;
    my $time_since_last_mjd;

    # Main loop here.  Since the fields have been sorted in time, all subsequent tuple matches
    # will extend the range of the tuple.  Only extend when the extension is less than some function
    # of the nominal TTI, for now 1.5 * TTI.
    while (@inp_fields) {
        $f = shift @inp_fields;     # get first in list
        $current_tuple = {
            TYPE => 'ORPHAN',   # a dummy value
            TUPLE => [ $f ],    # start current tuple
        };

        # Loop through all the remaining fields.
        $idx = 0;
        foreach ($idx = 0; $idx <= $#inp_fields; $idx++) {
            $f1 = $inp_fields[$idx];
            $time_since_last_mjd = $f1->{epoch} - $current_tuple->{TUPLE}->[-1]->{epoch};

            # Accept the current field into the tuple if the field center is approximately
            # coincident and within the allowed time interval (1.5 * nominal TTI).
            if (($args{any_filter} || ($f1->{filter} eq $f->{filter})) and _sep_rad($f, $f1) < $MAX_SEP_RAD) {
                if ($time_since_last_mjd > $min_tti_span_mjd and $time_since_last_mjd < $max_tti_span_mjd) {
                    # Normal TTI tuple.
                    $current_tuple->{TYPE} = 'TTI';
                    push @{$current_tuple->{TUPLE}}, $f1;
                    splice @inp_fields, $idx, 1;        # extract item, compressing list
                    $idx--;
                    last if (scalar @{$current_tuple->{TUPLE}} == $args{max_fields});
                }
                elsif ($time_since_last_mjd < $min_tti_span_mjd and !$args{no_deep_stacks}) {
                    # Deep stack (short time sequences).
                    $current_tuple->{TYPE} = 'DEEP';
                    push @{$current_tuple->{TUPLE}}, $f1;
                    splice @inp_fields, $idx, 1;        # extract item, compressing list
                    $idx--;
                    last if (scalar @{$current_tuple->{TUPLE}} == $max_deep_tuple);
                }
            }
        }    # foreach

        # Now figure out how to classify the tuple.  If the length is less than 
        # tuplewise_min_fields, it's an ORPHAN.  If length is equal to tuplewise_min_fields,
        # it's a TTI_TUPLE.  Otherwise (longer), it's a DEEP_STACK.

        if ($current_tuple->{TYPE} eq 'TTI' and scalar @{$current_tuple->{TUPLE}} >= $min_fields) {
            $tuples{$current_tuple->{TUPLE}->[-1]->{fieldId}} = $current_tuple->{TUPLE};      # store ref to list
        }
        elsif ($current_tuple->{TYPE} eq 'DEEP' and scalar @{$current_tuple->{TUPLE}} >= $min_fields) {
            $deep_stacks{$current_tuple->{TUPLE}->[-1]->{fieldId}} = $current_tuple->{TUPLE};      # store ref to list
        }
        else {
            # Not enough fields, so orphan them.
            push @orphans, @{$current_tuple->{TUPLE}};
        }

    }   # while

    $res->{TTI_TUPLES} = \%tuples;
    $res->{DEEP_STACKS} = \%deep_stacks;
    $res->{ORPHANS} = \@orphans;

    return $res;
}


sub DDtoHMS {
=item DDtoHMS

Convert an RA in Decimal degrees to Hour:Minute:Seconds.SSS format.  This
function is used to meet ICD requirements, so its output format must
not be changed.

=cut
	my ($dd) = @_;

	if ($dd < 0.0 || $dd > 360.0) {
		die "dd must be in [0.0, 360.0]";
	}

	# convert decimal degrees into decimal hours.
	my $hour = 24.0 * $dd / 360.0;

	# now split the hour into an integer hour + decimal minutes.
	my $min = ($hour - int($hour)) * 60.0;
	$hour = int($hour);

	# Now split the minutes into an integer minutes + decimal seconds.
	my $sec = ($min - int($min)) * 60.0;
	$min = int($min);

	return sprintf("%02d:%02d:%06.3f", $hour, $min, $sec);
}

sub DDtoDMS {
=item DDtoDMS

Convert a Dec in Decimal degrees to Degrees:Minute:Seconds format.  This
function is used to meet ICD requirements, so its output format must
not be changed.

=cut
	my ($dd) = @_;

	if ($dd < -90.0 || $dd > 90.0) {
		die "DD must be in [-90.0, 90.0]";
	}

	# Split off sign.
	my $sign;
	my $degrees;

	if ($dd < 0.0) {
		$degrees = -$dd;
		$sign = '-';
	} else {
		$degrees = $dd;
		$sign = '+';
	}

	# Now split the degrees into an integer degrees + decimal minutes.
	my $min = ($degrees - int($degrees)) * 60.0;
	$degrees = int($degrees);

	# Now split the minutes into an integer minutes + decimal seconds.
	my $sec = ($min - int($min)) * 60.0;
	$min = int($min);

	return sprintf("%s%02d:%02d:%02.0f", $sign, $degrees, $min, $sec);
}


1;
__END__

=head2 MOPS OC NUMBER

For convenience, MOPS aggregates observations according to an "Observing
Cycle" number.  Observations between full moons generally share the same
observing cycle number.  The value of this number changes at the latest
0h UT before a full moon.

OC = floor((floor(t - 0.5) + 0.5 - tref2) / Tsyn)

OC = for some Julian Date t, the largest integer less than or equal to the interval from reference time tref2 to the  latest 0h UT before t, divided by the synodic period Tsyn.

This definition always changes value at 0h UT.

floor(t - 0.5) + 0.5 == latest 0h UT prior to t, in JD
tref = 2451564.69729, full moon 21 JAN 2000 (from Tholen)
tref2 = 2451564.5, 0h UT prior to tref
Tsyn = 29.53058867

=head2 MOPS NIGHT NUMBER

MOPS night numbers are integer numbers related to MJD that are used to
number each night relative to the local time of MOPS processing.  The
MOPS night number is the integer MJD at the trailing edge (12:01 p.m.)
of local noon and remains the same until the following local noon.  The
night number of an arbitrary MJD is 

  MJD - 12h + offset

For example, Mauna Kea offset is -10h, so the night number changes
whenever MJD - 12h - 10h is zero, or 22h GMT.

=head1 SEE ALSO

PS::MOPS::DC modules.

=head1 AUTHOR

Larry Denneau, Jr. <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2005 by Larry Denneau, Jr., Institute for Astronomy, University of Hawaii

=cut
