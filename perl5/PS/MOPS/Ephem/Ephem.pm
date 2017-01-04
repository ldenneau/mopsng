# $Id: Ephem.pm 892 2006-03-01 20:11:59Z denneau $
package PS::MOPS::Ephem;

use 5.008;
use strict;
use warnings;

use base qw(Exporter);
our @EXPORT = qw(
    mopsephem_calcEphemerides	
    aa
);

our $VERSION = '0.01';

use PS::MOPS::DC::Orbit;
use PS::MOPS::JPLEPH;

our $JPLEPH_EXE = "$ENV{MOPS_HOME}/bin/genEphem2";
die "can't locate genEphem2" unless -x $JPLEPH_EXE;

$ENV{CAET_DATA} = "$ENV{MOPS_HOME}/data/caet_data" unless $ENV{CAET_DATA};

my $STA_MAUNA_KEA = '568';


sub _orb2inp {
    my ($orb, $ephemEpoch_MJD) = @_;
    return {
        q_AU => $orb->q,
        e => $orb->e,
        i_deg => $orb->i,
        node_deg => $orb->node,
        argPeri_deg => $orb->argPeri,
        timePeri_MJD => $orb->timePeri,
        hV => $orb->hV,
        epoch_MJD => $orb->epoch,
        objectName => $orb->objectName,
    };
}


sub aa {
}


sub mopsephem_calcEphemerides {
    # Input modes:

    # objectName => $, ephemEpoch_MJD => $
    # %orb_params, ephemEpoch_MJD => $

    # objectName => [], ephemEpoch_MJD => $
    # orbits => [], ephemEpoch_MJD => $

    # objectName => $, ephemEpoch_MJD => []
    # %orb_params, ephemEpoch_MJD => []
    my %args = @_;
    my @inputs;
    my $inp;
    my $multi_orb;
    my $multi_date;

    # Populate inputs.  Make a list of orbital elements.  Then handle multiple
    # epochs if necessary.
    if (exists($args{orbits}) or exists($args{q_AU})) {
        # Must be orbital params.
        if (exists($args{orbits})) {
            push @inputs, @{$args{orbits}}; # push list of hashrefs
            $multi_orb = 1;
        }
        else {
            $args{objectName} ||= '0000001';
            push @inputs, \%args;    # %args has all req'd keys
        }
    }
    elsif (exists($args{objectName})) {
        my $orb;
        if (ref($args{objectName}) eq 'ARRAY') {
            # List of objectNames.
            foreach (@{$args{objectname}}) {
                $orb = modco_retrieve(objectName => $_);
                die "unknown objectName: $args{objectName}" unless $orb;
                push @inputs, _orb2inp($orb);
            }
            $multi_orb = 1;
        }
        else {
            # Single objectName.
            $orb = modco_retrieve(objectName => $args{objectName});
            die "unknown objectName: $args{objectName}" unless $orb;
            push @inputs, _orb2inp($orb);
        }
    }
    else {
        die "insufficient input";
    }

    # Handle dates.  If there's multiple orbits, add ephem date to each.  If there's
    # one, multiply by each ephem date.
    if (ref($args{ephemEpoch_MJD}) eq 'ARRAY') {
        die "can't do multi_date with multi_orb" if $multi_orb;
        my @input2;
        foreach my $d (@{$args{ephemEpoch_MJD}}) {
            push @input2, { %{$inputs[0]}, ephemEpoch_MJD => $d };  # copy hash
        }
        @inputs = @input2;   # copy new list to @input
        $multi_date = 1;
    }
    else {
        # single date, add to each input
        $_->{ephemEpoch_MJD} = $args{ephemEpoch_MJD} foreach @inputs;
    }

    # At this point we should have a complete list of orbits to calculate, described
    # by orbital parameters.


    my $OUTFILE = File::Temp::tempnam('/tmp', 'genEphem2');
    # note: JPLEPH DIVA lib noise redirected to /dev/null
    open GEN_EPHEM, "|$JPLEPH_EXE $OUTFILE > /dev/null" or die "can't start genEphem2";
    foreach $inp (@inputs) {
        # Write 'em.
        print GEN_EPHEM join(" ",
            $inp->{q_AU},
            $inp->{e},
            $inp->{i_deg},
            $inp->{node_deg},
            $inp->{argPeri_deg},
            $inp->{timePeri_MJD},
            $inp->{hV},
            $inp->{epoch_MJD},
            $inp->{ephemEpoch_MJD},
            $args{obsCode} || $STA_MAUNA_KEA,
            $inp->{objectName})
            . "\n";
    }
    close GEN_EPHEM;

    # Assemble results.  Modes:
    # single orb, single date: { ra_deg, dec_deg, mag }
    # multi orb, single date: { id1 => { ra_deg, dec_deg, mag }, id2 => { ra_deg, dec_deg, mag },  ... }
    # single orb, multi date: [ { ra_deg, dec_deg, mag }, { ra_deg, dec_deg, mag },  ... ]
    if ($multi_orb) {
        my $line;
        my ($objectName, $epoch, $ra, $dec, $mag);
        my %results;    # calculated ephemerides for named objects
        open INP_EPHEM, $OUTFILE or die "can't read $OUTFILE";
        while (defined($line = <INP_EPHEM>)) {
            if ($line !~ /ERROR/) {
                chomp $line;
                ($objectName, $epoch, $ra, $dec, $mag) = split /\s+/, $line, 5;
                $results{$objectName} = {
                    ra_deg => $ra,
                    dec_deg => $dec,
                    mag => $mag,
                };
            }
        }
        close INP_EPHEM;
        unlink($OUTFILE) or warn "problem unlinking $OUTFILE";
        return wantarray ? %results : \%results;
    }
    elsif ($multi_date) {
        my $line;
        my ($objectName, $epoch, $ra, $dec, $mag);
        my @results;    # calculated ephemerides listed by MJD
        open INP_EPHEM, $OUTFILE or die "can't read $OUTFILE";
        while (defined($line = <INP_EPHEM>)) {
            if ($line !~ /ERROR/) {
                chomp $line;
                ($objectName, $epoch, $ra, $dec, $mag) = split /\s+/, $line, 5;
                push @results, {
                    objectName => $objectName,
                    ephemEpoch_MJD => $epoch,
                    ra_deg => $ra,
                    dec_deg => $dec,
                    mag => $mag,
                };
            }
        }
        close INP_EPHEM;
        unlink($OUTFILE) or warn "problem unlinking $OUTFILE";
        return wantarray ? @results : \@results;
    }
    else {
        # Handle single orbit/single date.
        my $line;
        my ($objectName, $epoch, $ra, $dec, $mag);
        my %results;    # calculated ephemerides
        open INP_EPHEM, $OUTFILE or die "can't read $OUTFILE";
        $line = <INP_EPHEM>;    # read first line only
        if ($line !~ /ERROR/) {
            chomp $line;
            ($objectName, $epoch, $ra, $dec, $mag) = split /\s+/, $line, 5;
            %results = (
                ra_deg => $ra,
                dec_deg => $dec,
                mag => $mag,
            );
        }
        close INP_EPHEM;
        unlink($OUTFILE) or warn "problem unlinking $OUTFILE";
        return wantarray ? %results : \%results;
    }

    return undef;   # shouldn't get here
}


1;
__END__

=head1 NAME

PS::MOPS::Ephem - Perl extension for generic ephemeris services for PS::MOPS applications

=head1 SYNOPSIS

  use PS::MOPS::Ephem;
  
  # Ephemeris for single object by objectName
  $stuff = mopsephem_calcEphemerides(objectName => 'S000000')

  # Ephemeris for single object by orbitId
  $stuff = mopsephem_calcEphemerides(orbitId => '123456789')

  # Ephemeris for single object by orbital params
  $stuff = mopsephem_calcEphemerides(
    q_AU => 1.09,
    e => .507,
    i_deg => 19.47,
    node_deg => 169.07,
    argPeri_deg => 115.35,
    timePeri_MJD => 52783,
    hV => 15.5,
    epoch_MJD => 53300,
    ephemEpoch_MJD => 53377,
  )

  # Ephemeris for single object, multiple epochs
  $stuff = mopsephem_calcEphemerides(
    objectName => 'S000000',
    ephemEpoch_MJD = [ 53377, 53378, 53379 ],
  )

  # Ephemeris for single object by orbital params, multiple epochs
  $stuff = mopsephem_calcEphemerides(
    q_AU => 1.09,
    e => .507,
    i_deg => 19.47,
    node_deg => 169.07,
    argPeri_deg => 115.35,
    timePeri_MJD => 52783,
    hV => 15.5,
    epoch_MJD => 53300,
    ephemEpoch_MJD = [ 53377, 53378, 53379 ],
  )

  # Ephemeris for multiple objectNames, single ephem epoch
  $stuff = mopsephem_calcEphemerides(
    epochEpoch_MJD = 53377,
    objectNames => [
        'S000001', 'S000002', ...
    ],

  # Ephemeris for multiple orbital params, single ephem epoch
  $stuff = mopsephem_calcEphemerides(
    ephemEpoch_MJD = 53377,
    orbits => [
        { q_AU => $q, e => $e, ... },
        { q_AU => $q, e => $e, ... },
    ],
  )

=head1 DESCRIPTION

PS::MOPS::Ephem provides generic ephemeris services for PS::MOPS
applications.  Ephemerides may be calculated for derived objects in
the PSMOPS data collection, or for new or provisional objects during
orbit determination.

You may specify orbits using PSMOPS objectNames or orbital parameters.
Multiple objects or multiple ephemeris times may be specified, but
not both.

For a single object and ephemeris time, the result is a hash or HASHREF
with the following keys:

    epoch_MJD 
    ra_DEG 
    dec_DEG 
    mag

For multiple objectNames and a single ephemeris time, the result is
a hash or hashref containing keys/values that are objectNames:

    {
      'S000001' => { epoch_MJD, ra_DEG, dec_DEG, mag },
      'S000002' => { epoch_MJD, ra_DEG, dec_DEG, mag },
      ...
    }

For multiple ephemeris times, the result is a list or ARRAYREF where
each element is a HASHREF corresponding to the input array of epoch_MJDs.

    [
      { epoch_MJD, ra_DEG, dec_DEG, mag },
      { epoch_MJD, ra_DEG, dec_DEG, mag },
      ...
    ]
   

PS::MOPS::Ephem is a front-end to JPLEPH, which is a front-end to
genEphem2, which is a front-end to JPL's RCAODP F90 code.

=head1 AUTHOR

Larry Denneau, Jr. E<lt>denneau@ifa.hawaii.eduE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2005 Larry Denneau, Jr., Institute for Astronomy, University of Hawaii.

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself.

=cut
