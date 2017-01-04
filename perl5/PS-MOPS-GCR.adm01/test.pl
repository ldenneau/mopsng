#!/usr/bin/env perl
#
use strict;
use warnings;
use PS::MOPS::GCR;

#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.

use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Tracklet;

my $tid = shift || 224782;
my $inst = PS::MOPS::DC::Instance->new(DBNAME => undef);
my $trk = modct_retrieve($inst, trackletId => $tid);
my $detref = $trk->detections;

my $gcr_arcsec = PS::MOPS::GCR::compute_gcr($detref);
print $gcr_arcsec, "\n";

my $gcr_vel_dd = PS::MOPS::GCR::compute_vel($detref);
print $gcr_vel_dd, "\n";
