package PS::MOPS::Config;

use 5.008;
use strict;
use warnings;

use base qw(Exporter);
our $VERSION = '0.01';

use FileHandle;



our $_COMMENT = qr'#.*$';
our $_LEADING_TRAILING_WHITESPACE = qr'^\s+|\s+$'; 

our $_RE_START_DICT = qr'\w+(\s*\{|\s*=\s*\{)$';
our $_RE_END_DICT = qr'\}';

our $_RE_START_LIST = qr'\w+(\s*\[|\s*=\s*\[)$';
our $_RE_END_LIST = qr'\]';

our $_RE_KEY_VAL = qr'\w+\s*=\s*.+';
our $_RE_TRAILING_STUFF = qr'[,;]$';
our $_RE_QUOTES = qr'(^[\'"]|[\'"]$)';

our $_RE_INT = qr'^(\+|-)?[0-9]+$';
our $_RE_FLOAT = qr'^(\+|-)?((\.[0-9]+)|([0-9]+(\.[0-9]*)?))$';
our $_RE_DOUBLE = qr'^(\+|-)?((\.[0-9]+)|([0-9]+(\.[0-9]*)?))([eE](\+|-)?[0-9]+)?$';


sub _clean_val {
    my ($val_str) = @_;
    # Clean up value by
    # * removing trailing , or ;
    # * converting to float if looks like a float/double
    # * converting to int if looks like int
    # Return undef if resulting string is empty
    my $val;

    $val_str =~ s/$_LEADING_TRAILING_WHITESPACE//g;
    $val_str =~ s/$_RE_TRAILING_STUFF//g;   # clean up trailing comma and semicolon
#    if ($val_str =~ $_RE_INT) {
#        $val = int($val_str);
#    }
#    elsif ($val_str =~ $_RE_FLOAT or $val_str =~ $_RE_DOUBLE) {
#        $val = float($val_str);
#    }
#    else {
    # In Perl we don't have to convert.  So just clean up the string.
    {
        $val_str =~ s/$_RE_QUOTES//g;
        $val = $val_str;
    }
    return $val;
}


sub _clean_line {
    my $s = $_[0];
    $s =~ s/$_LEADING_TRAILING_WHITESPACE//g;
    $s =~ s/$_COMMENT//;
    return $s;
}


sub _parse_list_val {
    my ($line) = @_;
    return _clean_val($line);
}


sub _parse_key_val {
    my ($line) = @_;
    my ($key, $val) = split '=', $line, 2;
    $key =~ s/$_LEADING_TRAILING_WHITESPACE//g;
    $val = _clean_val($val);
    return ($key, $val);
}


sub _process_list {
    my ($lref) = @_;            # ref to current list of lines we're processing
    my $aref = [];
    my $line;
    my ($k, $v);

    while (@{$lref}) {
        $line = _clean_line(shift @{$lref});
        next unless $line;

        # Hide any quoted stuff, so that dict and line delimiters in
        # quoted strings don't confuse our "parser".
        my $cooked = $line;
        $cooked =~ s/'.*'/'XXX'/;        # hide single-quoted
        $cooked =~ s/".*"/'XXX'/;        # hide double-quoted

        if ($cooked =~ $_RE_START_DICT) {
            ($k) = split /\s+/, $line;
            push @{$aref}, _process_dict($lref);   # anonymous hash
        }
        elsif ($cooked =~ $_RE_START_LIST) {
            ($k) = split /\s+/, $line;
            push @{$aref}, _process_list($lref);   # anonymous list
        }
        elsif ($cooked =~ $_RE_END_LIST) {
            last;
        }
        elsif ($cooked =~ $_RE_END_DICT) {
            die "end of dict found while expecting end of list";
        }
        else {
            push @{$aref}, _parse_list_val($line);
        }
    }

    return $aref;
}


sub _process_dict {
    my ($lref) = @_;            # ref to current list of lines we're processing
    my $href = {};
    my $line;
    my ($k, $v);

    while (@{$lref}) {
        $line = _clean_line(shift @{$lref});
        next unless $line;

        # Hide any quoted stuff, so that dict and line delimiters in
        # quoted strings don't confuse our "parser".
        my $cooked = $line;
        $cooked =~ s/'.*'/'XXX'/;        # hide single-quoted
        $cooked =~ s/".*"/'XXX'/;        # hide double-quoted

        if ($cooked =~ $_RE_START_DICT) {
            ($k) = split /\s+/, $line;
            $href->{$k} = _process_dict($lref);
        }
        elsif ($cooked =~ $_RE_START_LIST) {
            ($k) = split /\s+/, $line;
            $href->{$k} = _process_list($lref);
        }
        elsif ($cooked =~ $_RE_END_DICT) {
            last;
        }
        elsif ($cooked =~ $_RE_END_LIST) {
            die "end of list found while expecting end of dict";
        }
        else {
            ($k, $v) = _parse_key_val($line);
            $href->{$k} = $v;
        }
    }

    return $href;
}


sub LoadFile {
    my ($filename) = @_;
    my ($fh) = new FileHandle($filename);
    my @lines = <$fh>;
    chomp @lines;
    return bless _process_dict(\@lines);
}


sub new {
    # Return a HASHREF from a MOPS global config file.  Parse the file using our own parser.
    my ($pkg, $which) = @_;
    my $cfgfile = "$ENV{MOPS_HOME}/config/${which}.cf";
    if ($ENV{MOPS_DBINSTANCE}) {
        my $local_cfgfile = "$ENV{MOPS_HOME}/var/$ENV{MOPS_DBINSTANCE}/config/${which}.cf";
        $cfgfile = $local_cfgfile if -e $local_cfgfile;     # local exists; use it
    }
    die "can't load configuration file $cfgfile" unless -f $cfgfile;
    return LoadFile($cfgfile);
}


sub new_from_string {
    # Return a HASHREF from a MOPS global config file.  Parse the file using our own parser.
    my ($pkg, $text) = @_;
    my @lines = split /\n/, $text;
    return bless _process_dict(\@lines);
}


1;
__END__

=head1 NAME

PS::MOPS::Config - Perl extension to read global MOPS config files

=head1 SYNOPSIS

  use PS::MOPS::Config;
  $backend_cfg = PS::MOPS::Config->new('backend');  # reads backend.cf
  $cluster_cfg = PS::MOPS::Config->new('cluster');  # reads cluster.cf

=head1 DESCRIPTION

Reads the specified MOPS global database configuration file into a hashref
and returns it.  Normally looks in $ENV{MOPS_HOME}/config.

=head1 AUTHOR

Larry Denneau, Jr., <lt>denneau@ifa.hawaii.edu<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2006 by Larry Denneau, Jr., Institute for Astronomy,
University of Hawaii.

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.7 or,
at your option, any later version of Perl 5 you may have available.

=cut
