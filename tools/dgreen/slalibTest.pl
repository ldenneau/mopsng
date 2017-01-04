#!/usr/bin/perl

use strict;
use Astro::SLA;

my @mjd = (56109.0, 54832.0, 53736.0, 51179.0, 50630.0, 50083.0);

foreach my $mjd (@mjd){
    print "For $mjd, TAI-UTC = " . Astro::SLA::slaDat($mjd)."\n";
}