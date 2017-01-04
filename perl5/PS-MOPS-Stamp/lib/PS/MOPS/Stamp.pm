package PS::MOPS::Stamp;

use 5.008;
use strict;
use warnings;
use Carp;
use Params::Validate;

use LWP::UserAgent;
use PS::MOPS::DC::Field;


use base qw(Exporter);
our @EXPORT = qw(
    submit_tracklet
    new_submit_tracklet
    submit_detection
    $STAMP_DIFF
    $STAMP_CHIP
    $STAMP_WARP
);
our $VERSION = '0.01';


our $STAMP_HOST = 'mops13.ifa.hawaii.edu:7000';
our $STAMP_DIFF = 'diff';
our $STAMP_WARP = 'warp';
our $STAMP_CHIP = 'chip';

use subs qw(
    submit_detection
    submit_tracklet
    new_submit_tracklet
);


sub submit_detection {
    my ($prefix, $det, $type, $nuke, $logfh) = @_;
    my $ra = $det->ra;
    my $dec = $det->dec;
    my $det_id = $det->detId;
    my $subdir = int($det->epoch) - 1;      # TJD to MOPS nn
    my $res;
    my @res2;
    my $sname = '';

    my $url = "http://$STAMP_HOST/get-special-stamp";
    my $ua;
    if ($type eq 'diff') {
        my $diffimid = $det->procId;
        my $digit;
        if ($det_id =~ /(\d)$/) {
            $digit = $1;
        }
        else {
            die "weird det ID $det_id";
        }
        $subdir = "$prefix/$subdir/$digit";
        $sname = "D$det_id";

        $ua = LWP::UserAgent->new(timeout => 10);
        $res = $ua->post($url, {
            type => $type,
            submitted => 'YES',
            ra => $ra,
            dec => $dec,
            size => 200,
            subdir => $subdir,
            sname => $sname,
            nuke => ($nuke ? 'YES' : 'NO'),
            diffimid => $diffimid,
            NOTE => "$subdir/$sname",
			 });
        @res2 = grep /^%/, split /\r|\n|\r\n/, $res->content;
    }
    elsif ($type eq 'chip') {
        my $digit;
        if ($det_id =~ /(\d)$/) {
            $digit = $1;
        }
        else {
            die "weird det ID $det_id";
        }
        my $field = modcf_retrieve($det->{_inst}, fieldId => $det->fieldId);
        $subdir = "$prefix/$subdir/$digit";
        $sname = "C$det_id";

        $ua = LWP::UserAgent->new(timeout => 10);
        $res = $ua->post($url, {
            type => $type,
            submitted => 'YES',
            ippexpnam => $field->fpaId,
            ra => $ra,
            dec => $dec,
            size => 200,
            subdir => $subdir,
            sname => $sname,
            nuke => ($nuke ? 'YES' : 'NO'),
            NOTE => "$subdir/$sname",
			 });
        @res2 = grep /^%/, split /\r|\n|\r\n/, $res->content;
    }

    if ($logfh) {
        print $logfh join(' ', scalar(localtime), $det->detId, "$subdir/$sname\n");
        print $logfh join("\n", @res2), "\n";
    }

    return $res;
}


sub submit_tracklet {
    my ($prefix, $trk, $type, $nuke, $logfh) = @_;
    my @res;
    foreach my $det (@{$trk->detections}) {
        push @res, submit_detection($prefix, $det, $type, $nuke, $logfh);
    }
    return @res;
}

sub new_submit_tracklet {
    my ($prefix, $trk, $type, $nuke, $path, $logger) = @_;
    my $res = "";
    foreach my $det (@{$trk->detections}) {
	my $detId = $det->detId;
	my $subdir = int($det->epoch) - 1;      # TJD to MOPS nn
        my $stampFileName = $path . "/" . $subdir . "/" . ($detId % 10) . "/D" . $detId . ".fits";
        if (-e $stampFileName) {
            if (!$nuke) {
                #print "[$stampFileName] already on disk (use --nuke?)\n";
                next;
            } else {
                #print "[$stampFileName] already on disk but --nuke is active\n";
                ;
            }
        } else {
	    #print "[$stampFileName] not on disk\n";
	    ;
        }
	my $trkId = $trk->trackletId;
	my $ra = $det->ra;
	my $dec = $det->dec;
	my $fieldId = $det->fieldId;
	my $field = modcf_retrieve($det->{_inst}, fieldId => $det->fieldId);
	my $exposureName = $field->fpaId;
	# TODO: Replace 200 200 by the dx dy of stamps read from configuration?
        my $priority = "LOWER";
        my $diffSkyfileId = $det->procId;
	# we need to identify if we are working on the positive or the negative image to get the right stamp
        my $image = "U"; # stands for Undefined
        if ($trk->digest > 99) {
	    $priority = "HIGHER";
        } elsif ($trk->digest > 94) {
            $priority = "HIGH";
        } elsif ($trk->digest > 59) {
	    $priority = "NORMAL";
	} elsif ($trk->digest > 29) {
            $priority = "LOW";
        }
	# At some point PS2 will have to be supported
	my $obscode = $field->obscode;
	# Note: The sign of $diffSkyfileId tells us if it is the positive or the negative image!
	$res .= "$obscode $detId $exposureName $ra $dec $type $diffSkyfileId $trkId 200 200 $priority\n";
    }
    
    return $res;
}


1;
__END__

=head1 NAME

PS::MOPS::Stamp - Minimal cheesy wrapper to Jan Kleyna's stamp
harvester

=head1 SYNOPSIS

  use PS::MOPS::Stamp;
  submit_tracklet($prefix, $detection);       # send async request to stamp server
  submit_tracklet($prefix, $tracklet);       # send async request to stamp server

=head1 DESCRIPTION

This module provides a wrapper to Jan Kleyna's stamp harvester in the
Manoa MOPS cluster.  Many things are hard-coded, and no attempt is made
to wait for the requests to finish.

The request requires a prefix, which may be a single directory or
two-level directory, and a output name for the stamp file.

=head1 AUTHOR

Larry Denneau, denneau@ifa.hawaii.edu

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2010 by Larry Denneau, Institute for Astronomy, University of Hawaii.

=cut
