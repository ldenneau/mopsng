package PS::MOPS::DX;

use 5.008;
use strict;
use warnings;
use Carp;
use Params::Validate;

use base qw(Exporter);
our @EXPORT = qw(
    $MIF_D
    $MIF_O
    $MIF_OC

    $MOBS_D
    $MOBS_O

    $DX_DETECTION
    $DX_ORBIT
    $DX_RESIDUAL
    $DX_COVARIANCE
    $DX_METRICS
    $DX_IDENTIFICATION
    $DX_EPHEMERIDES
    $DX_FRAME
    $DX_FIELD
    $DX_PTR_TRACKLET
    $DX_PTR_IDENTIFICATION

    modx_toMOBS_D
    modx_toMOBS_O

    modx_toDETECTION
    modx_fromDETECTION

    modx_toORBIT
    modx_fromORBIT

    modx_toIODREQUEST
);
our $VERSION = '0.01';

our $MIF_D = 'MIF-D';           # MOPS detection serialization
our $MIF_O = 'MIF-O';           # MOPS short orbit serialization
our $MIF_OC = 'MIF-OC';         # MOPS orbit+covariance

our $MOBS_D = 'MOBS-D';         # Tholen MOPS/KNOBS detection
our $MOBS_O = 'MOBS-O';         # Tholen MOPS/KNOBS orbit

our $MOPS_DEFAULT_SERVICE = 'MOPS';     # ID for service (or whatever it's called) column in Orbit

our $DX_DETECTION = 'D';
our $DX_ORBIT = 'O';
our $DX_RESIDUAL = 'R';
our $DX_COVARIANCE = 'C';
our $DX_METRICS = 'M';
our $DX_IDENTIFICATION = 'I';
our $DX_EPHEMERIDES = 'E';
our $DX_FRAME = 'F';            # frame/field
our $DX_FIELD = 'F';            # frame/field
our $DX_PTR_TRACKLET = 'PT';            # tracklet <=> detection mapping
our $DX_PTR_IDENTIFICATION = 'PI';      # identification <=> tracklet mapping


use PS::MOPS::Constants qw(:all);


use subs qw(
    _hsplice
    _default
    _mapdx2mops
);


# Some constants.
our $HEADER_PREFIX = '!!';      # begins each header line

# DETECTION
our @DETECTION_COLS = qw(
OID TIME OBS_TYPE RA DEC APPMAG FILTER OBSERVATORY RMS_RA RMS_DEC RMS_MAG S2N Secret_name
);
our $header_DETECTION = $HEADER_PREFIX . (join ' ', @DETECTION_COLS);
our $OBS_TY_OPTICAL = 'O';      # letter O, optical observation type


# ORBIT
our @ORBIT_COLS = qw(
OID FORMAT q e i Omega argperi t_p H t_0 INDEX N_PAR MOID COMPCODE
);
our $header_ORBIT = $HEADER_PREFIX . (join ' ', @ORBIT_COLS);
our $DEFAULT_ORBIT_INDEX = 1;
our $DEFAULT_ORBIT_N_PAR = 6;
our $DEFAULT_ORBIT_MOID = -1;


# TRACKLET_POINTER
# These records associate a detection with a tracklet ID.
our @TRACKLET_POINTER_COLS = qw(
    DETECTION_ID TRACKLET_ID
);
our $header_TRACKLET_POINTER = $HEADER_PREFIX . (join ' ', @TRACKLET_POINTER_COLS);


# TRACK_POINTER
# These records associate a tracklet ID with a track ID.
our @TRACK_POINTER_COLS = qw(
    TRACKLET_ID TRACK_ID
);
our $header_TRACK_POINTER = $HEADER_PREFIX . (join ' ', @TRACK_POINTER_COLS);


# IODREQUEST
our @IODREQUEST_COLS = qw(
ID_OID NID TRACKLET_OIDs OP_CODE N_OBS N_SOLUTIONS N_NIGHTS ARC_TYPE NO_RADAR PARAM(4)
);
our $header_IODREQUEST = $HEADER_PREFIX . (join ' ', @IODREQUEST_COLS);
our $OP_REQUEST_PRELIM = 'REQUEST_PRELIM';
our $OP_REQUEST_ORBIT = 'REQUEST_ORBIT';


