#!/usr/bin/env perl

use strict ;
use warnings ;

use Getopt::Long ;
use Pod::Usage ;
use Data::Dumper ;

use PS::MOPS::DC::SSM ;
use PS::MOPS::DC::Field ;
use PS::MOPS::DC::Detection ;

sub unique (&@) {
  my($c,%hash) = shift ;
  grep{not $hash{&$c}++} @_
}

my $ocnum ;
my $help ;
GetOptions(
	'ocnum=i' => \$ocnum,
	help => \$help,
) or pod2usage(2) ;
pod2usage(2) unless $ocnum ;
pod2usage(-verbose => 2) if $help ;

print "Detections in OC", $ocnum, "\n" ;

# Find all the fields that were observed in the desired lunation.
my $num_fields = 0 ;
my $field ;
my @field_id ;
my $field_list = modcf_retrieve(ocnum=>$ocnum) ;
$num_fields = push @field_id, $field while($field=$field_list->next) ;

# Find all detections of objects releated to the desired lunation.
my $detections ;
my $detection ;
my $nextfield ;
my @all_detections;
my $num_detections_field ;
my $num_detections=0 ;
my @objects ;
my $num_objects ;
foreach $field (@field_id) {
  $nextfield = $field->fieldId ;
  my $detections = modcd_retrieve(fieldId=>$nextfield) ;
  while($detection=$detections->next) {
    $num_detections = push @all_detections, $detection;
    $num_objects = push @objects, $detection->objectName ; 
  }
}

# Remove duplicate objects.
my @unique_objects = unique{$_} @objects ;

# Find number of detections per object.
# Note that we also have to find the number of days the object was observered.
my %detection_object = () ;
my %nights_object_tmp = () ;
foreach $detection(@all_detections) { 
    $detection_object{$detection->objectName}++ ;
    $nights_object_tmp{$detection->objectName}{int($detection->epoch)} = 1 ;
}
my $r_nights_object_tmp = \%nights_object_tmp ;

my %nights_object = () ;
my $object ;
foreach $object (keys %$r_nights_object_tmp) {
   $nights_object{$object} = scalar %{$r_nights_object_tmp->{$object}} ;
}

my $objectkey ;
my $num_det_object ;
my %object_detection = () ;
while(($objectkey, $num_det_object) = each (%detection_object)) {
    $object_detection{$num_det_object}{$nights_object{$objectkey}}++ ;
}
my $r_object_detection = \%object_detection ;

# Output the numbers found.
print "Number of fields: ", $num_fields, "\n" ;
print "Number of detections: ", $num_detections, "\n" ;
print "Number of objects: ", $#unique_objects+1, "\n" ;
my $number ;
my $num_nights_object ;
foreach $num_det_object (sort keys %$r_object_detection) {
  foreach $num_nights_object ( sort keys %{$r_object_detection->{$num_det_object}}) {
    print "Detections: ", $num_det_object ;
    print "  Nights: ", $num_nights_object ;
    print "  objects: ", $r_object_detection->{$num_det_object}{$num_nights_object}, "\n" ;
  }
}



=head1 NAME

countDetections

=head1 SYNOPSIS

countDetections <input>

=head1 DESCRIPTION

Count Detections gives the total number of detections, number of 
objects.

=cut

