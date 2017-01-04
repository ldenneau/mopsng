package PS::MOPS::DC::Connection;

use 5.008;
use strict;
use warnings;

require Exporter;

our @ISA = qw(Exporter);

# Legacy imports.
our %EXPORT_TAGS = ( 'all' => [ qw(
    get_dbh	
    new_dbh	
    modc_getdbh
    modc_newdbh
    modc_pushAutocommit
    modc_popAutocommit
    modc_commit
    modc_backend
    modc_forgetdbh
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
    modc_getdbh
    modc_newdbh
    modc_pushAutocommit
    modc_popAutocommit
    modc_commit
    modc_backend
    modc_forgetdbh
);

our $VERSION = '0.01';

# Our stuff.
use DBI;
use Params::Validate;
use PS::MOPS::Config;
use PS::MOPS::Log;

our $mops_dbname;
our $mops_dbh;
our @ac_stack;  # AutoCommit stack

# Backend setup
our $mops_backend = ($ENV{MOPS_BACKEND} or 
    $mops_config->{database}->{backend} or 
    $mops_logger->logdie("can't determine DC backend"));

our $mops_dbinstance = $ENV{MOPS_DBINSTANCE} || $mops_config->{database}->{dbinstance}; # which database name
our $mops_hostname = $ENV{MOPS_DBHOSTNAME} || $mops_config->{database}->{hostname};     # host to connect to
our $mops_port = $ENV{MOPS_DBPORT} || $mops_config->{database}->{port} || 3306;     # port to connect to
our $mops_username = $ENV{MOPS_DBUSER} || $mops_config->{database}->{username};         # DB username
our $mops_password = $ENV{MOPS_DBPASSWORD} || $mops_config->{database}->{password};     # DB password
our @mops_connect;

&modc_setdbname();       # init DB name


sub modc_backend {
    return $mops_backend;
}


sub get_dbh {
    if (!$mops_dbh) {
        $mops_dbh = DBI->connect(@mops_connect) or die "can't connect to PSMOPS";
        $mops_dbh->{mysql_auto_reconnect} = 1;
    }
    return $mops_dbh;
}
*modc_getdbh = \&get_dbh;   # new (better) name


sub new_dbh {
    # Create a new database connection using flags provided by caller.
    my %args = validate(@_, {
            flags => 0
        });
    my $dbh;
    $dbh = DBI->connect(@mops_connect, $args{flags}) or die "can't connect to PSMOPS";
    return $dbh;
}
*modc_newdbh = \&new_dbh;   # new (better) name


sub modc_pushAutocommit {
    # set AutoCommit flag as specified; preserving current setting so we
    # can restore to previous.
    my ($new_ac) = validate_pos(@_, 1);
    my $dbh = get_dbh();    # force connect if not already
    push @ac_stack, $dbh->{AutoCommit};    # save current setting
    $dbh->{AutoCommit} = $new_ac;
}


sub modc_popAutocommit {
    # restore previous AutoCommit setting.
    if (@ac_stack == 0) {
        $mops_logger->warn("pop_autocommit with empty stack");
        return;
    }
    my $dbh = get_dbh();    # force connect if not already
    my $cur_ac = $dbh->{AutoCommit};
    $dbh->{AutoCommit} = pop @ac_stack;
    return $cur_ac; # return former value
}


sub modc_commit {
    # just commit dbh
    $mops_dbh->commit;
}


sub modc_forgetdbh {
    # "Forget" our global DBH so that new DB handles are created at next
    # request.  This sub exists to support ithreads operation.
    undef $mops_dbh;
}


sub modc_setdbname {
    my $mops_dbname = $_[0];

    $mops_dbinstance = $mops_dbname || 
        $ENV{MOPS_DBINSTANCE} || 
        $mops_config->{database}->{dbinstance}; # which database name

    if ($mops_backend =~ /oracle/i) {
        @mops_connect = ($ENV{MOPS_DBCONNECT} || "dbi:Oracle:host=$mops_hostname;sid=$mops_dbinstance;port=$mops_port", $mops_username, $mops_password);
    }
    elsif ($mops_backend =~ /mysql/i) {
        # If hostname is 'local' or unspecified use local connection.
        if ($ENV{MOPS_DBCONNECT}) {
            @mops_connect = ($ENV{MOPS_DBCONNECT}, $mops_username, $mops_password);
        }
        elsif (!$mops_hostname or ($mops_hostname eq 'local')) {
            @mops_connect = ("dbi:mysql:port=$mops_port;database=$mops_dbinstance", $mops_username, $mops_password);
        }
        else {
            @mops_connect = ("dbi:mysql:host=$mops_hostname;port=$mops_port;database=$mops_dbinstance", $mops_username, $mops_password);
        }
    }
    else { 
        die "unknown backend: $mops_backend";
    }

    undef $mops_dbh;
}


1;
__END__

=head1 NAME

PS::MOPS::DC::Connection - MOPS DC connection management

=head1 SYNOPSIS

  use PS::MOPS::DC::Connection;
  my $dbh = get_dbh();  # get DBI handle
  $dbh->prepare($sql_statement);
  etc.

=head1 DESCRIPTION

PS::MOPS::DC::Connection provides connection management to the SAIC MOPS
Oracle DC database.  Upon first request for a handle, it establishes a
DBI connection to the database and stores the database handle.

=item modc_setdbname

Sets the database name to be used for all future connections, and
undefs the package-global connection $mops_dbh so that any future
modc_getdbh() requests establish a new connection.

=head1 EXPORTS

    modc_getdbh
    modc_newdbh
    modc_pushAutocommit
    modc_popAutocommit
    modc_commit
    modc_backend
    modc_forgetdbh
    modc_setdbname

=head1 SEE ALSO

DBI
DBD::Oracle

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2004 by A. U. Thor

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut
