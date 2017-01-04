package PS::MOPS::LSD;

use 5.008008;
use strict;
use warnings;
use Carp;

require Exporter;
our $VERSION = '0.01';

use Cwd;
use FileHandle;
use File::Path;
use File::Copy;
use Params::Validate;

use PS::MOPS::DC::Field;
use PS::MOPS::DC::Detection;


use constant NSD_PACK_TEMPLATE => join('',
    'l',        # field_id (4)
    'l',        # det_num (4)
    'ddd',      # epoch/ra/dec (24)
    'fffff',    # mag/s2n/sigmas (20)
);
use constant NSD_TEMPLATE_LENGTH => 52;
our @NSD_FIELD_DESCRIPTIONS = qw(
    FIELD_ID DET_NUM
    EPOCH_MJD RA_DEG DEC_DEG
    MAG S2N RA_SIGMA_DEG DEC_SIGMA_DEG MAG_SIGMA
);

use constant SYD_PACK_TEMPLATE => join('',
    'l',        # field_id (4)
    'q',        # det_id (8)
    'ddd',      # epoch/ra/dec (24)
    'fffff',    # mag/s2n/sigmas (20)
    'A12',      # 9-char string (padded to 12)
);
use constant SYD_TEMPLATE_LENGTH => 68;
our @SYD_FIELD_DESCRIPTIONS = qw(
    FIELD_ID DET_ID
    EPOCH_MJD RA_DEG DEC_DEG
    MAG S2N RA_SIGMA_DEG DEC_SIGMA_DEG MAG_SIGMA OBJECT_NAME
);

our @USED_FIELD_DESCRIPTIONS = qw(
    FIELD_ID DET_NUM DET_ID
);


# File extensions for various stages of processing.
our $NSD_SRC_EXT = 'nsd.src.bin';       # source NSD file per-field
our $NSD_EXT = 'nsd.bin';               # processed NSD binary file
our $NSD_IDX_EXT = 'nsd.idx';           # binary index file for nonsynthetic detections

our $SYD_SRC_EXT = 'syd.src.bin';       # source SYD file per-field
our $SYD_EXT = 'syd.bin';               # processed SYD binary file (contains det_id)
our $SYD_IDX_EXT = 'syd.idx';           # binary index file for synthetic detections

our $USED_EXT = 'used';                 # exported nightly used list


sub GetNSDArchivePath {
    # For the given field, return the full path to the file where we whould archive its LSDs.
    my ($field) = @_;
    my $instance = $field->{_inst};     # owning instance
    my $ocnum = $field->ocnum;          # MOPS OC number
    my $nn = $field->nn;                # night number
    my $field_id = $field->fieldId;     # field's IPP fpa ID

    # Make sure the required directory exists.
    my $lsd_archive_dir = $instance->getEnvironment('LSDDIR');
    die("not a directory: $lsd_archive_dir") unless -d $lsd_archive_dir;
    my $lsd_nn_dir = "$lsd_archive_dir/$nn";
    eval { mkpath($lsd_nn_dir) };
    die("can't create directory: $lsd_nn_dir: $@") if $@;

    my $bin_filename = "$lsd_nn_dir/${field_id}.$NSD_SRC_EXT";
    return $bin_filename;       # that's it!
}


sub ArchiveNSDField {
    # Given a list of NSDs from an ingested field, store them in a binary file.
    my ($field, $dets_ar) = @_;
    my $instance = $field->{_inst};     # owning instance
    my $ocnum = $field->ocnum;          # MOPS OC number
    my $nn = $field->nn;                # night number
    my $field_id = $field->fieldId;     # field's IPP fpa ID

    # Make sure the required directory exists.
    my $ingest_dir = $instance->makeNNDir(NN => $nn, SUBSYS => 'lsdingest');
    my $bin_filename = "$ingest_dir/${field_id}.$NSD_SRC_EXT";

    # OK, prepare the string we'll be writing to the BIN file.
    my @packed_det_list;
    foreach my $det (@{$dets_ar}) {
        push @packed_det_list, 
            pack NSD_PACK_TEMPLATE, 
                $field_id,
                $det->detNum,
                $det->epoch,
                $det->ra,
                $det->dec,
                $det->mag,
                $det->s2n,
                $det->raSigma,
                $det->decSigma,
                $det->magSigma;
    }

    my $all_dets_str = join('', @packed_det_list);
    my $bin_fh = FileHandle->new(">>$bin_filename");
    print $bin_fh $all_dets_str;
    $bin_fh->close();
}

 
sub ArchiveSYDField {
    # Given a list of synthetic detections from an ingested field, store them in a binary
    # file temporarily.  These detections have detection IDs but not detection numbers.
    my ($field, $dets_ar) = @_;
    my $instance = $field->{_inst};     # owning instance
    my $ocnum = $field->ocnum;          # MOPS OC number
    my $nn = $field->nn;                # night number
    my $field_id = $field->fieldId;     # field's IPP fpa ID

    # Make sure the required directory exists.
    my $ingest_dir = $instance->makeNNDir(NN => $nn, SUBSYS => 'lsdingest');
    my $bin_filename = "$ingest_dir/${field_id}.$SYD_SRC_EXT";

    # OK, prepare the string we'll be writing to the BIN file.
    my @packed_det_list;
    my $object_name;
    foreach my $det (@{$dets_ar}) {
        my ($det_id, $det_num) =($det->detId, $det->detNum);
        $det_id = 0 if (!defined($det_id));             # nonsynthetic
        $det_num = 0 if (!defined($det_num));           # synthetic

        # Sanitize our object_name for the archive.
        $object_name = $det->objectName || die("synthetic detection has no object name");
        $object_name = substr($object_name, 0, 9);

        # Pack it!
        push @packed_det_list, 
            pack SYD_PACK_TEMPLATE, 
                $field_id,
                $det->detId,
                $det->epoch,
                $det->ra,
                $det->dec,
                $det->mag,
                $det->s2n,
                $det->raSigma,
                $det->decSigma,
                $det->magSigma,
                $object_name;
    }

    my $all_dets_str = join('', @packed_det_list);
    my $bin_fh = FileHandle->new(">>$bin_filename");
    print $bin_fh $all_dets_str;
    $bin_fh->close();
}


sub _sort_strip {
    # Given a string that is a series of NSDs, unpack it, sort it by RA, then pack it again.
    my ($template, $template_length, $buf) = @_;
    my $size = length($buf);            # number of bytes in buffer
    my $off = 0;                        # offset into buffer while unpacking
    my @packed_dets;

    while ($off < $size) {
        # Extract packed det at offset and append to array.
        push @packed_dets, substr($buf, $off, $template_length);
        $off += $template_length;
    }

    # Sort the array in RA.
    @packed_dets = sort {
        (unpack($template, $a))[3] <=> (unpack($template, $b))[3] 
    } @packed_dets;

    return join('', @packed_dets);
}


sub _analyze_field {
    # Scan a buffer of nightly LSD data for a single field and create 
    # a list of index entries whose format is as follows:
    # INDEX = {
    #   FIELD_ID => $field_id,
    #   RA_DEG => $field->ra,           # field center RA
    #   RA1_DEG => $ra_min_deg,         # slice min RA
    #   RA2_DEG => $ra_max_deg,         # slice max RA
    #   DEC => $field->dec,             # field center DEC
    #   DEC1 => $dec_min_deg,           # slice min DEC
    #   DEC2 => $dec_max_deg,           # slice max DEC
    #   START_RECNUM => n,              # starting record number in NSD file
    #   LENGTH => m,                    # length, in number of record   BUF => $buf,                    # NSD data
    # }
    my ($template, $template_length, $num_dets, $num_slices, $field, $nsd_buf) = @_;
    my $i_det = 0;
    my $det_buf;
    my $slice_num;
    my $slice_width;
    my $key;
    my $ra_deg;
    my $dec_deg;
    my %SLICE_DATA;
    my @stuff;

    # Our plan is to split everything into slices, then sort each slice.  While sorting each
    # slice, record min/max RA and Dec info, since the index wants it.

    # 1. Split into slices.  Just extract a det from the buffer and put in a slice.
    while ($i_det < $num_dets) {
        # Extract det data
        #
        $det_buf = substr($nsd_buf, $i_det * $template_length, $template_length);
        @stuff = unpack $template, $det_buf;
        ($ra_deg, $dec_deg) = @stuff[3, 4];     # RA/DEC encoded in template

        # Ensure RA is in 0,360.  (Should be check other stuff here as well?)
        if ($ra_deg < 0 or $ra_deg >= 360) {
            die("bogus RA: $ra_deg");
        }

        # Append det to appropriate slice.
        $slice_width = 360 / $num_slices;
        $slice_num = int(($ra_deg * $num_slices) / 360); # map to 0...(NUM_SLICES - 1)
        if (!exists($SLICE_DATA{$slice_num})) {
            $SLICE_DATA{$slice_num} = {
                FIELD_ID => $field->fieldId,
                RA => $field->ra,
                RA1 => $slice_num * $slice_width,
                RA2 => ($slice_num + 1) * $slice_width,
                DEC => $field->dec,
                DEC1 => $dec_deg,
                DEC2 => $dec_deg,
                START => 0,
                LENGTH => 0,
                BUFFER => $det_buf,         # actual det data
            };
        }
        else {
            $SLICE_DATA{$slice_num}->{BUFFER} .= $det_buf;        # how slow is this?
            if ($dec_deg < $SLICE_DATA{$slice_num}->{DEC1}) {
                $SLICE_DATA{$slice_num}->{DEC1} = $dec_deg;             # update min DEC
            }
            if ($dec_deg > $SLICE_DATA{$slice_num}->{DEC2}) {
                $SLICE_DATA{$slice_num}->{DEC2} = $dec_deg;             # update max DEC
            }
        }

        $i_det++;
    }

    # 2. Sort each strip in RA.
    foreach $key (keys %SLICE_DATA) {
        $SLICE_DATA{$key}->{BUFFER} = _sort_strip(
            $template,
            $template_length,
            $SLICE_DATA{$key}->{BUFFER},
        );
        $SLICE_DATA{$key}->{LENGTH} = length($SLICE_DATA{$key}->{BUFFER}) / $template_length;
    }

    return \%SLICE_DATA;
}


sub _make_index_entry {
    my ($item, $start) = @_;
    return 
        join(' ',
            $item->{FIELD_ID},
            $item->{RA},
            $item->{RA1},
            $item->{RA2},
            $item->{DEC},
            $item->{DEC1},
            $item->{DEC2},
            $start,
            $item->{LENGTH},
        ) . "\n";
}


sub MergeNSDFields {
    # Given an instance and night number, locate all NSD BIN files in the archive
    # directory, then merge the files into an RA-sorted binary file.
    my ($inst, $nn) = @_;

    # The plan will be to scan each available .src.bin file, scan the NSDs in
    # the file and simply append to .strip.bin files, then sort each
    # strip individually and concatenate into a single .nsd.bin file.  While
    # writing the nsd.bin file, record index boundaries then write the .idx
    # file.

    # Configuration stuff.
    my $mops_config = $inst->getConfig();
    my $mops_logger = $inst->getLogger();
    my $num_slices = $mops_config->{lsd}->{num_slices} || 3600;

    my $ingest_dir = $inst->makeNNDir(NN => $nn, SUBSYS => 'lsdingest');
    my $oldwd = getcwd;

    eval {
        chdir $ingest_dir or die "can't chdir to $ingest_dir";
        opendir LSDDIR, '.' or die "can't open $ingest_dir";
        my @all_files = readdir LSDDIR;
        close LSDDIR;
        my @src_files = grep { /$NSD_SRC_EXT$/ } @all_files;        # save src file list

        # Sort em using Schwartzian transform.
        @src_files = map  { $_->[0] }
              sort { $a->[1] <=> $b->[1] }
              map  { [$_, (split(/\W+/, $_))[0]] }          # sort using first part of filename
                   @src_files;


        # For each file (in numeric field ID ordering):
        # 1. Read the NSDs from the file
        # 2. Bin the NSDs into 1/NUM_SLICES RA strips (360/NUM_SLICES degrees)
        # 3. Append the detections for each memory strip to the file-based strips.
        my $field;
        my $field_id;
        my $nsd_filename = "$nn.$NSD_EXT";
        my $idx_filename = "$nn.$NSD_IDX_EXT";
        my $preidx_filename = "$nn.$NSD_IDX_EXT.pre";
        my $start = 0;          # slice starting location in file

        # Create empty bin and index files.
        if (-f $nsd_filename) {
            unlink $nsd_filename or die "can't unlink $nsd_filename";
        }
        if (-f $idx_filename) {
            unlink $idx_filename or die "can't unlink $idx_filename";
        }

        my $index_offset = 0;
        foreach my $src_file (@src_files) {
            die "can't open source file $src_file" unless -r $src_file;
            if ($src_file !~ /^(\d+)/) {
                die "can't get field ID from $src_file";
            }
            $field_id = $1;             # get from filename
            $field = modcf_retrieve($inst, fieldId => $field_id);

            # Execute an external C program to do stuff.
            my @cmd = (
                'mergeNSD',
                '--num_slices', $mops_config->{lsd}->{num_slices},
                '--field_id', $field->fieldId,
                '--field_ra_deg', $field->ra,
                '--field_dec_deg', $field->dec,
                '--index_file_offset', $index_offset,
                $src_file,
                $nsd_filename,
                $preidx_filename
            );
            $mops_logger->info("EXEC " . join(' ', @cmd));
            system(@cmd) == 0 or die("can't merge NSD file: $src_file");
        }

        # MergeNSD writes out a "preindex file" that contains slice lengths
        # but not offsets, since it can't see all files at once.  So scan
        # the preindex file and write out a true index file that contains
        # slice offsets and lengths.
        my @all_lines;
        if (-e $preidx_filename) {
            my $prefh = new FileHandle $preidx_filename or die "can't open $preidx_filename";
            @all_lines = $prefh->getlines();
            chomp @all_lines;
            $prefh->close();
        }

        my $idxfh = new FileHandle ">$idx_filename" or die "can't open $idx_filename for writing";
        my @stuff;
        my $offset = 0;
        my $len;
        for my $line (@all_lines) {
            @stuff = split /\s+/, $line;
            $len = pop @stuff;
            push @stuff, $offset, $len;
            print $idxfh join(' ', @stuff), "\n";
            $offset += $len;
        }
        $idxfh->close();

        # At this point, they're all merged, so copy the merge file to 
        # the LSD archive are and remove the per-field files.
        my $lsd_dest_dir = $inst->getEnvironment('LSDDIR') . "/$nn";
        unless (-d $lsd_dest_dir) {
            eval { mkpath($lsd_dest_dir) };
            die "can't create dir $lsd_dest_dir: $@" if $@;
        }

        if (-e $nsd_filename) {
            copy($nsd_filename, "$lsd_dest_dir/$nsd_filename") or die "can't copy $nsd_filename to LSD dir";
            copy($idx_filename, "$lsd_dest_dir/$idx_filename") or die "can't copy $idx_filename to LSD dir";
        }
        else {
            # Create empty files in the archive area.
            my $foo;
            $foo = new FileHandle ">$lsd_dest_dir/$nsd_filename";
            $foo->close;
            $foo = new FileHandle ">$lsd_dest_dir/$idx_filename";
            $foo->close;
        }
    #    foreach my $src_file (@src_files) {
    #        unlink $src_file or die "can't unlink $src_file";
    #    }

    };  # eval
    if ($@) {
        chdir $oldwd;
        die $@;
    }
}



sub MergeSYDFields {
    # Given an instance and night number, locate all SYD BIN files in the archive
    # directory, then merge the files into an RA-sorted binary file.
    my ($inst, $nn) = @_;

    # The plan will be to scan each available .src.bin file, scan the SYDs in
    # the file and simply append to .strip.bin files, then sort each
    # strip individually and concatenate into a single .syd.bin file.  While
    # writing the syd.bin file, record index boundaries then write the .idx
    # file.

    # Configuration stuff.
    my $mops_config = $inst->getConfig();
    my $num_slices = $mops_config->{lsd}->{num_slices} || 3600;

    # Retrieve the list of source files to process.
    my $lsd_dir = $inst->makeNNDir(NN => $nn, SUBSYS => 'lsdingest');
    my $oldwd = getcwd;

    eval {
        chdir $lsd_dir or die "can't chdir to $lsd_dir";

        opendir LSDDIR, '.' or die "can't open $lsd_dir";
        my @all_files = readdir LSDDIR;
        close LSDDIR;
        my @src_files = grep { /$SYD_SRC_EXT$/ } @all_files;   # save src file list

        # Sort em using Schwartzian transform.
        @src_files = map  { $_->[0] }
              sort { $a->[1] cmp $b->[1] }
              map  { [$_, (split(/\W+/, $_))[0]] }          # sort using first part of filename
                   @src_files;

        # For each file:
        # 1. Read the SYDs from th file
        # 2. Bin the SYDs into 1/NUM_SLICES RA strips (360/NUM_SLICES degrees)
        # 3. Append the detections for each memory strip to the file-based strips.
        my $field;
        my $field_id;
        my $syd_filename = "$nn.$SYD_EXT";
        my $idx_filename = "$nn.$SYD_IDX_EXT";
        my $start = 0;          # slice starting location in file

        # Create empty bin and index files.
        if (-f $syd_filename) {
            unlink $syd_filename or die "can't unlink $syd_filename";
        }
        if (-f $idx_filename) {
            unlink $idx_filename or die "can't unlink $idx_filename";
        }

        foreach my $src_file (@src_files) {
            die "can't open source file $src_file" unless -r $src_file;
            if ($src_file !~ /^(\d+)/) {
                die "can't get field ID from $src_file";
            }
            $field_id = $1;

            my $fh = new FileHandle $src_file;
            my $file_size = -s $src_file;
            my $buf;
            my $num_read = $fh->read($buf, $file_size);           # read entire LSD file
            die("incomplete read of $file_size bytes from $src_file") if $file_size != $num_read;
            die("buffer is not a multiple of SYD record size ($src_file, $file_size)") if $file_size % PS::MOPS::LSD::SYD_TEMPLATE_LENGTH != 0;
            my $num_dets = int($file_size / SYD_TEMPLATE_LENGTH);
            $field = modcf_retrieve($inst, fieldId => $field_id);
            my $slice_data = _analyze_field(
                SYD_PACK_TEMPLATE,
                SYD_TEMPLATE_LENGTH,
                $num_dets,
                $num_slices,
                $field, 
                $buf
            );

            my $nsd_fh = new FileHandle ">>$syd_filename";       # open/create/trunc+append
            my $idx_fh = new FileHandle ">>$idx_filename";       # open/create/trunc+append

            for my $key (sort keys %{$slice_data}) {
                print $nsd_fh $slice_data->{$key}->{BUFFER};    # raw sorted slice data
                print $idx_fh _make_index_entry($slice_data->{$key}, $start);    # index line
                $start += $slice_data->{$key}->{LENGTH};
            }

            $nsd_fh->close();
            $idx_fh->close();
        }

        # At this point, they're all merged, so copy the merge file to 
        # the LSD archive are and remove the per-field files.
        my $lsd_dest_dir = $inst->getEnvironment('LSDDIR') . "/$nn";
        unless (-d $lsd_dest_dir) {
            eval { mkpath($lsd_dest_dir) };
            die "can't create dir $lsd_dest_dir: $@" if $@;
        }

        if (-e $syd_filename) {
            copy($syd_filename, "$lsd_dest_dir/$syd_filename") or die "can't copy $syd_filename to LSD dir";
            copy($idx_filename, "$lsd_dest_dir/$idx_filename") or die "can't copy $idx_filename to LSD dir";
        }
        else {
            # Create empty files in the archive area.
            my $foo;
            $foo = new FileHandle ">$lsd_dest_dir/$syd_filename";
            $foo->close;
            $foo = new FileHandle ">$lsd_dest_dir/$idx_filename";
            $foo->close;
        }
    #    foreach my $src_file (@src_files) {
    #        unlink $src_file or die "can't unlink $src_file";
    #    }
    };  # eval

    if ($@) {
        chdir $oldwd;
        die $@;
    }
}


sub WriteUsedDetections {
    # Given a list of field IDs, extract unattributed detections from the fields
    # and archive them in an LSD-style binary archive.
    my ($instance, $nn) = @_;

    # Keep a table of detection IDs that we've written so that we don't write
    # out duplicates.
    my %written;

    # Make sure the required directory exists.
    my $lsd_archive_dir = $instance->getEnvironment('LSDDIR');
    die("not a directory: $lsd_archive_dir") unless -d $lsd_archive_dir;
    my $lsd_nn_dir = "$lsd_archive_dir/$nn";
    die("can't create directory: $lsd_nn_dir") unless -d $lsd_nn_dir;

    # Now write a binary file in this directory with the name NN.used.
    my $det_i = modcd_retrieve($instance, export_lsd => 1, nn => $nn);
    my $det;
    my $used_filename = "$lsd_nn_dir/$nn.$USED_EXT";

    # OK, prepare the string we'll be writing to the BIN file.
    my @packed_det_list;
    my $det_id;
    my $used_fh = FileHandle->new(">$used_filename");

    while ($det = $det_i->next()) {
        $det_id = $det->detId;
        if (!$written{$det_id}) {
            print $used_fh join(' ', $det->fieldId, $det->detNum || 0, $det->detId), "\n";
            $written{$det_id} = 1;
        }
    }

    $used_fh->close();
    return $used_filename;
}


1;
__END__

=head1 NAME

PS::MOPS::LSD - Perl extension for creation of nightly low-significance
detection (LSD) files

=head1 SYNOPSIS

  use PS::MOPS::LSD;

=head1 DESCRIPTION

This module provides library routines to manipulate MOPS low-significance
detection archives.  In particular, we will provide mechanisms to

* Take a list of nonsynthetic detections and archive them temporarily
in a simulation's LSD holding area

* Merge all LSDs for a night into a RA-sorted binary file pair (.lsd+.idx)
for rapid searching of detections for tracklet processing

* Extract a list of detections within an (RA, Dec, epoch) region

Files will be stored in per-night directories within a simulation's var/
directory structure.  At the end of ingest for a night, the per-night
BIN files are merged into .lsd and .idx files.


LSD ARCHIVE FILE FORMATS

The LSD files are designed for rapid read/write and minimum necessary
information.  Each detection will be a single fixed-length binary record.
There will be two kinds of archive files: one for nonsynthetic detections
(NSDs), and another for synthetics.  The reason for separate archive
files is that database detections include a MOPS detection ID, which
will be used to match detections against a "used detection" list during
nightly processing.  However, nearly all LSDs are never upgraded into
DB detections, so it would be immensely wasteful to allocate a slot in
the packed detection format for a detection ID when 99% of the time it
will be ununsed.

The packed detection format for NSDs is

field_id        float(4)        # MOPS field ID containing LSD
det_num         int(4)          # detection number in source IPP FITS file
epoch_mjd       float(8)        # epoch of detection
ra_deg          float(8)
dec_deg         float(8)
mag             float(4)
s2n             float(4)
ra_sigma_deg    float(4)
dec_sigma_deg   float(4)
mag_sigma_deg   float(4)

For synthetic detections (SYDs)

field_id        float(4)        # MOPS field ID containing LSD
det_id          int(8)          # MOPS detection ID
epoch_mjd       float(8)        # epoch of detection
ra_deg          float(8)
dec_deg         float(8)
mag             float(4)
s2n             float(4)
ra_sigma_deg    float(4)
dec_sigma_deg   float(4)
mag_sigma_deg   float(4)

Note that detection elongation information is not preserved from 
the original detections; at low-significance, there will be no
usable elongation information.


=head2 EXPORT

None by default.



=head1 SEE ALSO

PS::MOPS::DC modules, PS1 MOPS Software Design Description.

=head1 AUTHOR

Larry Denneau, Jr., Institute for Astronomy, University of Hawaii,
E<lt>denneau@ifa.hawaii.eduE<gt>.


=head1 COPYRIGHT AND LICENSE

Copyright (C) 2008 by Larry Denneau, Jr., Insitute for Astronomy,
University of Hawaii.

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.7 or,
at your option, any later version of Perl 5 you may have available.

=cut
