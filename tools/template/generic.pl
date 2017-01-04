#! /usr/bin/env perl
# $Id$

=head1 NAME

generic - Template MOPS Perl script

=head1 SYNOPSIS

generic [options] [FILENAMES]

  --instance=INSTANE_NAME : specify simulation to use

=head1 DESCRIPTION

Does something.

=cut

use warnings;
use strict;

use Getopt::Long;
use Pod::Usage;
use FileHandle;
use File::Copy;

use PS::MOPS::DC::Instance;
use PS::MOPS::Lib qw(:all);
use PS::MOPS::Constants qw(:all);

use PS::MOPS::DC::Field;
use PS::MOPS::DC::Detection;
use PS::MOPS::DC::Tracklet;
use PS::MOPS::DC::Orbit;
use PS::MOPS::DC::SSM;
use PS::MOPS::DC::DerivedObject;
use PS::MOPS::DC::History;
use PS::MOPS::DC::History::Derivation;
use PS::MOPS::DC::History::Attribution;
use PS::MOPS::DC::History::Identification;
use PS::MOPS::DC::History::Precovery;
use PS::MOPS::DC::Efficiency;
use PS::MOPS::JPLEPH;

#use PS::MOPS::FITS::IPP;


my $instance_name;
my $inst;
my $help;

GetOptions(
    'instance=s' => \$instance_name,
    help => \$help,
) or pod2usage(2);                      # opts parse error
pod2usage(-verbose => 3) if $help;      # user asked for help

$inst = PS::MOPS::DC::Instance->new(DBNAME => $instance_name);
my $mops_config = $inst->getConfig();
my $mops_logger = $inst->getLogger();

# Now check some args.
#pod2usage(-msg => 'No filenames specified.') unless @ARGV;

exit;

