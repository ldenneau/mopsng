#! /usr/bin/env perl

# $Id$

use lib '/home/yamada/prj/panstarrs/mops/src';
use lib '/opt/local/lib/perl5/site_perl';
use lib '/opt/local/lib/perl5/site_perl/5.8.8/mach';

my $destdir = "/usr/local/MOPS_DEV/ds/dsroot/det";

use warnings;
use strict;

use PS::MOPS::DET::cgi;

my $cgi = new PS::MOPS::DET::cgi;
$cgi->status($destdir);
