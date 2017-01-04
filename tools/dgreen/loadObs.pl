#!/usr/bin/perl -w

use strict;
use warnings;
use English;

use Getopt::Long;
use Pod::Usage;
use File::Slurp;
use Astro::Time;
use Fcntl qw(:seek);

use PS::MOPS::Lib qw(:all);
use PS::MOPS::Constants qw(:all);
use PS::MOPS::DC::Instance;
use PS::MOPS::DET::util qw(:all);
use PS::MOPS::DC::Field;
use PS::MOPS::DC::Detection;


my $DET_SUFFIX = "desfixed";
my $POSITION_ERROR_DEGREES = .3 / 3600.0;     # .3 arcsec, in degrees
my $LIMITING_MAG = 21.5; # made up number
my @default_de = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0);


my $instance_name;
my ($opt_debug, $opt_help, $opt_inst, $opt_det_dir, $opt_pf);
GetOptions(
    'inst=s' => \$opt_inst,         # database instance
    'pf=s' => \$opt_pf,             # pointing file
    'dir=s' => \$opt_det_dir,       # detections directiory
    'debug!' => \$opt_debug,
    'help!' => \$opt_help,
) or pod2usage(-verbose => 1) && exit;
pod2usage(-verbose => 1) && exit if defined $opt_help;

my $inst = PS::MOPS::DC::Instance->new(DBNAME => $opt_inst);
my $logger = $inst->getLogger();
my $dbh = $inst->dbh;


# Open telescope pointing file
if (!-r $opt_pf) {
    $logger->logdie("$opt_pf is not a readable file!");
}  
open(POINTINGFILE, "<", $opt_pf) or 
    $logger->logdie("Couldn't open $opt_pf for reading: $OS_ERROR\n");

# Verify that detection directory if valid and readable
if (!-d $opt_det_dir) {
    $logger->logdie("$opt_det_dir is not a directory!");
}

if (!-r $opt_det_dir) {
    $logger->logdie("$opt_det_dir is not readable.");
}

# Process contents of telescope pointing file.
my ($det_name, $ra_hr, $ra_min, $ra_sec, $dec_deg, $dec_min, $dec_sec);
my ($ra, $dec);
my ($obs_year, $obs_month, $obs_day, $obs_fraction, $obscode);
my ($mjd, $ocnum, $nn);
my ($field, $detection);
my $det_file;
my $mag;

while(<POINTINGFILE>) {
    chomp;  # remove terminating new line
    # process line read
    ($det_name, $ra_hr, $ra_min, $ra_sec, $dec_deg, $dec_min, $dec_sec) = split;
    
    # convert pointing coordinates to decimal degrees
    $ra = Astro::Time::str2deg("$ra_hr $ra_min $ra_sec", "H");
    $dec = Astro::Time::str2deg("$dec_deg $dec_min $dec_sec", "D");
    
    # open associated detection file
    $det_file = "$det_name.$DET_SUFFIX";
    open(DETFILE, "<", "$opt_det_dir/$det_file") or
        $logger->logdie("Couldn't open $opt_det_dir/$det_file for reading:  $OS_ERROR\n");
        
    # get observation time of detection
    my $in = <DETFILE>;
    $obs_year = substr($in, 15, 4);
    $obs_month = substr($in, 20, 2);
    $obs_day = substr($in, 23, 2);
    $obs_fraction = substr($in, 25, 7);
    $obscode = substr($in, 77, 3);
    
    # convert observation time to the modified julian date format
    $mjd = cal2mjd($obs_day, $obs_month, $obs_year, $obs_fraction);
        
    # insert pointing into fields table.
    $field = PS::MOPS::DC::Field->new(
        $inst,
        epoch => $mjd,
        ra => $ra,
        dec => $dec,
        timeStart => ($mjd - 5 / (24 * 60 * 60)),
        timeStop => ($mjd + 5 / (24 * 60 * 60)),
        filter => 'V',
        surveyMode => 'MITLL',
        limitingMag => $LIMITING_MAG,
        raSigma => 0.0005,
        decSigma => 0.0005,
        obscode => $obscode,
        xyidxSize => 101,
        de => \@default_de
    );
    $field->insert();
    
    # get detections associated with the field
    seek(DETFILE, 0, SEEK_SET);     #reset file ptr to start of file
    while(<DETFILE>) {
        chomp;
        $obs_year = substr($_, 15, 4);
        $obs_month = substr($_, 20, 2);
        $obs_day = substr($_, 23, 2);
        $obs_fraction = substr($_, 25, 7);
        $ra_hr = substr($_, 32, 2);
        $ra_min = substr($_, 35, 2); 
        $ra_sec = substr($_, 38, 6);
        $dec_deg = substr($_, 45, 3);
        $dec_min = substr($_, 48, 2);
        $dec_sec = substr($_, 51, 5);
        $mag = substr($_, 65, 5);
        $obscode = substr($_, 77, 3);

        # convert observation time to the modified julian date format
        $mjd = cal2mjd($obs_day, $obs_month, $obs_year, $obs_fraction);
        
        # convert detection coordinates to decimal degrees
        $ra = Astro::Time::str2deg("$ra_hr $ra_min $ra_sec", "H");
        $dec = Astro::Time::str2deg("$dec_deg $dec_min $dec_sec", "D");
        
        # insert detections into the detections table
        $detection = PS::MOPS::DC::Detection->new(
            $inst,
            epoch => $mjd,
            ra => $ra,
            dec => $dec,
            raSigma => 0.0005,
            decSigma => 0.0005,
            mag => $mag,
            refMag => $mag,
            filter => 'V',
            s2n => '10.0',
            obscode => $obscode,
            isSynthetic => 0,
            magSigma => 0.10,
            detNum => 0
        );
        $detection->addToField($field);
    }
}

# Update the parent_id column in the fields table with the field_id of the
# master field which is the last field in the quad.
#$dbh->do(<<"SQL",undef, $obscode);
#update fields as f
#inner join (
#    /* Create a table that only contains the last imaged fields of all quads.  */
#    /* select the row that corresponds to the last imaged field (>epoch_mjd)   */
#    /* and place it into an in memory temporary table .                        */
#    /* This temporary table contains the columns:                              */
#    /*      field_id    ra_deg      dec_deg     epoch_mjd    nn                */
#    select f.field_id, f.ra_deg, f.dec_deg, f.epoch_mjd, f.nn
#    from fields as f inner join 
#    (
#        /* Create a table that lists for each distinct ra_deg, dec_deg pair    */
#        /* the time at which it was last imaged.                               */
#       select ra_deg, dec_deg, max(epoch_mjd) as latest, nn  
#       from psmops_dgreen_mitll.fields 
#        group by ra_deg, dec_deg, nn
#   ) as temp 
#   using (ra_deg, dec_deg)
#   where f.epoch_mjd = temp.latest 
#) as t
#using (ra_deg, dec_deg, nn)
#set f.parent_id = t.field_id
#where f.epoch_mjd != t.epoch_mjd and
#f.parent_id is null and
#f.obscode = ?
#SQL
