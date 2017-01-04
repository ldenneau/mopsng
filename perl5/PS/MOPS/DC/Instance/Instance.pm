package PS::MOPS::DC::Instance;

=head1 NAME

PS::MOPS::DC::Instance - MOPS DC instance management

=head1 SYNOPSIS

  use PS::MOPS::DC::Instance;

  my $cxn = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_test');
  $field = PS::MOPS::DC::Field->new($cxn, %PARAMS);
  $field->insert;

  $homedir = $cxn->getEnvironment('HOMEDIR');      # old $MOPS_HOME
  $vardir = $cxn->getEnvironment('VARDIR');        # old $MOPS_VAR
  $logdir = $cxn->getEnvironment('LOGDIR');        # old $MOPS_VAR/log
  $configdir = $cxn->getEnvironment('CONFIGDIR');  # old $MOPS_VAR/config

=head1 DESCRIPTION

PS::MOPS::DC::Instance provides database and filsystem management to
MOPS database instance.  The Instance object is the entry point to any
code that retreives, inserts or manipulates objects in a MOPS database.
Filesystem directories that are used by the instance are managed by a
Instance object.

=head1 METHODS

=item new

Return a new instance handle for the specified database instance.

=item pushAutocommit

Save the autocommit state of the current instance database handle
and set it to the specified value.  The old state can be restored
using $instance->popAutocommit.

=item popAutocommit

Restore the autocommit state previously saved using pushAutocommit.

=item getEnvironment

Return the specified 'environment' setting, usually a directory used
by the instance.  Currently recognized environment values are

   HOMEDIR : MOPS home directory
   VARDIR : instance home directory
   CONFIGDIR : configuration file directory
   NNDIR : per-night processing directory tree
   OBJECTSDIR : SSM object cache directory
   LSDDIR : LSD archive directory

=item getLogger

Return a Log::Log4perl object that can be used to write log messages
for this instance.

=item getDBH

Get the global database handle (DBH) for this instance.

=item newDBH

Return a new DBH using this instance's connection params.

=head1 BUGS

The usage pattern for instances suggests that the MOPS modcXX_YYYY
functions should really be method calls of an instance objet.  This would
introduce alot of cruft into the PS::MOPS::DC::Instance module, so we're
not doing it (so far).

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2006 by Larry Denneau, Jr., Institute for Astronomy,
University of Hawaii.

=cut

use 5.008;
use strict;
use warnings;

use base qw(Exporter Class::Accessor);
our @EXPORT = qw();
our $VERSION = '0.01';

# Our stuff.
use Carp;
use File::Path;
use DBI;
use Params::Validate;

use PS::MOPS::DC::Config;
use PS::MOPS::Config;
use PS::MOPS::Lib qw(:all);


sub _load_config_module {
    my ($self, $which) = @_;
    my $inst_config = $self->{_ENV}->{CONFIGDIR} . "/${which}.cf";
    my $mops_config = $ENV{MOPS_HOME} . "/config/${which}.cf";
    if (-f $inst_config) {
        return PS::MOPS::Config::LoadFile($inst_config);
    }
    elsif (-f $mops_config) {
        return PS::MOPS::Config::LoadFile($mops_config);
    }
    else {
        my $logger = $self->{_LOGGER};
        if ($logger) {
            $logger->info("no config found for $which");
        }
        return {};          # empty
    }
}