sub modx_toDETECTION {
    # Convert single a MOPS Detection/Detracklet object to a DX DETECTION string.
    # Note that we currently do not support OBS_TY other than 0 for optical
    # observations.
    my ($href) = @_;
    if (!defined($href->{objectName})) {
        $href->{objectName} = 'NS';      # default if undef
    }
#    elsif ($href->{objectName} eq $MOPS_NONSYNTHETIC_OBJECT_NAME) {
#        $href->{objectName} = '';       # empty if NS
#    }

    # Convert sigmas to arcsec.
    my ($ra_sigma_arcsec) = defined($href->{raSigma}) ? sprintf "%.5f", $href->{raSigma} * 3600 : undef;
    my ($dec_sigma_arcsec) = defined($href->{decSigma}) ? sprintf "%.5f", $href->{decSigma} * 3600 : undef;
    my ($mag_sigma) = defined($href->{magSigma}) ? sprintf "%.5f", $href->{magSigma} : undef;
    my ($s2n) = defined($href->{s2n}) ? sprintf "%.5f", $href->{s2n} : undef;

    return join " ", (_default(
        @{$href}{qw(
            detId 
            epoch
        )},
        $OBS_TY_OPTICAL,
        @{$href}{qw(
            ra
            dec
            mag
            filter
            obscode
        )},

        $ra_sigma_arcsec,
        $dec_sigma_arcsec,
        $mag_sigma,
        $s2n,

        @{$href}{qw(
            objectName
        )},
    ));
}


my $_modx_toIODREQUEST_validate_args = {
        ID_OID => 1,                            # require track ID
        TRACKLETS_REF => 0,                     # require list of tracklets
        TRACKLET_IDS => 0,                      # optional list of tracklet IDs (better than TRACKLETS_REF)
        N_OBS => { default => 0 },              # number of obs
        N_NIGHTS => { default => 0 },           # number of nights
        N_SOLUTIONS => { default => 0 },        # number of solutions
        ARC_TYPE => { default => 0 },           # arc type (Milani et. al)
};

sub modx_toIODREQUEST {
    my %args = validate(@_, $_modx_toIODREQUEST_validate_args);
    my @tracklet_ids;

    if (defined($args{TRACKLETS_REF})) {
        @tracklet_ids = map { $_->trackletId } @{$args{TRACKLETS_REF}};
    }
    elsif (defined($args{TRACKLET_IDS})) {
        @tracklet_ids = @{$args{TRACKLET_IDS}};
    }
    else {
        die "neither TRACKLETS_REF nor TRACKLET_IDS specified";
    }

    my $nid = @tracklet_ids;   # number of tracklets
    my $op_code = $OP_REQUEST_PRELIM;
#    my $op_code = $OP_REQUEST_ORBIT;
    return join ' ',
        $args{ID_OID},
        $nid,
        @tracklet_ids,
        $op_code,
        @args{qw(N_OBS N_SOLUTIONS N_NIGHTS ARC_TYPE)},
        0,                      # number radar obs
        '0.0', '0.0', '0.0', '0.0',     # PARAM(4)  (radar stuff, unsupported currently)
    ;
}


sub modx_toMOBS_D {
    # Convert single a MOPS Detection/Detracklet object to a Tholen
    # MOPS/KNOBS detection.
    my ($href) = @_;
    if (!$href->{objectName}) {
        $href->{objectName} = '';       # empty string if undef
    }
    elsif ($href->{objectName} eq $MOPS_NONSYNTHETIC_OBJECT_NAME) {
        $href->{objectName} = '';       # empty if NS
    }

    # Convert sigmas to arcsec.
    my ($ra_sigma_arcsec) = $href->{raSigma} * 3600;
    my ($dec_sigma_arcsec) = $href->{decSigma} * 3600;

    return sprintf 
        #ID    EPOCH  RA     DEC    MAG
        "%-12s %16.8f %16.8f %16.8f %10.4f %16.8e %16.8e %10.4f %16.8f %3s", 
        @{$href}{qw(
            detId 
            epoch
            ra
            dec
            mag
        )},
        $ra_sigma_arcsec,
        $dec_sigma_arcsec,
        @{$href}{qw(
            magSigma
            s2n
            obscode
        )};
}

sub modx_fromDETECTION {
    # Convert a DX DETECTION string into a hashref that looks like a MOPS Detection
    # object.  In practice it will probably be necessary to convert this hashref
    # into an object via Detection->new().

    # The only allowed empty item is SECRET; if it's empty, store it as undef.
    # MOPS currently does not use OBS_TY, so ignore it.  Later we will have to 
    # respect it to handle radar observations.
    my ($str) = @_;
    my $href;
    my @stuff = split /\s+/, $str, scalar(@DETECTION_COLS); # scalar(@DETECTION_COLS) == number of items expected
    my $len_stuff = @stuff;             # length input list
    my $len_det = @DETECTION_COLS;      # number required DETECTION cols modulo optional SECRET name
    if ($len_stuff != $len_det) {
        croak "strange number of items in line: $len_stuff" if (@stuff != @DETECTION_COLS - 1);
        push @stuff, $MOPS_NONSYNTHETIC_OBJECT_NAME;                # add nonsynth for missing SECRET name
    }
    else {
        $stuff[-1] = $MOPS_NONSYNTHETIC_OBJECT_NAME if !$stuff[-1];     # change to std name if empty/undef
    }
    return _mapdx2mops(
        _hsplice(\@DETECTION_COLS, \@stuff),
        {
            OID => 'detId',
            TIME => 'epoch',
            # OBS_TYPE  # discard obs type
            ALPHA => 'ra',
            DELTA => 'dec',
            APPMAG => 'mag',
            FILTER => 'filter',
            IDSTA => 'obscode',
            RMS_ALPHA => 'raSigma',
            RMS_DELTA => 'decSigma',
            S2N => 's2n',
            SECRET_NAME => 'objectName',
        },
    );
}


