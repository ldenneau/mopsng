#!/usr/bin/env perl
#
# DataStore HTTP interface simulator
#
# The main purpose of this script is to implement the time-registered
# constraint on product lists. 
#
# To work the httpd must be configured similarly to below:
#
# <Directory "/ds/dsroot">
#   <IfModule dir_module>
#     DirectoryIndex /ds-cgi/dsindex.cgi
#   </IfModule>
# 
#   Options None
#   AllowOverride None
# 
#   Order allow,deny
#   Allow from all
# </Directory>
# 
# <Directory "/ds/cgi">
#   Options ExecCGI
# 
#   Order allow,deny
#   Allow from all
# </Directory>
# 
# <IfModule alias_module>
#   Alias /ds/ /ds/dsroot/
#   ScriptAlias /ds-cgi/ /ds/cgi/
# </IfModule>
                          

# root of the datastore directory structure
# items in the datastore have the form /product/fileset/file
# a file named "index" contains the metadata under each product/fileset dir
my $DS_DIR = '../dsroot';

#####

use strict;
use CGI ':standard';
use DateTime;

my $uri = $ENV{'REQUEST_URI'};

my $sincefs; # constraint param
my $ppflag; # pretty print flag

my @path; # requested file path


# parse request; check constraint parameter

if ($uri =~ /\?/) {
    (my $left, my $param) = split(/\?/, $uri);

    # Chuck trailing 'index' or 'index.txt' on URI.
    $left =~ s/index\.txt$//;
    $left =~ s/index$//;
    @path = split(/\//, $left);
    
	# parse a '+' to indicate pretty printing
	if ($param =~ /\+$/) {
        $ppflag = 1;

		# strip the +
		chop $param;

		$sincefs = $param if $param ne '';
    }

    else {
        $sincefs = $param;
    }
}

else {
    @path = split(/\//, $uri);
}


#
# @path is an array of tokens split on /
# note uri always starts with /, so $path[0] = ''
# $sincefs is the constraint parameter, if any
# $ppflag is whether to output html instead of text
#


# root query; list products

if ($#path == 1) {
    my $opfile1 = "$DS_DIR/index";
    my $opfile2 = "$DS_DIR/index.txt";

    open(IN, $opfile1) or open(IN, $opfile2) or exit404("root index");

	if (defined($ppflag)) {
        pprint_pre();
	    pprint($_) while (<IN>);
       	pprint_post();
	}

	else {
	    print header('text/plain', '200 OK');
	    print while (<IN>);
	}

    close(IN);
    exit;
}


# check next param; should be a valid product

if (stat("$DS_DIR/$path[2]") == undef) {
    exit404("product dir");
}


# product query; list filesets

if ($#path == 2) {
    my $opfile1 = "$DS_DIR/$path[2]/index";
    my $opfile2 = "$DS_DIR/$path[2]/index.txt";

    open(IN, $opfile1) or open(IN, $opfile2) or exit404("product index");


    # constraint given; selectively print newer filesets

    if (defined($sincefs)) {
        my $sincets;
        
        # find timestamp of the given fileset
        #XXX here we assume it's not sorted, but it probably will be
        while (<IN>) {
            next if /^\s*\#/;
        
            my ($fs, $ts) = split(/\|/, $_, 2);

            $fs =~ s/^\s+|\s+$//;
    
            # found the fileset
            if ($fs eq $sincefs) {
                # parse into datetime object and stop searching
                $sincets = parse_dt($ts);
                last;
            }
        }

        # bail if the fileset wasn't in the index
        unless (defined($sincets)) {
#            print header('text/html', '410 Gone');
#            print h1("410 Gone");
            seek(IN, 0, 0);
            if ($ppflag) {
                pprint_pre();
                pprint($_) while (<IN>);
                pprint_post();
            }
            else {
                print header('text/plain', '200 OK');
                print while (<IN>);
            }
            close(IN);
            exit;
        }

		# prepare for output
		if (defined($ppflag)) {
	        pprint_pre();
		}

		else {
			print header('text/plain', '200 OK');
		}

        # re-read the file buffer
        seek(IN, 0, 0);
        while (<IN>) {
			# don't try to parse comments
            if (/^\s*\#/) {
				if (defined($ppflag)) {
	                pprint($_);
				}

				else {
					print;
				}

                next;
            }
                
            my ($fs, $ts) = split(/\|/, $_, 2);

		    # print if timestamp is newer
            if (parse_dt($ts) gt $sincets) {
			 	if (defined($ppflag)) {
				    pprint($_);
				}

				else {
				    print;
				}
            }
        }

		pprint_post() if (defined($ppflag));
    }


    # no constraint given; print the whole index

    else {
 		if (defined($ppflag)) {
            pprint_pre();
		    pprint($_) while (<IN>);
            pprint_post();
		}

		else {
		    print header('text/plain', '200 OK');
		    print while (<IN>);
		}
    }
    
    close(IN);
    exit;
}


# check next param; should be valid fileset

if (stat("$DS_DIR/$path[2]/$path[3]") == undef) {
    exit404("fileset dir")
}


# fileset query; list files

if ($#path == 3) {
    my $opfile1 = "$DS_DIR/$path[2]/$path[3]/index";
    my $opfile2 = "$DS_DIR/$path[2]/$path[3]/index.txt";

    open(IN, $opfile1) or open(IN, $opfile2) or exit404("fileset index: $uri : " . join('|', @path));

 	if (defined($ppflag)) {
        pprint_pre();
		pprint($_) while (<IN>);
        pprint_post();
	}

	else {
		print header('text/plain', '200 OK');
		print while (<IN>);
	}

    close(IN);
    exit;
}


# if a file was requested and it existed, this script would not have been called
# so error here

exit404("file");


#
# quick hacky pretty printer
#

sub pprint_pre {
    print header('text/html', '200 OK');
    print start_html('Datastore');
    print '<table>';
}


sub pprint_post {
    print '</table>';
    print p();

	if ($#path > 1) {
		my $up = join('/', @path[0..$#path-1]);
	    print a({-href=>"$up/?+"}, "Back to $up");
	}

    print end_html();
}


sub pprint {
    my $line = shift;

    my $nolink;
    my $celltag = 'td';

	# assume a comment line means its the header
    if ($line =~ /^\s*#/) {
        $celltag = 'th';
        $nolink = 1;
    }

    my @toks = split(/\|/, $line);

	# assumes id is always first field
    my $href = join('/',@path)."/$toks[0]?+";

    print '<tr>';

    print "<$celltag>";

    if ($nolink) {
       print $toks[0];
    }

    else {
        print a({-href=>$href}, $toks[0]);
    }

    print "</$celltag>";

    foreach my $val (@toks[1..$#toks]) {
		print "<$celltag>";
		print '&nbsp;&nbsp;&nbsp;';
		print $val;
		print "</$celltag>";
    }

    print '</tr>';
}

#
# util
#

sub exit404 {
    my $arg = shift;

    print header('text/html','404 File Not Found');
    print start_html("404 Not Found");
    print h1('Not Found');
    print "($arg)" if defined($arg);
    print end_html();
    exit;
}


sub parse_dt {
    my $str = shift;

    $str =~ /(\d{4})-(\d\d)-(\d\d)T(\d\d):(\d\d):(\d\d)Z/;

    return DateTime->new(
        year    => $1,
        month   => $2,
        day     => $3,
        hour    => $4,
        minute  => $5,
        second  => $6,
        );
}
