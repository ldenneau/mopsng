#!/usr/bin/perl

use strict;
use warnings;
use Proc::Daemon;
use Pod::Usage;
use Getopt::Long;
use Fcntl qw/ :DEFAULT :flock :seek /;

use PS::MOPS::Constants qw(:all);

=head1 NAME

start_mops - Execute MOPS pipline whenever ready chunks are detected.
 
=head1 SYNOPSIS

start_mops [options] [INSTANCE_NAME]

  --instance : name of simulation to monitor
  --stage STAGE : execute mopper up to the specified stage. Stages in order are:
                  SYNTH, TRACKLET, POSTTRACKLET, ATTRIB, LINKOD, EON.
                  If not specified then defaults to EON.
  --wait : time interval in minutes to wait between checks for ready chunks.
  --mops_home DIR : use DIR as MOPS home directory; otherwise $ENV{MOPS_HOME} or /usr/local/MOPS
  --help : show usage

=head1 DESCRIPTION

This program runs as a daemon and monitors a MOPS instance for fields with the
status of ready ($FIELD_STATUS_READY). When it finds fields that are ready for
processing it will then invoke mopper to process the ready fields. The interval 
between checks is given by the --wait option in minutes.. 

=cut

my $instance_name;
my $inst;
my $stage;
my $help;
my $sleep;
my $log;
my $mops_home;

# Process command line options.
GetOptions(
    'instance=s' => \$instance_name,
    'mops_home=s' => \$mops_home,
    'stage=s' => \$stage,
    'wait=f' => \$sleep,
    help => \$help,
) or pod2usage(2);
pod2usage(-verbose => 2) if $help;
$instance_name ||= shift || $ENV{MOPS_DBINSTANCE};
pod2usage(3) unless $instance_name;

# Get wait time and convert from minutes to seconds.
$sleep = 5 unless defined($sleep); # Defaults wait to 5 min if not specified.
$sleep *= 60;

# Verify that a valid stage was specified.
my %stages = (
              "SYNTH"=> \&valid_stage,
              "TRACKLET"=> \&valid_stage,
              "POSTTRACKLET"=> \&valid_stage,
              "ATTRIB"=> \&valid_stage,
              "LINKOD"=> \&valid_stage,
              "EON"=> \&valid_stage,
              "DEFAULT"=> \&invalid_stage);

$stage = uc($stage);
$stage = "DEFAULT" unless defined($stages{$stage});  
$stages{$stage}->();#Calls the subroutine that this hash value is a reference to.

setup_env($instance_name);
require PS::MOPS::DC::Instance;     # now load, after %ENV set up
$inst = PS::MOPS::DC::Instance->new(DBNAME => $instance_name);
$log = $inst->getLogger;

# Fork of a child process which will monitor the specified database. This child
# process will run as a daemon in the background. 
Proc::Daemon::Init;

# Set umask so that created files and directories are only writable by owner and group.
umask 002;

# Set up PID file and verify that there is not already an instance of start_mops
# monitoring the database.
my $PIDFILE = "/tmp/monitor_$instance_name.pid";
check_pid();


my $continue = 1;
$SIG{TERM} = sub { 
	unlink($PIDFILE); 
	$continue = 0; 
}; # Kills loop when a terminate signal is trapped.

# Loop until a terminate signal is trapped.
my $nn;
my @nights;
my $log_file = $inst->getEnvironment("VARDIR") . "/log/mops.log";
my $temp_file = $inst->getEnvironment("VARDIR") . "/log/temp.log";
while ($continue) {
    @nights = get_nn($inst);
 
    # Process each of the nights through mopper.
    foreach $nn (@nights) {
    	`mopper --no_synth --nn $nn --stage $stage $instance_name >> $log_file 2>&1`;
    	# Clean up log file by removing duplicate lines from log file. The 
    	# cleaned up log is written to a temporary file.
    	`uniq $log_file $temp_file`;
    	
    	# Delete the old log file.
    	unlink($log_file); 
    	
    	# Make the cleaned up log file the new log file
    	rename($temp_file, $log_file);
    	
    	# The log file needs to be world writable as users who are not a member
    	# of the mops group will be writing to the file.
    	chmod(0666, $log_file);
    } 
    sleep($sleep);
}

sub get_nn {
	# Queries the database for all fields that are ready for processing and 
	# returns the night number of the nights with the ready fields.
	my $inst = shift;
	my $dbh = $inst->dbh;
	my $nights = $dbh->selectcol_arrayref(<<"SQL") or die $dbh->errstr;
select distinct nn from fields 
where status = "$FIELD_STATUS_READY"
order by nn 
SQL
    return @{$nights};
}
sub valid_stage {
	return 1;
}

sub invalid_stage {
	pod2usage("\n\nStage option not provided or is not a recognized stage.
Specify SYNTH, TRACKLET, POSTTRACKLET, ATTRIB, LINKOD, or EON for --stage\n");
}

sub setup_env {
    # Set up our environment for running MOPS simulations.
    my $inst_name = shift;

    my $MOPS_VAR = "$ENV{MOPS_HOME}/var/$ENV{MOPS_DBINSTANCE}";
    _setenv('MOPS_DBINSTANCE', $inst_name);
    _setenv('MOPS_HOME', $mops_home || $ENV{MOPS_HOME} || '/usr/local/MOPS');

    _setenv('PERL5LIB', $ENV{PERL5LIB} || "$ENV{MOPS_HOME}/lib/perl5");
    _addenv('PERL5LIB', "$MOPS_VAR/lib/perl5");

    _setenv('PYTHONPATH', $ENV{PYTHONPATH} || "$ENV{MOPS_HOME}/lib/python");
    _addenv('PYTHONPATH', "$MOPS_VAR/lib/python");

    _addenv('PATH', "$ENV{MOPS_HOME}/bin");
    _addenv('PATH', "/usr/local/condor/bin");
    _addenv('PATH', "$ENV{MOPS_HOME}/var/$ENV{MOPS_DBINSTANCE}/bin");

    _addenv('LD_LIBRARY_PATH', "$ENV{MOPS_HOME}/lib");
    _addenv('LD_LIBRARY_PATH', "$MOPS_VAR/lib");

    _setenv('CAET_DATA', $ENV{CAET_DATA} || "$ENV{MOPS_HOME}/data/caet_data");
    _setenv('OORB_DATA', $ENV{OORB_DATA} || "$ENV{MOPS_HOME}/data/oorb");
    _setenv('ORBFIT_DATA', $ENV{ORBFIT_DATA} || "$ENV{MOPS_HOME}/data/orbfit");
}

sub _setenv {
    my ($k, $v) = @_;
    $ENV{$k} = $v;
}

sub _addenv {
    # Add the specified item to the environment PATH-style.
    # First check to see if the thing we're adding is already there; if so, don't
    # do anything.
    my ($k, $v) = @_;
    my $re = qr/$v/;            # save regexp

    if (!$ENV{$k}) {
        $ENV{$k} = $v;          # wasn't set, so set it
    }
    else {
        my @stuff = split /:/, $ENV{$k};
        foreach my $item (@stuff) {
            return if $item =~ $re;     # found it, so bail
        }
        $ENV{$k} = join ':', $v, @stuff;
    }
}

sub check_pid {
    # Opens\creates a pid file. 
    sysopen my $fh, $PIDFILE, O_RDWR | O_CREAT or die "$0: open $PIDFILE: $!";
    flock $fh => LOCK_EX                       or die "$0: flock $PIDFILE: $!";

    # Retrieve the pid from the file
    my $pid = <$fh>;
    if (defined $pid) {
    	# File contained a pid and it was sucessfully read.
        chomp $pid;
        if (kill 0 => $pid) {
        	# The pid contained in the file is an active pid which means that
        	# the simulation is already being monitor. Exit.
            close $fh;
            $log->logdie("START_MOPS: Found an already running instance of start_mops that is monitoring $instance_name! Exiting.\n");
            exit 1;
        }
    }
    else {
    	# Die if an error occured while reading the file.
        die "$0: readline $PIDFILE: $!" if $!;
    }

    # Clear the contents of the PID file and write the pid of the current
    # process to the file.
    sysseek  $fh, 0, SEEK_SET or die "$0: sysseek $PIDFILE: $!";
    truncate $fh, 0           or die "$0: truncate $PIDFILE: $!";
    print    $fh "$$\n"       or die "$0: print $PIDFILE: $!";
    close    $fh              or die "$0: close: $!";
}
