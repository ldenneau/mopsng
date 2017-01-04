#!/usr/bin/env perl

use strict;
use warnings;

use Getopt::Long;
use File::Find;
use File::Path;
use Cwd;

use PS::MOPS::Lib qw(:all);
use PS::MOPS::FITS::IPP;            # basic FITS I/O for IPP-MOPS
use PS::MOPS::IPPDB;                # various IPP DB poking

use subs qw(process);

my $debug;
my $no_rename;
my $help;
GetOptions(
    debug => \$debug,
    no_rename => \$no_rename,
    help => \$help,
) or pod2usage(2);
pod2usage(-verbose => 3) if $help;


our $F51_GMT_OFFSET_HOURS = -10;    # Haleakala -10 hours from GMT
our $SUBDIR = 'IPP-MOPS';           # where to find stuff
our @exposures;
our %expname2comment;

die "$SUBDIR must be in current directory.\n" unless -d $SUBDIR;

# Inside the current directory hierarchy, find FITS files for exposures
# and create links to them using the following structure:

# $OC/
#   o1234g0058o.$CHUNK_LABEL.fits
#
# If a link exists, rename it with a numeric suffix, then create the link.
print STDERR "Scanning $SUBDIR...";
find({
    wanted => \&process,
    no_chdir => 1,
}, "$SUBDIR/raw");
printf STDERR "found %d exposures.\n", scalar @exposures;

# Now we have a list of all exposures, sorted by diff ID then pub ID.
# For each file, open it, get the exposure and chunk names, and set
# up the links.
@exposures = sort { 
    ($a->{DIFF_ID} <=> $b->{DIFF_ID}) 
    || ($a->{PUB_ID} <=> $b->{PUB_ID})
} @exposures;

print STDERR "Fetching comments from exposures...";
my $idbh = PS::MOPS::IPPDB::dbh();
my $isth = $idbh->prepare('select exp_name, comment from rawExp') or die $idbh->errstr;
$isth->execute() or die $isth->errstr;
my @row;
while (@row = $isth->fetchrow_array()) {
    if ($row[1] and $row[1] ne 'NULL') {
        $expname2comment{$row[0]} = $row[1];
    }
}
$isth->finish();
printf STDERR "done.\n";

foreach my $item (@exposures) {
    make_link($item);
}

exit;


sub process {
    # $File::Find::dir is the current directory name,
    # $_ is the current filename within that directory (== $File::Find::name since no_chdir=1)
    # $File::Find::name is the complete pathname to the file.

    # Next if it's not a .mops FITS file.
    return unless -f and /\.mops$/;

    my $pub_id = 0;
    my $diff_id;
    unless (/diff\.(\d\d+)/) {
        die "Can't get diff ID for $_\n";
    }
    $diff_id = $1;
    if (/^pub\.(\d\d+)/) {
        $pub_id = $1;
    }
    push @exposures, {
        PUB_ID => $pub_id,
        DIFF_ID => $diff_id,
        FILE => $File::Find::name,
    };
}


sub make_link {
    my ($item) = @_;
    my $file = $item->{FILE};
    my $table = new PS::MOPS::FITS::IPP;
    $table->openfile($file);

    my $exp_name = $table->{_keys}->{EXP_NAME} || 'NA';
    $table->closefile();    # just need FITS headers

    die "bad EXP_NAME ($exp_name) for $file" unless $exp_name =~ /^.\d\d\d\d.\d\d\d\d.$/;
    my ($comment) = $expname2comment{$exp_name} || 'NA';
    my $survey_mode = comment2survey_mode($comment) || 'NA';

    # Got all our info, so link away. Need several:
    # $SUBDIR/$OCNUM/$exp_name => $file
    # $SUBDIR/$OCNUM/chunks/$survey_mode.$exp_name => $SUBDIR/$OCNUM/$exp_name
    my $ocnum = mopslib_mjd2ocnum($table->{_keys}->{'MJD-OBS'});
    my $nn = mopslib_mjd2nn($table->{_keys}->{'MJD-OBS'}, $F51_GMT_OFFSET_HOURS);

    # 1. Generic link $EXP_NAME => $FILE.
    my $ocpath;
    eval {
        $ocpath = "$SUBDIR/$ocnum/$nn";
        if (!-d $ocpath) {
            mkpath([$ocpath]);
        }
    };
    make_symlink($file, '../../..', $ocpath, $exp_name) or die "can't create symlink $ocpath/$exp_name to $file";
    print STDERR "Linked $file to $ocpath/$exp_name.\n";
}


sub comment2survey_mode {
    my ($comment) = @_;

    # Expect one of
    # 3PI
    #   $OC.3PI.something
    #   3pi.$OC.something
    #   dm3pi.$OC.something
    # Solar System
    #   ESS.$OC.something
    #   MSS.$OC.something
    #   OSS.something
    #   ss.$OC.something
    # Medium Deep s/\s+/./g
    #   md??
    # M31
    # STS
    # SVS
    #   svs.$OC.something
    # SAS

    if ($comment =~ /^3PI\.\d+\.\S+/) {
        return $1;
    }
    elsif ($comment =~ /^\d+\.3PI\.\S+/) {
        return $1;
    }
    elsif ($comment =~ /^\d+\.3PI\.\S+/) {
        return $1;
    }

    return '';
}


sub make_symlink {
    # Make a symlink to $file with the name $link.
    # If the link already exists, rename the link using a numeric suffix.
    my ($file, $rel, $dir, $link) = @_;
    my $newlink;
    my $tries = 0;


    my $oldwd = getcwd;
    chdir $dir or die "can't chdir to $dir";

    if ($no_rename) {
        if (!-l $link) {
            # Rel contains relative directory information to get to the root of our 
            # our raw file tree.
            symlink "$rel/$file", $link or die "can't link $link to $file";
        }
    }
    else {
        # Rename link if exists.
        if (-l $link) {
            while ($tries++ < 10) {
                $newlink = "$link.$tries";
                if (!-e $newlink) {
                    rename $link, $newlink or die "can't rename $link to $newlink";     # suffixed link
                    last;
                }
            }
            die "existing symlink $link to $file" if -l $link;
        }
        # Rel contains relative directory information to get to the root of our 
        # our raw file tree.
        symlink "$rel/$file", $link or die "can't link $link to $file";
    }

    chdir $oldwd or die "can't restore working directory $oldwd";
}
