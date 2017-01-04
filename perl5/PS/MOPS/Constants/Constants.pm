package PS::MOPS::Constants;

use 5.008;
use strict;
use warnings;

use base qw(Exporter);

our %EXPORT_TAGS = (
    'all' => [ qw(
        $PI
        $TWOPI

        $DEG_PER_RAD
        $SECONDS_PER_DAY
        $MINUTES_PER_DAY
        $MJD2JD_OFFSET
        $METERS_PER_AU
        $GM

        $OC_TREF_0HUT_JD
        $OC_SYNODIC_PERIOD

        $B62_CHARS
        $MOPS_NONSYNTHETIC_OBJECT_NAME

        $DIFF_TYPE_WW
        $DIFF_TYPE_WS
        $DIFF_TYPE_SW
        $DIFF_TYPE_SS
        $DIFF_TYPE_UNSUPPORTED
        
        $IPP_FITS_VER_1
        $IPP_FITS_VER_2
        $IPP_FITS_VER_3
        
        $MOPS_EFF_UNFOUND
        $MOPS_EFF_CLEAN
        $MOPS_EFF_MIXED
        $MOPS_EFF_NONSYNTHETIC
        $MOPS_EFF_BAD

        $MOPS_EFF_ORBIT_FAIL
        $MOPS_EFF_ORBIT_UNLINKED
        $MOPS_EFF_ORBIT_REJECTED
        $MOPS_EFF_ORBIT_RESIDUALS
        $MOPS_EFF_ORBIT_IODFAIL
        $MOPS_EFF_ORBIT_DIFFAIL
        $MOPS_EFF_ORBIT_OK

        $DETECTION_STATUS_UNFOUND
        $DETECTION_STATUS_FOUND
        $DETECTION_STATUS_USED

        $INGEST_STATUS_NEW
        $INGEST_STATUS_INGESTED

        $FIELD_STATUS_NEW
        $FIELD_STATUS_INGESTED
        $FIELD_STATUS_READY
        $FIELD_STATUS_TRACKLETS_DONE
        $FIELD_STATUS_POSTTRACKLET
        $FIELD_STATUS_ATTRIBUTIONS
        $FIELD_STATUS_READYTOLINK
        $FIELD_STATUS_LINK1
        $FIELD_STATUS_LINKDONE
        $FIELD_STATUS_SKIP

        $FIELD_STATUS_SYNTH
        $FIELD_STATUS_TRACKLET
        $FIELD_STATUS_ATTRIB


        $TRACKLET_STATUS_UNATTRIBUTED
        $TRACKLET_STATUS_ATTRIBUTED
        $TRACKLET_STATUS_KILLED
        $TRACKLET_STATUS_KNOWN

        $DERIVEDOBJECT_STATUS_NEW
        $DERIVEDOBJECT_STATUS_MERGED
        $DERIVEDOBJECT_STATUS_KILLED

        $EONQUEUE_STATUS_NEW
        $EONQUEUE_STATUS_IDENTIFIED
        $EONQUEUE_STATUS_PRECOVERED
        $EONQUEUE_STATUS_POSTFIT
        $EONQUEUE_STATUS_REPORTED
        $EONQUEUE_STATUS_RETIRED
        %EONQUEUE_STATUS_NEXT

        %DETECTION_STATUS_DESCS
        %TRACKLET_STATUS_DESCS
        %DERIVEDOBJECT_STATUS_DESCS
        %MOPS_EFF_DESCS
        %MOPS_EFF_ORBIT_DESCS
        %EONQUEUE_STATUS_DESCS
        
        
    ) ]
);
our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );
our $VERSION = '0.01';


# Mathmatical constants.
our $PI = atan2(1, 0) * 2;
our $TWOPI = atan2(1, 0) * 4;
our $DEG_PER_RAD = 180.0 / $PI;

# Date handling.
our $SECONDS_PER_DAY = 86400;
our $MINUTES_PER_DAY = 1440;
our $MJD2JD_OFFSET = 2_400_000.5;  # convert MJD to JD

# Astrodynamic parameters.  See http://ssd.jpl.nasa.gov/astro_constants.html.
our $METERS_PER_AU = 1.49597870691e11;     # m
our $GM = 1.32712440018e20;           # m3/s2

# MOPS Observation Cycle.
our $OC_TREF_0HUT_JD = 2451564.5;         # nearest 0h UT prior to 21 JAN 2000 full moon
our $OC_SYNODIC_PERIOD = 29.53058867;

# MOPS parameters/constants.
our $B62_CHARS = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
our $MOPS_NONSYNTHETIC_OBJECT_NAME = 'NS';      # all nonsynth detections use this object name

# Various efficiency- and processing-related statuses.
our $DETECTION_STATUS_UNFOUND = 'X';
our $DETECTION_STATUS_FOUND = 'F';
our $DETECTION_STATUS_USED = 'U';

our %DETECTION_STATUS_DESCS = (
    $DETECTION_STATUS_UNFOUND => 'UNFOUND',
    $DETECTION_STATUS_FOUND => 'FOUND',
    $DETECTION_STATUS_USED => 'USED',
);

# IPP Difference types.
our $DIFF_TYPE_WW = 1;              # warp - warp diff
our $DIFF_TYPE_WS = 2;              # warp - stack diff
our $DIFF_TYPE_SW = 3;              # stack - warp diff
our $DIFF_TYPE_SS = 4;              # stack - stack diff
our $DIFF_TYPE_UNSUPPORTED = 0;     # Unsupported diff_mode

# Version of IPP fits file.
our $IPP_FITS_VER_1 = 'PS1_DV1';
our $IPP_FITS_VER_2 = 'PS1_DV2';
our $IPP_FITS_VER_3 = 'PS1_DV3';

# Field status chain: I => R => D => T => U => A => K => L, X => SKIP
our $FIELD_STATUS_INGESTED = 'I';                   # detections ingested, awaiting siblings
our $FIELD_STATUS_READY = 'N';                      # ready for processing (all siblings ingested), compatible with previous NEW 'N'
our $FIELD_STATUS_NEW = $FIELD_STATUS_READY;        # legacy synonym
our $FIELD_STATUS_SYNTH = 'D';                      # synth detections done

our $FIELD_STATUS_TRACKLETS_DONE = 'T';             # ingested, tracklets done
our $FIELD_STATUS_TRACKLET = $FIELD_STATUS_TRACKLETS_DONE;  # alias

our $FIELD_STATUS_POSTTRACKLET = 'U';               # post-tracklet accounting done

our $FIELD_STATUS_ATTRIBUTIONS = 'A';               # derived orbits attributed
our $FIELD_STATUS_ATTRIB = $FIELD_STATUS_ATTRIBUTIONS;      # alias

our $FIELD_STATUS_LINK1 = 'K';                      # slow link pass completed
our $FIELD_STATUS_LINKDONE = 'L';                   # all linking completed
our $FIELD_STATUS_READYTOLINK = $FIELD_STATUS_ATTRIB;

our $FIELD_STATUS_SKIP = 'X';                       # don't process this field

our @FIELD_STATUS_CHAIN = (
    $FIELD_STATUS_READY,
    $FIELD_STATUS_SYNTH,
    $FIELD_STATUS_TRACKLET,
    $FIELD_STATUS_POSTTRACKLET,
    $FIELD_STATUS_ATTRIB,
    $FIELD_STATUS_LINK1,
    $FIELD_STATUS_LINKDONE,
);


##our $TRACKLET_STATUS_UNFOUND = 'X';       # deprecated
our $TRACKLET_STATUS_UNATTRIBUTED = 'U';
our $TRACKLET_STATUS_ATTRIBUTED = 'A';
our $TRACKLET_STATUS_KILLED = 'K';          # contains dets in another attribed tracklet, so unusable
our $TRACKLET_STATUS_KNOWN = 'W';           # externally attributed to known object

our %TRACKLET_STATUS_DESCS = (
    $TRACKLET_STATUS_UNATTRIBUTED => 'UNATTRIBUTED',
    $TRACKLET_STATUS_ATTRIBUTED => 'ATTRIBUTED',
    $TRACKLET_STATUS_KILLED => 'KILLED',
    $TRACKLET_STATUS_KNOWN => 'KNOWN',
);


our $DERIVEDOBJECT_STATUS_NEW = 'I';        # normal, new status
our $DERIVEDOBJECT_STATUS_MERGED = 'M';     # DO was merged with another DO
our $DERIVEDOBJECT_STATUS_KILLED = 'K';     # DO has been marked as being invalid

our %DERIVEDOBJECT_STATUS_DESCS = (
    $DERIVEDOBJECT_STATUS_NEW => 'NEW',
    $DERIVEDOBJECT_STATUS_MERGED => 'MERGED',
    $DERIVEDOBJECT_STATUS_KILLED => 'KILLED',
);

# MOPS efficiency values.
our $MOPS_EFF_UNFOUND = 'X';            # unfound object
our $MOPS_EFF_CLEAN = 'C';              # all same synthetic object
our $MOPS_EFF_MIXED = 'M';              # all synthetic, but different objs
our $MOPS_EFF_BAD = 'B';                # synth + nonsynth tracklet
our $MOPS_EFF_NONSYNTHETIC = 'N';       # all nonsynth

our %MOPS_EFF_DESCS = (
    $MOPS_EFF_UNFOUND => 'UNFOUND',
    $MOPS_EFF_CLEAN => 'CLEAN',
    $MOPS_EFF_MIXED => 'MIXED',
    $MOPS_EFF_BAD => 'BAD',
    $MOPS_EFF_NONSYNTHETIC => 'NONSYNTHETIC',
);


our $MOPS_EFF_ORBIT_IODFAIL = 'I';      # orbit failed IOD
our $MOPS_EFF_ORBIT_DIFFAIL = 'D';      # orbit passed IOD but failed diffcor
our $MOPS_EFF_ORBIT_FAIL = 'F';         # other failure
our $MOPS_EFF_ORBIT_UNLINKED = $MOPS_EFF_ORBIT_FAIL;            # synonym, might get its own code later
our $MOPS_EFF_ORBIT_REJECTED = 'J';     # got an orbit, was rejected by tracklet mgt
our $MOPS_EFF_ORBIT_RESIDUALS = 'R';    # residuals analysis failed
our $MOPS_EFF_ORBIT_OK = 'Y';           # orbit passed IOD and diffcor

our %MOPS_EFF_ORBIT_DESCS = (
    $MOPS_EFF_ORBIT_IODFAIL => 'IOD FAILED',
    $MOPS_EFF_ORBIT_DIFFAIL => 'DIFF FAILED',
    $MOPS_EFF_ORBIT_REJECTED => 'TRACKLET MGT FAILED',
    $MOPS_EFF_ORBIT_RESIDUALS => 'RESIDUALS ANALYSIS FAILED',
    $MOPS_EFF_ORBIT_FAIL => 'FAILED',
    $MOPS_EFF_ORBIT_OK => 'OK',
);


# End-of-night (EON) queue processing.
our $EONQUEUE_STATUS_NEW = 'N';
our $EONQUEUE_STATUS_IDENTIFIED = 'I';
our $EONQUEUE_STATUS_PRECOVERED = 'P';
# Change the following when we officially support reporting.
#our $EONQUEUE_STATUS_REPORTED = 'R';
our $EONQUEUE_STATUS_POSTFIT = 'X';
our $EONQUEUE_STATUS_REPORTED = 'X';
our $EONQUEUE_STATUS_RETIRED = 'X';

our %EONQUEUE_STATUS_NEXT = (
    $EONQUEUE_STATUS_NEW => $EONQUEUE_STATUS_IDENTIFIED,
    $EONQUEUE_STATUS_IDENTIFIED => $EONQUEUE_STATUS_PRECOVERED,
    $EONQUEUE_STATUS_PRECOVERED => $EONQUEUE_STATUS_POSTFIT,
    $EONQUEUE_STATUS_POSTFIT => $EONQUEUE_STATUS_REPORTED,
    $EONQUEUE_STATUS_REPORTED => $EONQUEUE_STATUS_RETIRED,
    $EONQUEUE_STATUS_RETIRED => undef,
);

our %EONQUEUE_STATUS_DESCS = (
    $EONQUEUE_STATUS_NEW => 'NEW OBJECT',
    $EONQUEUE_STATUS_IDENTIFIED => 'IDENTIFIED',
    $EONQUEUE_STATUS_PRECOVERED => 'PRECOVERED',
    $EONQUEUE_STATUS_POSTFIT => 'POSTFIT',
    $EONQUEUE_STATUS_REPORTED => 'REPORTED',
    $EONQUEUE_STATUS_RETIRED => 'RETIRED FROM PROCESSING',
);

# Descriptions of efficiency values.

1;
__END__

=head1 NAME

PS::MOPS::Constants - Perl module defining common MOPS contant values

