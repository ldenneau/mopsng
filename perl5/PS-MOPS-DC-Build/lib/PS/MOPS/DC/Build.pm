package PS::MOPS::DC::Build;

use warnings;
use strict;

use base qw(Exporter);

our $VERSION = '0.01';
our @EXPORT = qw(
    modcb_build 
    modcb_remove
);

use Carp;
use Data::Dumper;
use Params::Validate;
use File::Slurp;
use Cwd;
use PS::MOPS::Config;

our $INSERT_SYNTHETIC_FIELDS = 'insertSyntheticFields';
our $CAT_ORBITS = 'catOrbits';
our $GEN_SHAPE = 'genshape.pl';
#our $mops_backend = PS::MOPS::Config->new('backend');

=head1 NAME

PS::MOPS::DC::Build - Build PSMOPS instances for MOPS processing

=head1 VERSION

Version 0.01

=head1 SYNOPSIS

    use PS::MOPS::DC::Build;

    my $results = modcb_build(  
        name => 'psmops_test',
        model => '1/100',
    );

=head1 DESCRIPTION

PS::MOPS::DC::Build is a front-end for creating PSMOPS instances for
MOPS processing and testing.

The return value is a hashref describing the built model.
PS::MOPS::DC::Test uses this information to set up environment variables
for on-demand PSMOPS instance creation for testing.

=head1 EXPORT

modcb_build()

=head1 FUNCTIONS

=head2 modcb_build

Build a PSMOPS instance, given at least a NAME.

=cut


sub _cmd {
    # Run a system command.
    my ($cmd, %flags) = @_;
    my $rv;
    $rv = system($cmd);
    
    if ($flags{WARN}) {
        $rv == 0 or warn "$cmd failed: $?";
    }
    else {
        $rv == 0 or die "$cmd failed: $?";
    }
}


sub _mysql_root_cmd {
    # Run a command as mysql root.
    my ($backend, $cmd) = @_;
    my $dbhost = $backend->{hostname} or croak "can't get DB hostname";
    my $dbport = $backend->{port} || 3306;
    my $dbuser = $backend->{rootuser} || "mopsroot";

    # Look for root pw in user's home dir.
    my $root_pwfile = "$ENV{HOME}/.psmops_rootpw";
    my $upw = (-f $root_pwfile ? read_file($root_pwfile) : "");
    chomp $upw;

    my $pw = $ENV{MOPS_ROOT_PASSWORD} || $upw || "";
    print STDERR "Executing mysql root command.  " unless $pw;
    system(qq{echo "$cmd" | mysql -u$dbuser -p$pw -h$dbhost -P$dbport}) == 0 
        or warn "$cmd failed: $?";
}


sub _mysql_cmd {
    # Run a mysql command as the mops user.
    my ($backend, $cmd) = @_;
    my $dbhost = $backend->{hostname} or croak "can't get DB hostname";
    my $dbport = $backend->{port} || 3306;
    my $mops_user = $backend->{username} or croak "can't get DB username";
    my $mops_pass = $backend->{password} or croak "can't get DB password";
    system("echo '$cmd' | mysql -u$mops_user -p$mops_pass -h$dbhost -P$dbport") == 0 
        or warn "$cmd failed: $?";
}