sub new {
    my $pkg = shift;
    my %args = validate(@_, {
        DBNAME => 1,
        BACKEND => 0,
    });

    my $dbname = $args{DBNAME} || $ENV{MOPS_DBNAME} || $ENV{MOPS_DBINSTANCE};
    die "can't determine a database name" unless $dbname;       # punt

    my %self;

    # Set up runtime environment.
    $self{_ENV}->{HOMEDIR} = $ENV{MOPS_HOME} || "/usr/local/MOPS";                  # home directory
    $self{_ENV}->{VARDIR} = $self{_ENV}->{HOMEDIR} . "/var/$dbname";                # var directory
    $self{_ENV}->{CONFIGDIR} = $self{_ENV}->{VARDIR} . "/config";                   # per-instance configs
    $self{_ENV}->{NNDIR} = $self{_ENV}->{VARDIR} . "/nn";                           # per-night processing data
    $self{_ENV}->{OBJECTSDIR} = $self{_ENV}->{VARDIR} . "/objects";                 # SSM object cache
    $self{_ENV}->{LSDDIR} = $self{_ENV}->{VARDIR} . "/lsd";                         # LSD archive

    $self{_AC_STACK} = [];   # autocommit stack
    $self{_DBNAME} = $dbname;                                     # which database name

    # Load configuration.  Attempt to load configuration data in this order:
    # backend.cf : 
    #     from file
    # cluster.cf : 
    #     from file
    # master.cf : 
    #     from database
    #     from file
    # Then copy backend and cluster configs into master config data.

    my $backend_cfg = _load_config_module(\%self, 'backend');
    my $cluster_cfg = _load_config_module(\%self, 'cluster');


    # Database stuff.
    $self{_DBHOSTNAME} = $ENV{MOPS_DBHOSTNAME} || $backend_cfg->{hostname};       # host to connect to
    $self{_DBPORT} = $ENV{MOPS_DBPORT} || $backend_cfg->{port} || 3306;       # port to connect to
    $self{_DBUSERNAME} = $ENV{MOPS_DBUSERNAME} || $backend_cfg->{username};       # DB username
    $self{_DBPASSWORD} = $ENV{MOPS_DBPASSWORD} || $backend_cfg->{password};       # DB password
    $self{_DBH} = undef;

##    $self{_DBCONNECT} = '';
    if (%{$backend_cfg}) {
        if ($backend_cfg->{backend} =~ /oracle/i) {
            die "Oracle backend is unsupported";
            $self{_DBCONNECT} = [
                ($ENV{MOPS_DBCONNECT} || "dbi:Oracle:host=$self{_DBHOSTNAME};sid=$self{_DBINSTANCE};port=$self{_DBPORT}"),
                $self{_DBUSERNAME},
                $self{_DBPASSWORD},
                {
                    RaiseError => 1,
                },
            ];
        }
        elsif ($backend_cfg->{backend} =~ /mysql/i) {
            # If hostname is 'local' or unspecified use local connection.
            if ($ENV{MOPS_DBCONNECT}) {
                $self{_DBCONNECT} = [
                    $ENV{MOPS_DBCONNECT},
                    $self{_DBUSERNAME},
                    $self{_DBPASSWORD},
                    {
                        RaiseError => 1,
                    },
                ];
            }
            elsif (!$self{_DBHOSTNAME} or ($self{_DBHOSTNAME} eq 'local')) {
                $self{_DBCONNECT} = [
                    "dbi:mysql:database=$dbname;port=$self{_DBPORT}",
                    $self{_DBUSERNAME},
                    $self{_DBPASSWORD},
                    {
                        RaiseError => 1,
                    },
                ];
            }
            else {
                $self{_DBCONNECT} = [
                    "dbi:mysql:host=$self{_DBHOSTNAME};database=$dbname;port=$self{_DBPORT}",
                    $self{_DBUSERNAME},
                    $self{_DBPASSWORD},
                    {
                        RaiseError => 1,
                    },
                ];
            }
        }
        else {
            die "unknown backend: $backend_cfg->{backend}";
        }
    }   # if (%{$backend_cfg})
    bless \%self;


    my $config_text;
    eval {
       $config_text = modcfg_retrieve(\%self);
    };
    if ($@) {
        my $logger = getLogger(\%self);
        $logger->warn("Error finding master config; using files instead.") if $@;
    }

    if ($config_text) {
        $self{_CONFIG} = PS::MOPS::Config->new_from_string($config_text);
    }
    else {
        $self{_CONFIG} = _load_config_module(\%self, 'master');
    }
    $self{_CONFIG}->{backend} = $backend_cfg;
    $self{_CONFIG}->{cluster} = $cluster_cfg;

    # Config compatibility hacks.
    if (!defined($self{_CONFIG}->{site}->{field_size_deg2})
        and defined($self{_CONFIG}->{ssm}->{field_size_deg2})) {
        # copy field_size_deg2 from ssm to site, where it's supposed to live.
        $self{_CONFIG}->{site}->{field_size_deg2} = $self{_CONFIG}->{ssm}->{field_size_deg2};
    }

    return \%self;
}


sub DESTROY {
    # Disconnect our database handle prior to destroy.
    my $self = shift;
    if ($self->{_DBH}) {
        $self->{_DBH}->disconnect;
        undef($self->{_DBH});
    }
}


sub dbname {
    my $self = shift;
    return $self->{_DBNAME};
}


sub dbh {
    my $self = shift;
    if (!$self->{_DBH}) {
        $self->{_DBH} = DBI->connect(@{$self->{_DBCONNECT}}) or die "can't connect to PSMOPS";
        $self->{_DBH}->{mysql_auto_reconnect} = 1;
    }
    return $self->{_DBH};
}


sub new_dbh {
    # Create a new database instance handle using flags provided by caller.
    my $self = shift;
    my %args = validate(@_, {
            flags => 0
        });
    my $dbh;
    my %flags = %{$self->{_DBCONNECT}->[3]};                        # get flags portion of _DBCONNECT
    $flags{$_} = $args{flags}->{$_} foreach keys %{$args{flags}};   # map input flags on top
    $dbh = DBI->connect(@{$self->{_DBCONNECT}}[0..2], \%flags) or die "can't connect to PSMOPS";
    return $dbh;
}


sub forget_dbh {
    # Undef our database handle so that next attempt at usage establishes a new
    # connection.  Normally you would only use this after a potentially lengthy
    # operation where the DBH might time out (yes, should be fixed in mysql server).
    my $self = shift;
    undef($self->{_DBH});       # boink
}


sub pushAutocommit {
    # set AutoCommit flag as specified; preserving current setting so we
    # can restore to previous.
    my $self = shift;
    my ($new_ac) = validate_pos(@_, 1);
    my $dbh = $self->dbh;
    push @{$self->{_AC_STACK}}, $dbh->{AutoCommit};
    #$dbh->{AutoCommit} = $new_ac;
}


sub popAutocommit {
    # restore previous AutoCommit setting.
    my $self = shift;
    if (@{$self->{_AC_STACK}} == 0) {
        die "pop_autocommit with empty stack";
        return;
    }
    my $dbh = $self->dbh;
    my $cur_ac = $dbh->{AutoCommit};
    #$dbh->{AutoCommit} = pop @{$self->{_AC_STACK}};
    return $cur_ac; # return former value
}


sub atomic {
    # Wrap the specified coderef into an atomic operation.  If the database handle
    # already has autocommit turned off, do nothing.  If autocommit is on, turn it
    # off and start a transaction.  After the coderef executes, perform a commit.
    # If an exception (die) occurred, rollback the database and re-throw the exception.
    my ($inst, $dbh, $coderef) = @_;
    my $orig_autocommit = $dbh->{AutoCommit};

    if ($orig_autocommit) {
        $dbh->begin_work;
        eval &$coderef;
        if ($@) {
            $dbh->rollback;
            die $@;
        }
        else {
            $dbh->commit;
        }
    }
    else {
        # AutoCommit is already off, so we should already be in a transaction.
        eval &$coderef;
        die $@ if $@;
    }
}


sub getEnvironment {
    my ($self, $key) = @_;
    croak "non-existent isntance environment key requested: $key" unless exists(${$self->{_ENV}}{$key});
    return $self->{_ENV}->{$key};
}


sub getConfig {
    # Return the master config file for this MOPS instance.  Cache it so we only
    # have one copy.
    my $self = shift;
    return $self->{_CONFIG};
#    if (!$self->{_CONFIG}) {
#        my $cfgfile = join '/', $self->{_ENV}->{HOMEDIR}, 'var', $self->{_DBNAME}, 'config/master.cf';
#        die "MOPS master configuration file $cfgfile not a regular file" unless -f $cfgfile;
#        require Config::Scoped;
#        my $parser = Config::Scoped->new(file => $cfgfile, warnings => {permissions => 'off'});
#        $self->{_CONFIG} = $parser->parse();
#    }

    return $self->{_CONFIG};
}


sub getLogger {
    # Return the Log4perl logging service hande for this MOPS instance.  Cache it so we only
    # have one copy.
    my $self = shift;
    my $dbname = $self->{_DBNAME};

    if (!$self->{_LOGGER}) {
        require Log::Log4perl;

        # Try to figure out how we need to set up our logger.  If we are on a cluster
        # node or under Apache, then we don't want to log to a central file on the host; we need to
        # log to the "current directory" or to a central network syslogger.
        my $logfile;
        my $logdir;

        my $cfg;
        if ($ENV{_CONDOR_SCRATCH_DIR}) {
            $cfg = <<"CFG";
############################################################
# Simple root logger with a Log::Log4perl::Appender::File
############################################################
# Default level = INFO, appenders = Screen, Log
log4perl.logger = INFO, Screen

log4perl.appender.Screen = Log::Log4perl::Appender::Screen
log4perl.appender.Screen.layout = PatternLayout
log4perl.appender.Screen.layout.ConversionPattern = %d %m %n
CFG
        }
        else {
            $logdir = "$ENV{MOPS_HOME}/var/${dbname}/log";
            $logfile = "$logdir/mops.log";
            system ('mkdir', '-p', $logdir) == 0 or die("can't create log directory $logdir");

            $cfg = <<"CFG_FILE";
############################################################
# Simple root logger with a Log::Log4perl::Appender::File
############################################################
# Default level = INFO, appenders = Screen, Log
log4perl.logger = INFO, Screen, Log

log4perl.appender.Screen = Log::Log4perl::Appender::Screen
log4perl.appender.Screen.layout = PatternLayout
log4perl.appender.Screen.layout.ConversionPattern = %d %m %n

log4perl.appender.Log = Log::Log4perl::Appender::File
log4perl.appender.Log.filename = $logfile
log4perl.appender.Log.layout = PatternLayout
log4perl.appender.Log.layout.ConversionPattern = %d %m %n
CFG_FILE
        }

        Log::Log4perl->init_once(\$cfg);
        $self->{_LOGGER} = Log::Log4perl->get_logger('');
    }

    return $self->{_LOGGER};
}


sub makeNNDir {
    # Given a processing night number and pipeline subsystem,
    # create a directory for processing in the MOPS instance's
    # directory tree.
    my $self = shift;
    my $MAX_TRIES = 999;
    my %args = validate(@_, {
        NN => 1,
        SUBSYS => 1,
        FORCE_EMPTY => 0,
    });
    my $dir = join('/', $self->getEnvironment('NNDIR'), sprintf("%05d", $args{NN}), $args{SUBSYS});

    if ($args{FORCE_EMPTY} and -d $dir) {
        # Directory exists but caller wants it empty.  So rename the existing one.
        my $newdir;
        my $tries = 0;
        while ($tries++ < $MAX_TRIES) {
            $newdir = "$dir.$tries";
            if (!-e $newdir) {
                rename $dir, $newdir or die "can't rename $dir to $newdir";     # suffixed link
                last;
            }
        }
        die "can't prevent collision with $dir" if -l $dir;
    }

    if (!-d $dir) {
        eval { mkpath($dir) };
        die "can't create $dir" if $@;
        $self->getLogger()->info("Created $dir");
    }

    return $dir;
}


sub getOldestUnfinishedField {
    # Return a field object for the oldest field which does not have
    # status FIELD_STATUS_LINKDONE.
    # An optional night number can be specified. If it is specified then
    # the field object for the oldest field on that night number is returned.
    my $self = shift;
    my $nn = shift;
    require PS::MOPS::DC::Field;
    if (defined($nn)){
    	return PS::MOPS::DC::Field::modcf_retrieve($self, unfinishedNN => $nn);
    } else {
    	return PS::MOPS::DC::Field::modcf_retrieve($self, oldestUnfinished => 1);
    } 
}


sub getLastProcessedField {
    # Return the night number of the last processed field.
    my $self = shift;
    require PS::MOPS::DC::Field;
    return PS::MOPS::DC::Field::modcf_retrieve($self, lastProcessed => 1); 
}


1;

