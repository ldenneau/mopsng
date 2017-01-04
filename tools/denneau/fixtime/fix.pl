#!/usr/bin/perl

use strict;
use warnings;

# Just a stub for quick debugging.
use Getopt::Long;
use Pod::Usage;
use Data::Dumper;

use PS::MOPS::DC::Instance;
use PS::MOPS::Lib qw(:all);


my $instance_name;
my $help;
GetOptions(
    'instance=s' => \$instance_name,
    help => \$help,
) or pod2usage(2);
pod2usage(-verbose => 3) if $help;

my $inst = PS::MOPS::DC::Instance->new(DBNAME => $instance_name);
my $dbh = $inst->dbh;

my $sth = $dbh->prepare("select field_id, epoch_mjd from fields where epoch_mjd between 56136 and 56241 order by epoch_mjd")
    or die $dbh->errstr;
$sth->execute() 
    or die $sth->errstr;
my $href;
my $datestr;
while ($href = $sth->fetchrow_hashref()) {
    $datestr = mopslib_mjd2utctimestr($href->{epoch_mjd});
    print <<"SQL";
/* Field ID $href->{field_id} $datestr */
update detections set epoch_mjd=$href->{epoch_mjd} where field_id=$href->{field_id};
SQL
}