sub modcb_build {
    my %args = validate(@_, {
        name => 1,      # name of model, e.g. 'psmops_100'
        model => 1,     # SSM model to use '1/100', '1/1000', 'test', 'larry', whatever
        shape => 0,     # whether to insert shape defs
        control => 0,   # load MOPS control orbits
        run => 0,       # how much MOPS processing to perform
        cleanup => 0,   # cleanup if error
        description => 0, # description of simulation
        quiet => 0,     # quiet output (only for errors)
        noprompt => 0,  # don't prompt (dangerous!)
    });

    croak "invalid \$MOPS_HOME ($ENV{MOPS_HOME})" unless $ENV{MOPS_HOME} && -d $ENV{MOPS_HOME};

    my %res;            # summary of what we ded
    my $INSTANCE_DIR = "$ENV{MOPS_HOME}/var/$args{name}";

    eval {
        my $mops_backend = PS::MOPS::Config->new('backend');
        croak "can't get backend config" unless $mops_backend;

        # Warn user, unless $args{quiet}.
#        unless ($args{noprompt}) {
#            print STDERR "About to DELETE $args{name}, Enter to continue...";
#            my $dummy = <>; # wait for Enter
#        }

        # Toss old instance.
##        eval {
##            _mysql_root_cmd("drop database $args{name}") or croak "drop $args{name} failed";
##        };
##        warn $@ if $@;  # can fail; DB might not exist
        if (-d $INSTANCE_DIR) {
#            print STDERR "Removing old $INSTANCE_DIR";
            _cmd("rm -rf $INSTANCE_DIR");
        }
       
        # Create new instance.
        $ENV{MOPS_DBINSTANCE} = $args{name};
        my $oldwd = getcwd();
        chdir "$ENV{MOPS_HOME}/schema" or croak "can't chdir to $ENV{MOPS_HOME}/schema";
        
        my $quiet_str = $args{quiet} ? '--quiet' : '';
        _cmd("./create_psmops $quiet_str $args{name}");
        chdir $oldwd or croak "can't restore cwd to $oldwd";

        # Update master configuration and description.
        _cmd("mkdir -p $INSTANCE_DIR/run");
        _cmd("mkdir -p $INSTANCE_DIR/config");
        write_file("$INSTANCE_DIR/config/description", $args{description} || $args{name});
        _cmd("cp $ENV{MOPS_HOME}/config/master.cf $INSTANCE_DIR/config/master.cf");
        _cmd("cp $ENV{MOPS_HOME}/config/orbfit.opt $INSTANCE_DIR/config/orbfit.opt");
        _cmd("cp $ENV{MOPS_HOME}/config/version $INSTANCE_DIR/config/version");

        # Set up per-instance bin dir for extreme testing.
        _cmd("mkdir -p $INSTANCE_DIR/bin");

        # Set up log dir.
        _cmd("mkdir -p $INSTANCE_DIR/log");
        for my $logfile (qw(mops.log)) {
            write_file("$INSTANCE_DIR/log/$logfile", "");
            chmod 0666, "$INSTANCE_DIR/log/$logfile" or croak "can't chdir log file $logfile";
        }

        # Insert synthetic orbits.
        if ($args{model}) {
            $oldwd = getcwd();

            # Insert control orbits.
            if ($args{control}) {
                chdir "$ENV{MOPS_HOME}/data/ssm/orbits/CONTROL" or croak "can't chdir to CONTROL dir";
                _cmd("/bin/rm done*", WARN => 1);
                _cmd("make insert");
            }

            # Insert Solar System model.
            chdir "$ENV{MOPS_HOME}/data/ssm/orbits/$args{model}" or croak "can't chdir to model dir";
            _cmd("/bin/rm done*", WARN => 1);
            _cmd("make insert");

            chdir $oldwd or croak "can't restore cwd to $oldwd";
        }

        # Insert shapes.
        if ($args{shape}) {
            _cmd("$CAT_ORBITS --ssm | $GEN_SHAPE --insert -");
        }

        %res = (
            name => $args{name},
            instance_dir => $INSTANCE_DIR,
        );

        # XXX Init datastore for this sim.  Clean this up ASAP!  In the future we should use an
        # datastore administrative program.
        my $datastore_index = "$ENV{MOPS_HOME}/ds/dsroot/det/index";
        my $datastore_dir = "$ENV{MOPS_HOME}/ds/dsroot/det/$args{name}";
        if (-d $datastore_dir and -f $datastore_index) {
            _cmd("/bin/rm -rf $datastore_dir");
            _cmd("grep -v '$args{name}' $datastore_index > /tmp/modcb.$$; mv /tmp/modcb.$$ $datastore_index");
        }

        # Low-significance detection (LSD) archive.
        my $lowsig_dir = "$ENV{MOPS_HOME}/lsd/$args{name}";
        if (-d $lowsig_dir) {
            _cmd("/bin/rm -rf $lowsig_dir");    # old directory exists, toss it
        }
        _cmd("/bin/mkdir -p $lowsig_dir");
        _cmd("/bin/ln -s $lowsig_dir $INSTANCE_DIR/lsd");

    };
    if ($@) {
        warn $@;
        %res = ();      # empty it

        if ($args{cleanup}) {
            # Mop up database.


            # Removed directories.
            if ($INSTANCE_DIR and -d $INSTANCE_DIR) {
                _cmd("rm -rf $INSTANCE_DIR");
            }
        }
    }

    return \%res;
}

=head2 modcb_remove

Remove a PSMOPS instance.

=cut

sub modcb_remove {
    my %args = validate(@_, {
        name => 1,
        quiet => 0,
        backend => 0
    });
    croak "invalid \$MOPS_HOME ($ENV{MOPS_HOME})" unless $ENV{MOPS_HOME} && -d $ENV{MOPS_HOME};

    my $INSTANCE_DIR = "$ENV{MOPS_HOME}/var/$args{name}";

    my $mops_backend;
    if(defined $args{backend}){
        $mops_backend = $args{backend};
    }
    else {
        $mops_backend = PS::MOPS::Config->new('backend');
    }

    croak "can't get $mops_backend" unless $mops_backend;
#    print Dumper($mops_backend);

    # Drop database.
    eval {
        _mysql_root_cmd($mops_backend, "use mysql; delete from db where Db='$args{name}'") or croak "revoke privs failed";
        _mysql_root_cmd($mops_backend, "drop database $args{name}") or croak "drop $args{name} failed";
        _mysql_root_cmd($mops_backend, "flush privileges") or croak "drop $args{name} failed";
    };
    warn $@ if $@;  # can fail; DB might not exist
    if (-d $INSTANCE_DIR) {
        _cmd("rm -rf $INSTANCE_DIR");
    }

    # Clean up datastore for this simulation.
#    my $datastore_index = "$ENV{MOPS_HOME}/ds/dsroot/index";
    my $datastore_dir = "$ENV{MOPS_HOME}/ds/dsroot/$args{name}";
    if (-d $datastore_dir) {
        _cmd("/bin/rm -rf $datastore_dir");
    }

    my $lowsig_dir = "$ENV{MOPS_HOME}/lsd/$args{name}";
    if (-d $lowsig_dir) {
        _cmd("/bin/rm -rf $lowsig_dir");
    }

    1;
}


=head1 AUTHOR

Larry Denneau, Jr., C<< <denneau@ifa.hawaii.edu> >>

=head1 COPYRIGHT & LICENSE

Copyright 2005 Larry Denneau, Jr., all rights reserved.

This program is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.

=cut

1; # End of PS::MOPS::DC::Build
