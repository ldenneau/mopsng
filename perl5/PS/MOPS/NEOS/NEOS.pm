package PS::MOPS::NEOS;

use 5.008;
use strict;
use warnings;

require Exporter;

use Params::Validate ':all';
use Cwd;
use PS::MOPS::MPC ':all';

our @ISA = qw(Exporter);

our %EXPORT_TAGS = ( 'all' => [ qw(
    neos_run
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	
);

our $VERSION = '0.01';

# Globals.
our $MOPS_HOME = $ENV{MOPS_HOME};
our $NEOS_DIR = "$MOPS_HOME/var/NEOS";
our $NEOS_PROG = './neos';

our $NEOS_TRACKS = 'tracks.sum';
our $NEOS_ORBITS = 'tracks.orb';


sub _prep {
    # Check that required stuff is there.
    my ($args) = @_;
    chdir $NEOS_DIR or die "can't chdir to $NEOS_DIR\n";
    die "\$MOPS_HOME not specified\n" unless -d $MOPS_HOME;
    die "can't find neos dir $NEOS_DIR\n" unless -d $NEOS_DIR;

    # Write out detections file.
    $args->{DETECTIONS_FILE} = "work/$$.mpc";
    mpc_write($args->{detections}, $args->{DETECTIONS_FILE});
}



sub _go {
    my ($args) = @_;

    # Build command line, and execute NEOS.
    my @cmd = (
        $NEOS_PROG,
        'file' => $args->{DETECTIONS_FILE},
        'eval' => 'true',
        'fit_orbs' => ($args->{fit_orbs} ? 'true' : 'false'),
    );

    die "can't locate neos executable\n" unless -x $NEOS_PROG;

    # Capture output.
    my $cmd_str = join ' ', @cmd;
    $args->{NEOS_COMMAND} =  $cmd_str;
    if ($args->{capture_output}) {
        # qx() the command, so we can get the output.
        my $output = qx($cmd_str);
        $args->{OUTPUT} = $output;  # save it
    }
    else {
        # Just call using system(), STDOUT and STDERR unchanged.
        my $rc = system @cmd;
        if ($rc) {
            die "invocation of '$NEOS_PROG' failed: $?\n";
        }
    }
}


sub _snarf {
    my ($args) = @_;

    # Grab results from NEOS files, mostly tracks.sum (or equivalent).
    my $orbit;
    my @orbits;
    my %orbit_tbl;

    my $track;
    my @tracks;
    my %track_tbl;

    my $line;
    my @stuff;
    my ($tr_id, $det_id, $line_no);

    # Read orbits file.  Keep these around for next step (reading tracks file).
    if ($args->{fit_orbs}) {
        open my $orbits_fh, $NEOS_ORBITS or die "can't open $NEOS_ORBITS\n";
        @orbits = <$orbits_fh>;
        chomp @orbits;
        close $orbits_fh;
        push @{$args->{NEOS_FILES}}, $NEOS_ORBITS;

        foreach $line (@orbits) {
            ($tr_id, @stuff)  = split /\s+/, $line;
            shift @stuff;    # toss leading ':'
            $orbit_tbl{$tr_id} = { 
                COEFS => \@stuff,
                RMS => $stuff[8],
            };
        }
    }


    # Read tracks file.
    open my $tracks_fh, $NEOS_TRACKS or die "can't open $NEOS_TRACKS\n";
    @tracks = <$tracks_fh>;
    chomp @tracks;
    close $tracks_fh;
    push @{$args->{NEOS_FILES}}, $NEOS_TRACKS;

    # Munge track data into table of tracks.  Each track is a hashref containing an ID and list of
    # detection indices.  Each input line of the track data file is
    #
    # TRACK_ID  DETECTION_ID    LINE_NUMBER
    #
    my $o;
    foreach $line (@tracks) {
        ($tr_id, $det_id, $line_no) = split /\s+/, $line, 3;

        if (!exists($track_tbl{$tr_id})) {
            if ($args->{fit_orbs}) {
                $o = $orbit_tbl{$tr_id} or warn "no orbit found for track ID=$tr_id\n";
                $track_tbl{$tr_id} = {
                    ORBIT => $o,
                    DETECTIONS => [$line_no],
                };
            }
            else {
                # fit_orbs was not specified, so there's no orbit data to refer to.
                $track_tbl{$tr_id} = {
                    DETECTIONS => [$line_no],
                };
            }
        }
        else {
            push @{$track_tbl{$tr_id}->{DETECTIONS}}, $line_no; # add line id to table
        }
    }

    # Save our stuff.
    $args->{TRACKS} = \%track_tbl;
    $args->{ORBITS} = \%orbit_tbl;
}


sub _cleanup {
    my ($args) = @_;
    my $file;

    return if $args->{no_delete};   # don't delete NEOS files

    foreach $file (@{$args->{NEOS_FILES}}) {
        unlink($file) or warn "can't remove $file\n";
    }
    unlink($args->{DETECTIONS_FILE}) or warn "can't remove $args->{DETECTIONS_FILE}\n";
}


sub neos_run {
    my %args = validate(@_, {
            detections => 1,   
            lin_thresh => 0,
            lin_thresh_t => 0,
            lin_thresh_i => 0,
            quad_thresh => 0,
            quad_thresh_t => 0,
            quad_thresh_i => 0,
            fit_thresh => 0,
            fit_thresh_t => 0,
            fit_thresh_i => 0,
            max_hyp => 0,
            max_match => 0,
            min_obs => 0,
            max_mean => 0,
            'eval' => 0,
            fit_orbs => 0,
            keep_bad_orbits => 0,
            remove_subsets => 0,
            remove_overlaps => 0,
            allow_conflicts => 0,
            min_overlap => 0,
            plate_width => 0,

            no_delete => 0, # don't delete NEOS output files
            capture_output => { default => 1 },
        }
    );

    my $oldwd;
    my $results;
    eval {
        $oldwd = getcwd;
        _prep(\%args);    # create working dir(?), write input files from data
        _go(\%args);      # execute neos
        _snarf(\%args);   # grab results
    };

    _cleanup(\%args);   # remove neos output files, tmp files, etc.
    warn $@ if $@;
    chdir $oldwd if $oldwd;    # restore working directory

    return \%args;
}

1;
__END__

=head1 NAME

PS::MOPS::NEOS - Perl module to wrap NEOS execution

=head1 SYNOPSIS

  use PS::MOPS::NEOS ':all';
  use PS::MOPS::MPC ':all';

  $detections = mpc_slurp('detections.mpc');        # read file into mem
  $ret = neos_run(
    detections => $detections,
    lin_thresh => $lin_thresh,
    quad_thresh => $quad_thresh,
    etc..
  );       # call NEOS

  $tracks = $ret->{TRACKS};             # track data structures
  $orbits = $ret->{ORBITS};             # orbit data structures

=head1 DESCRIPTION

This module wraps invocations of Jeremy Kubica's NEOS program so that
it can be run from Perl.  Inputs to NEOS initially will consist of Perl
lists of detections obtained from PS::MOPS::MPC, and the NEOS output
(normally three files, tracks.sum, tracks.obs and tracks.orb) will
be consilidated into a single multilevel Perl structure.  The result
output data can easily be re-input to NEOS for further processing of
the same input dataset; for example, to extract detections that have
been associated with low-residual orbits and process the remainder.

=head1 SEE ALSO

PS::MOPS::MPC

=head1 AUTHOR

Larry Denneau, <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2004 Institute for Astronomy, University of Hawaii

=cut
