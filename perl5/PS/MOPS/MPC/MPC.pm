package PS::MOPS::MPC;

=head1 NAME

PS::MOPS::MPC - Perl module for reading and writing Minor Planet Center (MPC) files

=head1 SYNOPSIS

  use PS::MOPS::MPC ':all';
  $stuff = mpc_slurp("observations.mpc");
  $stuff = mpc_read("observations.mpc");    # same as mpc_slurp()
  mpc_write($stuff, "output.mpc");

  $str = mcp_format(\%vals);
  print FILE $str, "\n";

  $num = mpc_count_designations($stuff);

=head1 ABSTRACT

  This module reads and writes Minor Planet Center (MPC) files, using
  the 80-column format for minor planets described in

    http://cfa-www.harvard.edu/iau/info/OpticalObs.html

  Observations are read into a hash, keyed by the Provisional/Temporay
  designation specified for an observation.

=head1 DESCRIPTION

  This module reads and writes Minor Planet Center (MPC) files, using
  the 80-column format for minor planets described in

    http://cfa-www.harvard.edu/iau/info/OpticalObs.html

  Observations are read into a hash, keyed by the Provisional/Temporay
  designation specified for an observation.

  When writing files, keys are sorted in alphanumeric order and one
  line is written per observation.  If an error occurs while reading or
  writing the file, die() is called.

  Fields are:

=over 4

=item

NUMBER : Minor Planet Number (5 digits)

=item

DESIGNATION : Temporary or provisional designation (7 digits)

=item

DISCOVERY : "Discovery" asterisk (one digit)

=item

CODE : Standard code descriptor (one digit)

=item

OBSCODE : Describes how observation was made, normally "C" for CCD

=item

DATE : Date of observation.  Typically in the format "2003 08 25.02083".

=item

RA : Observed RA

=item

DEC : Observed declination

=item

MAG : Observed magnitude

=item

BAND : Observed band

=item

OBSERVATORY : Observatory code

=back 4

=head2 EXPORT

None by default.

=head1 SEE ALSO

Official documentation of the MPC format can be found here:

  http://cfa-www.harvard.edu/iau/info/OpticalObs.html

=head1 AUTHOR

Larry Denneau, E<lt>denneau@ifa.hawaii.eduE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright 2004 by Institute for Astronomy, University of Hawaii

=cut


use 5.008;
use strict;
use warnings;

use base qw(Exporter);

