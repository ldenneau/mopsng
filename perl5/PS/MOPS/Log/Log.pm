package PS::MOPS::Log;

use 5.008;
use strict;
use warnings;

require Exporter;

our @ISA = qw(Exporter);

# Note: $logger is deprecated
our @EXPORT = qw(
    $mops_logger
);

our $VERSION = '0.01';


# This is it.
use Log::Log4perl;
our $mops_logger;

my $MOPS_HOME = $ENV{MOPS_HOME};
if (-d "$MOPS_HOME" && -f "$MOPS_HOME/config/log4perl.conf") {
    Log::Log4perl->init_once("$MOPS_HOME/config/log4perl.conf");
    $mops_logger = Log::Log4perl->get_logger('');    # get root logger
}
else {
    warn "can't locate $MOPS_HOME; using plain log config\n";
    my $cfg = <<'END';
log4perl.logger = INFO, SCREEN
log4perl.appender.SCREEN = Log::Log4perl::Appender::Screen
log4perl.appender.SCREEN.filename = /var/log/myerrs.log
log4perl.appender.SCREEN.layout = PatternLayout
log4perl.appender.SCREEN.layout.ConversionPattern = %d %m %n
END

    Log::Log4perl->init_once(\$cfg);
    $mops_logger = Log::Log4perl->get_logger('');
}

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

PS::MOPS::Log - one-line logging setup for PS::MOPS modules

=head1 SYNOPSIS

  use PS::MOPS::Log;
  $logger->fatal("something really bad happened!");

=head1 DESCRIPTION

This module sets up Log::Log4perl logging facilities for MOPS modules
and programs.  Simply use this module in your code and the Log4perl root
logger object will be imported into your package space as variable $logger.

If you need custom logging behavior, you're free to use your own Log4perl
setup.

=head2 EXPORT

logger

=head1 SEE ALSO

Log::Log4perl

=head1 AUTHOR

Larry Denneau, dennea@ifa.hawaii.edu

=head1 COPYRIGHT AND LICENSE

Copyright 2004 Institute for Astronomy, University of Hawaii

=cut
