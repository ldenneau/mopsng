package PS::MOPS::DC::Test;

use 5.008;
use strict;
use warnings;

use base qw(Exporter);

our %EXPORT_TAGS = ( 'all' => [ qw(
	
) ] );
our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );
our $VERSION = '0.01';


# Preloaded methods go here.

1;
__END__

=head1 NAME

PS::MOPS::DC::Test - Perl module for high-level MOPS DC testing

=head1 SYNOPSIS

  use PS::MOPS::DC::Test qw(:all);

  modctest_build(   
    PARAMS => 1,
    SSM_MODEL => $MODEL_100,
    
    CONFIG => {
        # complete master.cf configuation allowed
    },
    # other creation parameters here
    CLEANUP => 0,           # don't delete when done (default is to delete)
  );         # build model with fields and SSM

  modctest_DTCTL();         # run detections and tracklets
  modctest_LODCTL();        # run linking and OD

  # Analyze DB objects
  modco_something();

=head1 DESCRIPTION

PS::MOPS::DC::Test provides high-level MOPS DC services for the testing
of other components.  This module can create an entire PSMOPS instance
on-the-fly and then run the model as specified so that other modules can
test their operation on the model.  When the calling program exits, the
entire instance is deleted, so that any online models are not perturbed.

=head1 SEE ALSO

PS::MOPS::DC
PS::MOPS::DC::Field
PS::MOPS::DC::SSM
PS::MOPS::DC::Detection
PS::MOPS::DC::Tracklet
PS::MOPS::DC::Orbit

=head1 AUTHOR

Larry Denneau, Jr., <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2005 by Larry Denneau, Jr., Institute for Astronomy, University
of Hawaii.

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut
