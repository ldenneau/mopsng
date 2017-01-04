package PS::MOPS::MITI;

use 5.008;
use strict;
use warnings;

use base qw(Exporter);

our @EXPORT = qw(
    miti_read
    miti_write
    miti_slurp
    miti_parse
    miti_parse_det
    miti_format
    miti_format_obsdet
    miti_format_orbit
    miti_format_det
    miti_format_hash

    $MITI_COLUMN_ID
    $MITI_COLUMN_EPOCH
    $MITI_COLUMN_RA
    $MITI_COLUMN_DEC
    $MITI_COLUMN_MAG
    $MITI_COLUMN_OBSCODE
    $MITI_COLUMN_OBJECT_NAME
);

our $VERSION = '0.01';
our $MITI_COLUMN_ID = 0;
our $MITI_COLUMN_EPOCH = 1;
our $MITI_COLUMN_RA = 2;
our $MITI_COLUMN_DEC = 3;
our $MITI_COLUMN_MAG = 4;
our $MITI_COLUMN_OBSCODE = 5;
our $MITI_COLUMN_OBJECT_NAME = 6;


# Our stuff.
sub miti_format {
    # Formats a list into a MITI format.
    my @fields = @_;
    return join(" ", @fields) . "\n";    # join list elements
}


sub miti_format_hash {
    my %item = @_;
    my @goo = grep { $_ } @item{qw( ID EPOCH_MJD RA_DEG DEC_DEG MAG OBSCODE OBJECT_NAME)};
    return join(" ", @goo) . "\n";    # join elements
}


sub miti_read {
    # Read in MITI file. pair file.  Return list of ARRAYREFs.  If filename is "-", use
    # STDIN.
    my $filename = shift;
    my $line;
    my @items;

    open my $fh, $filename or die "can't open MITI file $filename";
    while (defined($line = <$fh>)) {
        if ($line !~ /^#/) {    # ignore comments
            push @items, [split /\s+/, $line];
        }
    }
    close $fh;
    return @items;
}
*miti_slurp = \&miti_read;


sub miti_parse {
    # Parse a string into MITI fields.  Return a hash containing the keys:
    #  ID EPOCH_MJD RA_DEG DEC_DEG MAG OBSCODE OBJECT_NAME
    my $line = shift;
    my %result;
    my @items = split /\s+/, $line;     # separate tokens
    @result{qw(ID EPOCH_MJD RA_DEG DEC_DEG MAG OBSCODE OBJECT_NAME)} = @items;  # bulk assign
    return %result;
}


sub miti_parse_det {
    # Parse a MITI string into a detection-link object.  This means a
    # hashref with well-known keys.
    my $line = shift;
    my %result;
    my @items = split /\s+/, $line;     # separate tokens
    @result{qw(detId epoch ra dec mag obscode objectName)} = @items;  # bulk assign
    return \%result;
}


sub miti_write {
    # Write out MITI file.  Input is filename, then list of ARRAYREFs or HASHREFs.
    my ($filename, @data) = @_;
    open my $fh, ">$filename" or die "can't open MITI file $filename for writing";
    # Header
    print $fh <<"HEADER";
# MITI Version: $VERSION
# ID EPOCH_MJD RA_DEG DEC_DEG MAG OBSCODE OBJECT_NAME
HEADER

    foreach my $item (@data) {
        if (ref($item) eq 'HASH') {
            print $fh miti_format_hash(%$item);
        }
        else {  # assume ARRAY
            print $fh miti_format(@$item);
        }
    }
    close $fh;
}


sub miti_format_obsdet {
    my ($obs, $det, $id) = @_;
    if (!$det->objectName) {
        warn "no objectName for det ID " . $det->detid;
        return undef;
    }

    my @args = (
        $id || $det->objectName,
        $det->epoch,
        $det->ra,
        $det->dec,
        $det->mag,
        $obs->obscode,
    );
    push @args, $det->objectName if $id;    # ID went in 0th item, so append objectName
    return miti_format(@args);
}


sub miti_format_orbit {
    my ($orb) = @_;
    return join " ", @{$orb}{qw(
        q e i node argPeri timePeri hV epoch
    )};
}


sub miti_format_det {
    my ($det, $force_id) = @_;
    my @args = (
        $force_id || $det->detId,
        $det->epoch,
        $det->ra,
        $det->dec,
        $det->mag,
        $det->obscode,
        $det->objectName || 'NONE',
    );
    return miti_format(@args);
}


1;
__END__

=head1 NAME

PS::MOPS::MITI - Perl extension for manipulation of MITI (MOPS Interim
Text Interchange) files.

=head1 SYNOPSIS

  use PS::MOPS::MITI;
  my @data = miti_read($filename); # read entire file into list
  miti_write(@data, $filename);    # write data to MITI file

  my $str = miti_format(@fields);
  my $str = miti_format_hash(%fields);
  my $str = miti_format_obsdet($obs, $det); # format observation+detection
  my $str = miti_format_obsdet($obs, $det, '000001'); # format observation+detection+ID

  my %fields = miti_parse($line);           # convert line of text to MITI hash
  my $objlike = miti_parse_det($line);      # convert line of text to detection-like hashref

=head1 DESCRIPTION

Read and write MITI (MOPS Interim Text Interchange) files.  Format
fields into MITI format.  The MITI format is extremely simple:

ID EPOCH_MJD RA_DEG DEC_DEG MAG OBSCODE OBJECT_NAME

OBJECT_NAME is always optional.  Values are separated by whitespace: one or
more tab or space characters.

miti_read - read the specified filename and write out a MITI file to STDOUT.  If
the filename is "-", then read from STDIN.

miti_slurp - synonym for miti_read

miti_parse - convert a string to a hash containing MITI keys.

miti_write - write data to MITI file.  Data is an array of ARRAYREFs.

miti_format - convert array of values to MITI string

miti_format_hash - convert hash of values to MITI string

miti_format_orbit - convert MOPS Orbit or SSM hashref into whitespace-delimited MITI line

miti_format_obsdet - convert PSMOPS observation and detection objects to MITI
detection

MITI data are just Perl HASHREFs containing the keys:

    ID 
    EPOCH_MJD 
    RA_DEG 
    DEC_DEG 
    MAG 
    OBSCODE 
    OBJECT_NAME

=head1 EXPORTS

miti_read
miti_slurp
miti_parse
miti_write
miti_format
miti_format_obsdet
miti_format_orbit

=head1 AUTHOR

Larry Denneau, Jr. <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2005 by Larry Denneau, Jr., Institute for Astronomy, University
of Hawaii.

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut
