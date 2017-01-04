#!/usr/bin/perl

# $Id$
# Separate support code stub for testing.

=head1 NAME

update_chunk - Change the status of fields that make up the chunk from ingested
               to new.
 
=head1 SYNOPSIS

update_chunk.pl --instance INST --nn NIGHTNUM 'CHUNK1 CHUNK2 ...'

   CHUNK1 CHUNK2 	: name of the chunk(s) to be updated.
  --instance INST	: name of simulation containing chunks to be updated.
  --nn				: night on which chunks to be updated were observed.

=head1 DESCRIPTION

Update the status of ingested chunks from Ingested to Ready for processing.

=cut

use strict;
use warnings;
use Getopt::Long;
use Pod::Usage;

use PS::MOPS::Admin qw(change_chunk_status);

# Process command line options and arguments.
my $debug;
my $nn;
my $help;
my $instance_name;
my $chunks;
my $chunk;
GetOptions(
    debug => \$debug,
    'instance=s' => \$instance_name,
    'nn=i' => \$nn,
    help => \$help,
) or pod2usage(2);
pod2usage(-verbose => 3) if $help;

my $result = change_chunk_status($instance_name, $nn, @ARGV);

print "Updated $result fields.";