sub modx_toORBIT {
    # Convert single a MOPS Orbit to a DX COMetary ORBIT.
    my ($href) = @_;

    return join " ", 
        $href->{objectName}, 'COM',
        @{$href}{qw(
            q e i node argPeri timePeri hV epoch
        )},
        $DEFAULT_ORBIT_INDEX,
        $DEFAULT_ORBIT_N_PAR,
        ($href->{moid_1} || $DEFAULT_ORBIT_MOID),
        $MOPS_DEFAULT_SERVICE;
}



sub modx_fromORBIT {
    # Convert a DX ORBIT string into a hashref that looks like a MOPS Orbit
    # object.  In practice it will probably be necessary to convert this hashref
    # into an object via Orbit->new().
    my ($str) = @_;
    my $href;
    my @stuff = split /\s+/, $str, scalar(@ORBIT_COLS); # scalar(@DETECTION_COLS) == number of items expected
    my $len_stuff = @stuff;             # length input list
    my $len_orb = @ORBIT_COLS;          # number required DETECTION cols modulo optional SECRET name
    croak "strange number of items in line: $len_stuff" if ($len_stuff != $len_orb);
    return _hsplice(\@ORBIT_COLS, \@stuff);
}


sub modx_toTRACKLET_POINTER {
    # Export a single MOPS tracklet object as a DES TRACKLET_POINTER.  Does not include the final "\n",
    # in accordance with modx_toXXX() subs.
    my ($tref) = @_;

    my @det_strs;
    my $tracklet_id = $tref->trackletId;
    foreach my $det ($tref->detections) {
        push @det_strs, join(' ', $det->detectionID, $tracklet_id);
    }

    return join("\n", @det_strs);
}


sub modx_toTRACK_POINTER {
    # Export a single MOPS tracklet object as a DES TRACKLET_POINTER.  Does not include the final "\n",
    # in accordance with modx_toXXX() subs.
    # Note that tracks are not first-class citizens in the MOPS Perl hierarchy yet -- they're really
    # just a track ID and a list of tracklet IDs.
    my ($track_id, @tracklet_ids) = @_;

    my @trk_strs;
    foreach my $tid (@tracklet_ids) {
        push @trk_strs, join(' ', $tid, $track_id);
    }

    return join("\n", @trk_strs);
}


sub _hsplice {  
    # Given a list of column names (keys), and another list of values,
    # convert them to a hashref of key => value pairs.
    my ($keys, $vals) = @_;
    croak "spliced lists do not have matching length" if @{$keys} != @{$vals};
    my $href = {};
    @{$href}{@{$keys}} = @{$vals};
    return $href;
}


sub _default {
    # Provide default values for items in the specified list.
    foreach (@_) {
        $_ = '' unless defined $_;
    }
    return @_;
}


sub _mapdx2mops {
    # Given a DX DETECTION hashref created from a string, 
    # map its hash values to something that behaves like a
    # MOPS detection object.
    my ($dx_href, $kvpairs_href) = @_;
    my $mops_href = {};
    my ($k, $v);
    while (($k, $v) = each %$kvpairs_href) {
        $mops_href->{$v} = $dx_href->{$k};
    }
    return $mops_href;
}

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

PS::MOPS::DX - Perl extension for to read and write MOPS objects
according to Andrea Milani's proposed Data Exchange Stanrard.

=head1 SYNOPSIS

  use PS::MOPS::DX;
  modx_something()

=head1 DESCRIPTION

This module implements subroutines to read and write files, and perform
Perl-level data conversions, of MOPS objects to Andrea Milani's Data
Exchange Standard.

Further documentation for the Standard is provided in the
DATA_EXCHANGE_STANDARD document included with this module.

=head2 EXPORT

Not sure yet.

=head1 SEE ALSO

The DATA_EXCHANGE_STANDARD document.

=head1 AUTHOR

Larry Denneau, denneau@ifa.hawaii.edu

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2006 by Larry Denneau, Institute for Astronomy, University of Hawaii.

=cut
