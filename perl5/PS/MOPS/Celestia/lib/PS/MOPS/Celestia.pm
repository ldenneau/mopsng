package PS::MOPS::Celestia;

use 5.008;
use strict;
use warnings;

require Exporter;
our @ISA = qw(Exporter);
our %EXPORT_TAGS = ( 'all' => [ qw(
    pmcel_convertOrbits
    pmcel_dumpOrbits
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );
our $VERSION = '0.01';


use FileHandle;
use Params::Validate;
use File::Slurp;
use PS::MOPS::DC::SSM;


# Convert a file or orbital elements to Celestia format.  Write
#results to STDOUT.
sub pmcel_convertOrbits {
    my %params = validate(@_, {
        filename => 0,          # inputs are files
        elems => 0,             # input is list of elems
    });
    my @output;                 # output list of entries

    # Read entire input file.
    my @stuff;
    if ($params{filename}) {
        @stuff = read_file($params{filename}) or die "can't read $params{filename}";
        chomp foreach @stuff;
    }
    else {
        push @stuff, join(' ', @{$params{elems}});
    }

    # Process lines.
    my ($q_au, $e, $i_deg, $node_deg, $arg_peri_deg, $time_peri_mjd, $h_v, $epoch_mjd);
    my $class;
    my $id;
    my $line;
    my $radius_km;
    my $period_y;

    foreach $line (@stuff) {
        ($q_au, $e, $i_deg, $node_deg, $arg_peri_deg, $time_peri_mjd, $h_v, $epoch_mjd, $id) = split /\s+/, $line;
        warn "bogus line $line" unless defined($epoch_mjd);
        $epoch_mjd += 2400000.5;    # convert MJD to JD
        $time_peri_mjd += 2400000.5;    # convert MJD to JD

        # Calculate radius in km.  Note: eqn returns diameter in km, so / 2 for radius.  Also
        # 0.316 = sqrt(.1).  Also convert to m.
        $radius_km = (1329 * 10 ** (-0.25 * $h_v)) / .316 / 2 * 1000;
        $period_y = sqrt($q_au ** 3);

        if ($id =~ /^Sc/) {
            $class = 'comet';
        }
        else {
            $class = 'asteroid';
        }

        push @output, <<"TEMPLATE";
# $id
# "name of object"  "name of primary"
  "$id"       "Sol"
{
        Class   "$class"
        Texture "asteroid.jpg"
        Mesh    "asteroid.cms"
        Radius         $radius_km
        RotationPeriod 11.34  # arbitrary

        EllipticalOrbit
         {
                 Period                $period_y
                 PericenterDistance    $q_au
                 Eccentricity          $e
                 Inclination           $i_deg
                 AscendingNode         $node_deg
                 ArgOfPericenter       $arg_peri_deg
                 MeanAnomaly           0.0       # position at T
                 Epoch                 $time_peri_mjd
         }

}
TEMPLATE
    }

    return join('', @output);
}


sub pmcel_dumpOrbits {
    my %params = validate(@_, {
        density => 1
    });
}


1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

PS::MOPS::Celestia - Perl extension for blah blah blah

=head1 SYNOPSIS

  use PS::MOPS::Celestia qw(:all);

  # Dump objects from SSM in Celestia format.
  pmcel_dumpOrbits(density => 100);

  # Convert file of orbital elements to Celestia format.
  pmcel_convertOrbits(density => 100);


=head1 DESCRIPTION

PS::MOPS::Celestia translates PSMOPS objects into Celestia .ssc
format, for making pretty pictures and animations with Celestia.

=head1 SEE ALSO

Celestia, http://www.shatters.net/celestia.

=head1 AUTHOR

Larry Denneau, Jr., denneau@ifa.hawaii.edu.

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2005 by Larry Denneau, Jr., Institute for Astronomy,
University of Hawaii.

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.7 or,
at your option, any later version of Perl 5 you may have available.


=cut