our %EXPORT_TAGS = ( 'all' => [ qw(
    mpc_slurp
    mpc_read
    mpc_write
    mpc_format
    mpc_format_miti
    mpc_format_det
    mpc_format_obsdet
    mpc_format_tracklet
    mpc_count_designations
    mpc_split
    mpc_to_miti
    mpcorb_to_obj
    mpcorb_to_des
    packedepoch_to_mjd
    det2mpc
    _deg2hms
    _deg2dms
    _hms2deg
    _dms2deg
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );
our $VERSION = '0.01';

use Astro::Time;
use Astro::SLA;


# Globals
#our $MPC_FORMAT_SPEC = "%5.5s%-7.7s%1.1s%1.1s%1.1s%-17.17s%-12.12s%-12.12s         %-5.2f%1.1s      %-3.3s";
# New slightly reduced-precision spec, so as not to appear presumptious.
#     T37501   C2010 09 16.48549223 05 34.846-08 05 12.50         21.23w      F51
our $MPC_FORMAT_SPEC = "%5.5s%-7.7s%1.1s%1.1s%1.1s%-17.17s%-12.12s%-12.12s         %s%1.1s      %-3.3s";

our $MPC_DATE_FORMAT = "%4d %02d %08.5f";

# Downgraded due to IPP time reporting errors.
#our $MPC_PS1_DATE_FORMAT = "%4d %02d %09.6f";       # higher precision for PS1
our $MPC_PS1_DATE_FORMAT = "%4d %02d %08.5f";

# Pan-STARRS.
#our $MPC_RA_PRECISION = 3;
#our $MPC_DEC_PRECISION = 2;
#our $MPC_MAG_PRECISION = 1;
# ATLAS.
#our $MPC_RA_PRECISION = 2;
#our $MPC_DEC_PRECISION = 1;
#our $MPC_MAG_PRECISION = 1;
my %PRECISION = (
  F51 => {
    RA => 3, DEC => 2, MAG => 1,
  },
  F52 => {
    RA => 3, DEC => 2, MAG => 1,
  },
  DEFAULT => {
    RA => 2, DEC => 1, MAG => 1,
  },
);
sub get_ra_precision {
    my $code = $_[0];
    if (exists($PRECISION{$code})) {
        return $PRECISION{$code}->{RA};
    }
    else {
        return $PRECISION{DEFAULT}->{RA};
    }
}
sub get_dec_precision {
    my $code = $_[0];
    if (exists($PRECISION{$code})) {
        return $PRECISION{$code}->{DEC};
    }
    else {
        return $PRECISION{DEFAULT}->{DEC};
    }
}


# Some conversion routines.  Probably put in a module later.

# Use Astro::Time for this!
sub _deg2hms {
    # Convert pure degrees to HMS (RA hour/minutes/second) representation.
    my ($deg) = @_;
    my ($h, $m, $s);
    $deg += 360 if $deg < 0;        # if < 0, put in 0-360
    $h = int($deg / 15);    # cvt to hours
    $deg -= $h * 15;
    $m = int($deg * 4);
    $deg -= $m / 4;
    $s = $deg * 240;
    return sprintf "%d %02d %06.3f", $h, $m, $s;
}


sub _deg2dms {
    # Convert pure degrees to DMS (declination degrees/arcminutes/arcseconds) representation.
    my ($deg) = @_;
    my ($d, $arcmin, $arcsec);
    $d = int($deg);
    $deg -= $d;
    $arcmin = int($deg * 60);
    $deg -= $arcmin / 60;
    $arcsec = $deg * 3600;
    return sprintf "%d %02d %06.3f", $d, $arcmin, $arcsec;
}


sub _hms2deg {
    # Convert HMS string to pure degrees.
    my ($str) = @_;
    my ($h, $m, $s) = split /\s+/, $str, 3;
    return $h * 15 + $m / 4 + $s / 240;
}


sub _dms2deg {
    # Convert DMS string to pure degrees.
    my ($str) = @_;
    my ($d, $arcmin, $arcsec) = split /\s+/, $str, 3;
    if ($d < 0) {
        return $d - $arcmin / 60 - $arcsec / 3600;
    }
    else {
        return $d + $arcmin / 60 + $arcsec / 3600;
    }
}


sub mpc_split {
    # Split a line into its separate MPC components.  Return a hashref.
    my ($line) = @_;
    my $foo = {
        DISCOVERY => substr($line, 12, 1),
        CODE => substr($line, 13, 1),
        OBSCODE => substr($line, 14, 1),
        DATE => substr($line, 15, 17),
        RA => substr($line, 32, 12),
        DEC => substr($line, 44, 12),
        MAG => substr($line, 65, 5),
        BAND => substr($line, 70, 1),
        OBSERVATORY => substr($line, 77, 3),
    };


    # Special comet handling.
    if ($line =~ m|^(\d\d\d\dP)|) {
        $foo->{NUMBER} = $1;            # numbered comet
        $foo->{DESIGNATION} = $1;       # numbered comet
    }
    elsif ($line =~ m|^    ([PC]\w\w\w\w\w\w\w)|) {
        $foo->{NUMBER} = '';
        $foo->{DESIGNATION} = $1;    # comet designation
    }
    else {
        $foo->{NUMBER} = substr($line, 0, 5);
        if ($foo->{NUMBER}) {
            $foo->{NUMBER} =~ s/^\s+|\s+$//g;
        }
        $foo->{DESIGNATION} = substr($line, 5, 7);
        if ($foo->{DESIGNATION}) {
            $foo->{DESIGNATION} =~ s/^\s+|\s+$//g;
        }
    }

    return $foo;
}


sub mpc_slurp {
    # Read observations in specified file into list; return reference to list.
    my ($filename) = shift;
    my $line;
    my @stuff;      # full list in input order
    my $item;       # per-line fields

    open FILE, $filename or die "can't open $filename for reading";
    while (defined($line = <FILE>)) {
        chomp $line;
        next if $line =~ /^\s*$/;   # skip blank lines

        # "Split" line into fields; note some fields abut each other; also
        # MPC format defines first column as column 1, not 0.
        $item = mpc_split($line);
        next unless $item;      # couldn't parse

        # trim leading/trailing spaces.
        foreach (keys %$item) {
            $item->{$_} =~ s/^\s+|\s+$//g;
        }

        # Add to list.
        push @stuff, $item;
    }
    close FILE;
    return \@stuff;
}


sub mpc_write {
    # Write out list of observations to specified filename.
    # If filename is undef or '-', write to STDOUT.
    my ($stuff, $filename) = @_;
    my $str;
    my $line;

    if (not $filename or $filename eq '-') {
        foreach $line (@$stuff) {
            $str = mpc_format($line);    # format into single line
            print $str, "\n";
        }
    }
    else {
        open OUTFILE, ">$filename" or die "can't open $filename for writing";
        foreach $line (@$stuff) {
            $str = mpc_format($line);    # format into single line
            print OUTFILE $str, "\n";
        }
        close OUTFILE;
    }
}


sub mpc_format {
    # Format specified single observation table into single MPC-format line.
    # No tests are done to verify the data conforms to MPC format; namely
    # that the fields fit into allocated columns in the format.  If a
    # data item is larger than the columns allowed by the format, the data
    # item is truncated.
    my $obs = shift;
    my $line = sprintf
        $MPC_FORMAT_SPEC,
        $obs->{NUMBER} ? sprintf("%05s", $obs->{NUMBER}) : "     ",
        $obs->{DESIGNATION},
        $obs->{DISCOVERY},
        $obs->{CODE},
        $obs->{OBSCODE},
        $obs->{DATE},
        $obs->{RA},
        $obs->{DEC},
        defined($obs->{MAG}) ? sprintf("%-5.1f", $obs->{MAG}) : '     ',
        $obs->{BAND},
        $obs->{OBSERVATORY};
    return $line;
}


sub mpc_format_miti {
    # Format a MITI hash into MPC.
    my %stuff = @_;
    my ($day, $month, $year, $ut) = mjd2cal($stuff{EPOCH_MJD});   # cvt to cal date
    my $obscode = $stuff{OBSCODE};
    my $date;
    if ($obscode eq 'F51' or $stuff{PS1}) {
        $date = sprintf $MPC_PS1_DATE_FORMAT, $year, $month, $day + $ut;   # extra precision for PS1
    }
    else {
        $date = sprintf $MPC_DATE_FORMAT, $year, $month, $day + $ut;   # format YYYY MM DD.DDDDDD
    }

    my $saveZero = $Astro::Time::StrZero;   # save package global val
    $Astro::Time::StrZero = 2;  # 2 leading digits for stringified RA/DEC

    my $ra = deg2str($stuff{RA_DEG}, 'H', get_ra_precision($obscode), ' ');
    my $dec = ($stuff{DEC_DEG} > 0 ? '+' : '') . deg2str($stuff{DEC_DEG}, 'D', get_dec_precision($obscode), ' ');

    my $line = sprintf
        $MPC_FORMAT_SPEC,
        "     ",                # number
        $stuff{ID},           # designation
        "",                     # discovery
        "",                     # code
        "C",                    # observation code
        $date,
        $ra,
        $dec,
        defined($stuff{MAG}) ? sprintf("%-5.1f", $stuff{MAG}) : '     ',
        "V",                    # MITI handles V only
        $obscode,
        "";

    $Astro::Time::StrZero = $saveZero;  # restore package global
    return $line;
}


sub det2mpc {
    # Format a MOPS detection into MPC format.
    my ($det, $desig) = @_;
    my ($day, $month, $year, $ut) = mjd2cal($det->{epoch});   # cvt to cal date
    my $obscode = $det->{obscode};
    my $date;
    if ($obscode eq 'F51') {
        $date = sprintf $MPC_PS1_DATE_FORMAT, $year, $month, $day + $ut;   # extra precision for PS1
    }
    else {
        $date = sprintf $MPC_DATE_FORMAT, $year, $month, $day + $ut;   # format YYYY MM DD.DDDDDD
    }

    my $saveZero = $Astro::Time::StrZero;   # save package global val
    $Astro::Time::StrZero = 2;  # 2 leading digits for stringified RA/DEC

    my $ra = deg2str($det->{ra}, 'H', get_ra_precision($obscode), ' ');
    my $dec = ($det->{dec} > 0 ? '+' : '') . deg2str($det->{dec}, 'D', get_dec_precision($obscode), ' ');

    my $line = sprintf
        $MPC_FORMAT_SPEC,
        "     ",                # number
        $desig || $det->{objectName},
        "",                     # discovery
        "",                     # code
        "C",                    # observation type (CCD)
        $date,
        $ra,
        $dec,
        defined($det->{mag}) ? sprintf("%-5.1f", $det->{mag}) : '     ',
        $det->{filter},
        $det->{obscode},
        "";

    $Astro::Time::StrZero = $saveZero;  # restore package global
    return $line;
}


sub _datestr2dmyu {
    # Convert the DATE field from an MPC line to an MJD.  The date is
    # in the format 'YYYY MM DD.UUUUUU'.  Return a list containing the
    # values.
    my ($y, $m, $d) = split /\s+/, $_[0];
    return (int($d), $m, $y, $d - int($d));
}


sub _rastr2ra_deg {
    # Convert the RA field (in HH MM SS.SSS) from an MPC line to RA in degrees.
    return str2deg($_[0], 'H');
}


sub _decstr2dec_deg {
    # Convert the RA field (in DD MM SS.SS) from an MPC line to DEC in degrees.
    return str2deg($_[0], 'D');
}



sub mpc_to_miti {
    # Convert a MPC hash into miti, converting the time, RA and DEC
    # representations.  Return a hashref containing the MITI fields.
    my ($line) = @_;
    my $href = mpc_split($line);

    my %stuff = %{$href};
    my ($d, $m, $y, $ut) = _datestr2dmyu($stuff{DATE});

    return {
        ID => ($stuff{DESIGNATION} || $stuff{NUMBER}),
        EPOCH_MJD => cal2mjd($d, $m, $y, $ut),
        RA_DEG => _rastr2ra_deg($stuff{RA}),
        DEC_DEG => _decstr2dec_deg($stuff{DEC}),
        MAG => $stuff{MAG},
        OBSCODE => $stuff{OBSERVATORY},
    };
}


sub mpc_format_det {
    # Format PSMOPS Detection pair into MPC format.
    my ($det, $use_detIds) = @_;
    my $line;
    my $date;

    my $saveZero = $Astro::Time::StrZero;
    $Astro::Time::StrZero = 2;  # 2 leading digits for stringified RA/DEC

    my ($day, $month, $year, $ut) = mjd2cal($det->epoch);
    $date = sprintf $MPC_DATE_FORMAT, $year, $month, $day + $ut;
    $line = sprintf
        $MPC_FORMAT_SPEC,
        "     ",            # formerly NUMBER
        ($use_detIds ? $det->detId : ($det->{objectName} or "XXXXXX")),           # MPC designation/det Id
        " ",                # "discovery" asterisk
        " ",                # NOTE 1 code
        "C",                # NOTE 2 code, usually observation method
        $date,
        deg2str($det->{ra}, 'H', get_ra_precision($det->obscode), ' '),
        ($det->{dec} > 0 ? '+' : '') . deg2str($det->{dec}, 'D', get_dec_precision($det->obscode), ' '),
        defined($det->{mag}) ? sprintf("%-5.1f", $det->{mag}) : '     ',
        'V',    # for now
        $det->obscode;

    $Astro::Time::StrZero = $saveZero;  # restore
    return $line;
}


sub mpc_format_obsdet {
    # Format PSMOPS Field and Detection pair into MPC format.
    # If use_detIds is specified, write out the detection ID; otherwise objectName
    my ($field, $det, $use_detIds) = @_;
    my $line;
    my $date;

    my $saveZero = $Astro::Time::StrZero;
    $Astro::Time::StrZero = 2;  # 2 leading digits for stringified RA/DEC

    my ($day, $month, $year, $ut) = mjd2cal($field->epoch);
    $date = sprintf $MPC_DATE_FORMAT, $year, $month, $day + $ut;
    $line = sprintf
        $MPC_FORMAT_SPEC,
        "     ",            # formerly NUMBER
        ($use_detIds ? $det->detId : ($det->{objectName} or "XXXXXX")),           # MPC designation/det Id
        " ",                # "discovery" asterisk
        " ",                # NOTE 1 code
        "C",                # NOTE 2 code, usually observation method
        $date,
        deg2str($det->{ra}, 'H', get_ra_precision($field->obscode), ' '),
        ($det->{dec} > 0 ? '+' : '') . deg2str($det->{dec}, 'D', get_dec_precision($field->obscode), ' '),
        defined($det->{mag}) ? sprintf("%-5.1f", $det->{mag}) : '     ',
        'V',    # for now
        $field->obscode;

    $Astro::Time::StrZero = $saveZero;  # restore
    return $line;
}


sub mpc_format_tracklet {
    # Return an array of MPC-formatted tracklet strings.
    my ($trk, $cheat) = @_;
    my $date;
    my $obscode = '568';

    my $saveZero = $Astro::Time::StrZero;
    $Astro::Time::StrZero = 2;  # 2 leading digits for stringified RA/DEC

    my @lines;
    foreach my $det (@{$trk->detections}) {
        my ($day, $month, $year, $ut) = mjd2cal($det->epoch);
        my $objectName;
        $objectName = $det->objectName if $cheat;

        $date = sprintf $MPC_DATE_FORMAT, $year, $month, $day + $ut;
        push @lines, (sprintf(
            $MPC_FORMAT_SPEC,
            "     ",            # formerly NUMBER
            $trk->trackletId,           # MPC designation/det Id
            " ",                # "discovery" asterisk
            " ",                # NOTE 1 code
            "C",                # NOTE 2 code, usually observation method
            $date,
            deg2str($det->{ra}, 'H', get_ra_precision($obscode), ' '),
            ($det->{dec} > 0 ? '+' : '') . deg2str($det->{dec}, 'D', get_dec_precision($obscode), ' '),
            defined($det->{mag}) ? sprintf("%-5.1f", $det->{mag}) : '     ',
            'V',    # for now
            $obscode) . 
            ($cheat ? " $objectName" : '')
        );
    }

    $Astro::Time::StrZero = $saveZero;  # restore
    return @lines;
}



sub mpc_count_designations {
    # Routine used mostly for testing and simulation; just counts number
    # of different designations in a list of MPC detections read in by
    # mpc_slurp.
    my ($stuff) = @_;
    my $det;
    my %them;
    my $des;
    foreach $det (@{$stuff}) {
        $des = $det->{DESIGNATION};
        if (defined($des) && $des) { 
            $them{$des} = 1;
        }
    }

    return scalar(keys %them);  # count of keys
}


# MPC packed converters.
my $_i;
our $char2month_str = '123456789ABC';
$_i = 1;
our %char2month = map { $_ => $_i++ } split //, $char2month_str;

$_i = 1;
our $char2day_str = '123456789ABCDEFGHIJKLMNOPQRSTUV';
our %char2day = map { $_ => $_i++ } split //, $char2day_str;

our $METERS_PER_AU = 1.49597870691e11;     # m


=head1 mpcorb_to_des

The only complicated thing is to convert the packet epoch format and convert mean anomaly
to time of perihelion.

MPCORB.DAT is formatted:

[Header fluff]

----------------------------------------------------------------------------------------------------------------------------------------------------------------
00001    3.34  0.12 K107N 113.41048   72.58976   80.39321   10.58682  0.0791382  0.21432817   2.7653485  0 MPO110568  6063  94 1802-2006 0.61 M-v 30h MPCW       0000      (1) Ceres              20061025
00002    4.13  0.11 K107N  96.14829  310.15090  173.12949   34.84091  0.2309995  0.21353950   2.7721533  0 MPO135420  7253  93 1827-2008 0.59 M-v 28h MPCW       0000      (2) Pallas             20080101
00003    5.33  0.32 K107N  32.09611  248.10804  169.91138   12.98211  0.2549812  0.22589931   2.6700913  0 MPO135420  6278  91 1824-2008 0.60 M-v 38h MPCW       0000      (3) Juno               20080308
00004    3.20  0.32 K107N 307.80111  149.83731  103.90970    7.13407  0.0886225  0.27152411   2.3619124  0 MPO135420  6449  82 1827-2007 0.60 M-v 18h MPCW       0000      (4) Vesta              20070808
00005    6.85  0.15 K107N 239.04556  358.35340  141.64423    5.36884  0.1904177  0.23823543   2.5771032  0 MPO 41498  2178 100 1845-2002 0.56 M-v 38h Goffin     0000      (5) Astraea            20021114
00006    5.71  0.24 K107N 331.77097  239.35969  138.74019   14.75228  0.2026188  0.26089412   2.4256410  0 MPO135420  5011  81 1848-2008 0.55 M-v 38h MPCW       0007      (6) Hebe               20080222
00007    5.51  0.15 K107N   4.63161  145.17384  259.66157    5.52346  0.2305487  0.26748589   2.3856247  0 MPO135420  4443  76 1848-2008 0.60 M-v 38h MPCW       0000      (7) Iris               20080308
00008    6.49  0.28 K107N 309.28594  285.24976  110.95525    5.88820  0.1568436  0.30170750   2.2016350  0 MPO 25662  2438 102 1847-2000 0.54 M-v 38h Goffin     0000      (8) Flora              20001115
00009    6.28  0.17 K107N 141.42944    6.26642   68.94869    5.57477  0.1223679  0.26737617   2.3862774  0 MPO 25662  2131  96 1822-2001 0.54 M-v 38h Goffin     0000      (9) Metis              20011024
...

http://www.minorplanetcenter.org/iau/info/MPOrbitFormat.html

=cut

sub mpcorb_to_obj {
    # Given a split line from MPCORB.DAT, convert to DES.  Note that H and/or G can be missing, so the caller
    # must take precautions.
    my @fields = _orbsplit($_[0]);

    my ($desig, $H, $G, $packed_epoch, $M, $argPeri, $node, $incl, $e, $n, $a, $longname) = @fields;
    my $epoch_mjd_TT = packedepoch_to_mjd($packed_epoch);
    my $epoch_mjd_UTC = $epoch_mjd_TT - slaDtt($epoch_mjd_TT) / 86400;

    # Do some conversions.  RJ provides a, not q; and M, not tau.
    my $a_au = $a;   # semi-major axis in AU
    my $a_m = $a_au * $METERS_PER_AU;

    my $q = $a_au * (1 - $e);
    my $tp;
    if ($M > 180) {
        $tp = $epoch_mjd_UTC - ($M - 360) / $n;
    }
    else {
        $tp = $epoch_mjd_UTC - $M / $n;  # tau = epoch - M / n
    }

    return {
        desig => $desig,
        a_AU => $a_au,
        q_AU => $q,
        e => $e,
        i_deg => $incl,
        node_deg => $node,
        argPeri_deg => $argPeri,
        timePeri_mjd_UTC => slaDtt($tp),
        H => $H,
        G => $G,
        epoch_mjd_UTC => $epoch_mjd_UTC,
        epoch_mjd_TT => $epoch_mjd_TT,
        longname => $longname,
    }
}


sub mpcorb_to_des {
    # Given a split line from MPCORB.DAT, convert to DES.
    my $obj = mpcorb_to_obj($_[0]);

    # Now make DES string.
    my $des_str = join(' ', $obj->{desig}, 'COM', $obj->{q_AU}, $obj->{e}, $obj->{i_deg}, $obj->{node_deg}, $obj->{argPeri_deg}, $obj->{timePeri_MJD_UTC}, $obj->{H}, $obj->{epoch_mjd_UTC}, 1, 6, -1, 'MOPS');
}


sub packedepoch_to_mjd {
    my ($packed_epoch) = @_;

    # Convert MPC packed dates of the format K107N to an MJD.  See
    #   http://www.minorplanetcenter.net/iau/info/PackedDates.html
    die "can't suss format of $packed_epoch" unless $packed_epoch =~ /^([IJKL])(\d\d)(\w)(\w)(\d*)$/;
    my $pcen = $1;
    my $pyr = $2;
    my $pmon = $3;
    my $pday = $4;
    my $pfrac = $5 || 0;

    my $year = 1800 + (ord($pcen) - ord('I')) * 100 + $pyr;
    my $mon = $char2month{$pmon} or die "can't convert month $pmon";
    my $day = $char2day{$pday} or die "can't convert day $pday";
    my $frac = $pfrac / (10 ** length($pfrac));

    # Convert calendar date to MJD.
    my $mjd = cal2mjd($day, $mon, $year, $frac);
    return $mjd;
}


sub _orbsplit {
    # Split an MPC orbit line by columns instead of whitespace, since there may be missing values.
    my $line = $_[0];

    my $longname = substr($line, 166, 27);
    $longname =~ s/^\s+|\s+$//g;        # strip whitespace

    my @items = (
        substr($line, 0, 7),        # number/desig
        substr($line, 8, 5),        # H
        substr($line, 14, 5),       # G
        substr($line, 20, 5),       # packed epoch_TT
        substr($line, 26, 9),       # mean anomaly at epoch, deg
        substr($line, 37, 9),       # argPeri, deg
        substr($line, 48, 9),       # node, deg
        substr($line, 59, 9),       # incl, deg
        substr($line, 70, 9),       # e
        substr($line, 80, 10),      # mean motion, deg/day
        substr($line, 92, 11),      # a, AU
        $longname,
    );
    s/^\s+|\s+$//g foreach @items;
    return @items;
}


# Subroutine aliasing.
*mpc_read = \&mpc_slurp;


1;
