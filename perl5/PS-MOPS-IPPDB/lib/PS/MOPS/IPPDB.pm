#!/usr/bin/env perl
package PS::MOPS::IPPDB;

use strict;
use warnings;
use Carp;
use DBI;
use Astro::FITS::CFITSIO;
use Astro::Time;
use PS::MOPS::Lib;


our $VERSION = '0.01';
#    'dbi:mysql:database=gpc1;port=13306;host=127.0.0.1',
our @conspec = (
#    'dbi:mysql:database=gpc1;port=3306;host=ippdb03.ifa.hawaii.edu',
    'dbi:mysql:database=gpc1;port=3306;host=ippdb05.ifa.hawaii.edu',
#    'ippuser', 'ippuser',
    'mops', 'mops',
);

sub patch_exposures {
    my @exp_names = @_;
    my %stuff;
    my $dbh = DBI->connect(@conspec) or die "can't connect to IPP database";
    my $href;
    my $foo;
    my ($y, $m, $d, $h, $min, $s);
    my $epoch_mjd;


    # Normally we will call this with a largish batch of exp_names.  So
    # make a table of inputs that we'll match with query outputs.
    my %inp_tbl;
    foreach my $exp_name (@exp_names) {
        $inp_tbl{$exp_name} = 1;
    }

    # Now set up our query to get all known exp_names from IPP.
    my $sth = $dbh->prepare(<<"SQL") or die $dbh->errstr;
select 
    exp_name exp_name, 
    exp_id exp_id, 
    dateobs dateobs_utc, 
    exp_type exp_type, 
    ra ra_rad, 
    decl dec_rad, 
    posang pos_ang_deg, 
    exp_time exp_time_sec,
    filter filter, 
    airmass airmass, 
    comment comment
from rawExp /*where exp_name=?*/
SQL
    $sth->execute() or die $sth->errstr;
    while ($href = $sth->fetchrow_hashref()) {
        my $exp_name = $href->{exp_name} or die "got empty EXP_NAME";
        if (exists($inp_tbl{$exp_name})) {
            # Clean up some stuff.
            $href->{ra_deg} = rad2deg($href->{ra_rad});
            $href->{dec_deg} = rad2deg($href->{dec_rad});
            ($y, $m, $d, $h, $min, $s) = split /[- :]+/, $href->{dateobs_utc};
            $epoch_mjd = cal2mjd($d, $m, $y, ($h * 3600 + $min * 60 + $s) / 86400);

            # Make it look like a PS::MOPS::DC::Field;
            $stuff{$exp_name} = {
                fpaId => $href->{exp_name},
                fieldId => $href->{exp_name},
                ra => $href->{ra_deg},
                dec => $href->{dec_deg},
                epoch => $epoch_mjd,
                ocnum => PS::MOPS::Lib::mopslib_mjd2ocnum($epoch_mjd),
                pa_deg => $href->{pos_ang_deg},
                filter => $href->{filter},
                rawData => $href,
            };
        }
    }

    return \%stuff;
}


sub dbh {
    my $dbh = DBI->connect(@conspec) or die "can't connect to IPP database";
    return $dbh;
}


1;

__END__

=head1 NAME

PS::MOPS::IPPDB - Interface to IPP database for patching exposure information

=head1 SYNOPSIS

use PS::MOPS::IPPDB;
@stuff = PS::MOPS::IPPDB::patch_exposures(@input_exposure_names);

# @stuff looks like
# [
#   exp_name1 => {
#     ra_deg => RA_DEG
#     dec_deg => DEC_DEG
#     epoch_mjd => EPOCH_MJD,
#     exp_time_sec => EXP_TIME_SEC,
#     pos_ang_deg => POS_ANG_DEG,
#     comment => OTIS_COMMENT,
#     filter => FILTER,
#     airmass => AIRMASS,
#     exp_type => EXP_TYPE,
#   }
#   ...

=head1 DESCRIPTION

Module to patch IPP information into exposure names.  Given a list of
exposure name, say from IPP datastore, provide detailed metadata so that
we can decide what to do with them (skip, ingest, etc.)

=cut