=head1 SYNOPSIS

  use PS::MOPS::Constants;
  my $v1 = tan($PS::MOPS::Constants::PI);
  my $v2 = tan($PS::MOPS::Constants::TWOPI);

  use PS::MOPS::Constants qw(:all);
  my $x1 = tan($PI);
  my $x2 = tan($TWOPI);
=head1 DESCRIPTION

This module defines commonly used constant values, including astrodynamical
constants, for MOPS.

=head1 MATH AND PHYSICAL CONSTANTS

=item $PI = atan2(1, 0) * 2

=item $TWOPI = atan2(1, 0) * 4

=item $DEG_PER_RAD = 180.0 / $PI

=item $SECONDS_PER_DAY = 86400

=item $MINUTES_PER_DAY = 1440

=item $MJD2JD_OFFSET = 2_400_000.5

Convert MJD to JD

=item $METERS_PER_AU = 1.49597870691e11

=item $GM = 1.32712440018e20

m3/s2

=item $OC_TREF_0HUT_JD = 2451564.5

Nearest 0h UT prior to 21 JAN 2000 full moon

=item $OC_SYNODIC_PERIOD = 29.53058867

In days.

=head1 EFFICIENCY CONSTANTS

=item $DETECTION_STATUS_UNFOUND = 'X'

=item $DETECTION_STATUS_FOUND = 'F'

=item $DETECTION_STATUS_USED = 'U'

The detection is part of a tracklet that belongs to a derived object.

=item $TRACKLET_STATUS_UNATTRIBUTED = 'U'

=item $TRACKLET_STATUS_ATTRIBUTED = 'A'

=item $TRACKLET_STATUS_KILLED = 'K'

Indicates the tracklet contains detections that are used by some other
attributed tracklet; thus this tracklet cannot sensibly be used for
any other processing unless it is "unkilled".

=item $TRACKLET_STATUS_KNOWN = 'W'

Indicates the tracklet was marked as KNOWN in tracklet processing and
is to be omitted for linking or further processing.

=item $MOPS_EFF_UNFOUND = 'X'

=item $MOPS_EFF_CLEAN = 'C'

=item $MOPS_EFF_MIXED = 'M'

=item $MOPS_EFF_BAD = 'B'

=item $MOPS_EFF_NONSYNTHETIC = 'N'

=item $MOPS_EFF_ORBIT_IODFAIL = 'I'

=item $MOPS_EFF_ORBIT_DIFFAIL = 'D'

=item $MOPS_EFF_ORBIT_FAIL = 'F'

=item $MOPS_EFF_ORBIT_OK = 'Y'

=head1 FIELD PROCESSING STATUS VALUES

=item $FIELD_STATUS_INGESTED = 'I'                   # detections ingested, awaiting siblings

=item $FIELD_STATUS_READY = 'N'                      # ready for processing (all siblings ingested)

=item $FIELD_STATUS_SYNTH = 'D'                      # synth detections done

=item $FIELD_STATUS_TRACKLET = 'T'

=item $FIELD_STATUS_POSTTRACKLET = 'U'               # post-tracklet accounting done

=item $FIELD_STATUS_ATTRIB = 'A'               # derived orbits attributed

=item $FIELD_STATUS_LINK1 = 'K'                      # slow link pass completed

=item $FIELD_STATUS_LINKDONE = 'L'                   # all linking completed

=head1 IPP exposure difference types

=item $DIFF_TYPE_WW = 1              # warp - warp diff

=item $DIFF_TYPE_WS = 2              # warp - stack diff

=item $DIFF_TYPE_SW = 3              # stack - warp diff

=item $DIFF_TYPE_SS = 4              # stack - stack diff

=item $DIFF_TYPE_UNSUPPORTED = 0     # Unsupported diff_mode

=item $IPP_FITS_VER_1 = 'PS1 DV1'

=item $IPP_FITS_VER_2 = 'PS1 DV2'
        
=item $IPP_FITS_VER_3 = 'PS1 DV3'


=head1 EXPORT

None by default.

=head1 SEE ALSO

JPL's Astrodynamic Constants and Parameters page, 

    http://ssd.jpl.nasa.gov/astro_constants.html

=head1 AUTHOR

Larry Denneau, Jr., <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2005 by Larry Denneau, Jr., Institute for Astronomy, University of Hawaii.

=cut
