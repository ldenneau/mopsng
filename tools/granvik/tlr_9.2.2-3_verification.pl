#!/usr/bin/env perl

use strict;

use warnings;

#use Pod::Usage;             # POD documentation => usage message
use Getopt::Long;           # command-line options handline
#use FileHandle;             # better file descriptors

#use Algorithm::CurveFit;

my $mopsbindir = "/usr/local/MOPS/bin";
my $datafile;
my $instance_name;
my $help;
my $verbose;

GetOptions(
   'instance=s' => \$instance_name,
   'datafile=s' => \$datafile,
   'verbose' => \$verbose,
	   );
pod2usage(-message => "INSTANCE is required.\n") unless $instance_name;
pod2usage(-message => "DATAFILE is required.\n") unless $datafile;

# Find start and end epochs for the computation
my $mjd_begin;
my $mjd_end;
my @sqlout;
@sqlout = `/bin/echo \'select min(epoch_mjd) from \`fields\`\' | /usr/bin/mysql -umops -pmops -hschmopc01 $instance_name`;
$mjd_begin = int($sqlout[1]);
# Same as above but for end date
@sqlout = `/bin/echo \'select max(epoch_mjd) from \`fields\`\' | /usr/bin/mysql -umops -pmops -hschmopc01 $instance_name`;
$mjd_end = int($sqlout[1]);

my $nderobj;
my $nvisobj;
my $mjd_tmp = $mjd_begin;
while ($mjd_tmp < $mjd_end) {
    @sqlout = `/bin/echo \'select * from history h, \`fields\` f where h.event_type = \"D\" and h.field_id = f.field_id and f.epoch_mjd < $mjd_tmp group by h.ssm_id\' | /usr/bin/mysql -umops -pmops -hschmopc01 $instance_name | grep -c ''`;
    $nderobj = int($sqlout[0]) - 1;
    if ($nderobj < 0) {
	$nderobj = 0;
    }
    @sqlout = `/bin/echo \'select count(*) from ssm where first_visible < $mjd_tmp\' | /usr/bin/mysql -umops -pmops -hschmopc01 $instance_name`;
    $nvisobj = int($sqlout[1]);
    if ($nderobj != 0 and $nvisobj != 0) {
	system "/bin/echo \"$mjd_tmp $nderobj $nvisobj\" >> $datafile";
    }
    $mjd_tmp++;
}

# Return detection ratio corresponding to the last observation date.
# Detection ratio for a full-length mission has to be extrapolated
# (this not currently done).
print $nderobj/$nvisobj;

exit;
