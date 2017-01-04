#!/usr/bin/env perl

use strict ;
use Data::Dumper ; # inspection of variables ;

use PS::MOPS::DC::SSM ;
use PS::MOPS::DC::Shape ;
use PS::MOPS::DC::Detection ;

# 0. Fetch detections
my $detection_it = modcd_retrieve(objectName=>'S000001') ;
my $detection ;
my @all_detections ;
push @all_detections, $detection while ($detection=$detection_it->next) ;
print Dumper(\@all_detections) ; 

# 1. Fetch an SSM object description.
my $ssm_obj = modcs_retrieve( objectName=>'S000001' ) ;
my $a_AU = $ssm_obj->q*(1-$ssm_obj->e) ; # calculate the semi-major axis.

#my $shape_info = modcsh_retrieve(ssmld => $ssm_obj->ssmld); # fetch shape description.
print Dumper($ssm_obj) ;

# 2. Fetch all SSM objects.
my @ssm_list ;  # list to be populated.
my $ssm_it ; # the iterator 
my $ssm_it = modcs_retrieve(all=>1) ;
push @ssm_list, $ssm_obj while ($ssm_obj=$ssm_it->next) ;
print Dumper(\@ssm_list) ;

# 3. Fetch fields for one night.
my @field_list ;
my $field_it = modcf_retrieve(date_mjd=>53373) ;
my $field ;
push @field_list, $field while($field=$field_it->next) ;
print Dumper(\@field_list) ;

=head1 NAME

  db_example.pl - example PS::MOPS::DC module usage 

=head1 DESCRIPTION

  This is a short program to demonstrate usage of the PS::MOPS::DC
Perl modules.

Make sure that you have your $MOPS_HOME set up!

=head1 MORE INFO

  Just perldoc modules you are interested in:

     perldoc PS::MOPS::Lib
     perldoc PS::MOPS::Constants
     perldoc PS::MOPS::DC::SSM (modcs_* functions)
     perldoc PS::MOPS::DC::Shape (modcsh_* functions)
     perldoc PS::MOPS::DC::Field (modcf_* functions)
     perldoc PS::MOPS::DC::Orbit (modco_* functions)
     perldoc PS::MOPS::DC::Detection (modcd_* functions)
     perldoc PS::MOPS::DC::Tracklet (modct_* functions)

=cut

