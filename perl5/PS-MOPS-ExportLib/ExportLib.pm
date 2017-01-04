package PS::MOPS::ExportLib;

## @class PS::MOPS::ExportLib
# PS1-MOPS data export functions.
# @par DESCRIPTION
# This module implements the data export process for MOPS Orbits and
# Detections. Data to be exported is recorded in the MOPS database prior to
# export. During processing, Detections previously exported are compared to
# Detections that are to be newly exported. Removal records are added to the
# export set, as necessary, when Detections previously exported are no longer
# associated with a particular DO.
# @par
# The output consists of a single line for each object. Orbits and Detections
# appear in separate files and are associated by an alphanumeric Derived
# Object ID.

# @todo Adapt export library to use MOPS configuration settings.
# @todo Generate export configuration settings appropriate to export.
# @todo Remove access to private functions.
# @todo Generate Build package for module.

use strict;
our $VERSION = sprintf "%d", q$Revision: 2230 $ =~ /(\d+)/g;

use warnings;

require Exporter;

our @ISA=qw(Exporter);
our @EXPORT=qw(get_mops_detections
               insert_detections_for_export
               _retrieve_export_detections
               _get_detection_set_difference
               retrieve_orbit
               export_pending_mops_objects);

use PS::MOPS::DC::Detection;

# Define constants.
use constant DETECTION_LINE_MAXIMUM => 250000;
use constant ORBIT_LINE_MAXIMUM => 250000;
use constant PENDING => 'P';

# The two sets of Detections that are treated by this module are termed
# the MOPS detections and the export detections.

# Error conditions:
# Instance verification is performed by DBI.
# If dbh() is not available, DBI connect() will fail.

# Get all the MOPS detections for a DO.
# @param inst DB instance.
# @param do Derived Object ID.
# @return array reference to array of Detections.
sub get_mops_detections {
    my($inst, $do)=@_;
    # Retrieve orbit ID for DO.
    # @todo Verify the correctness of this statement.
    # @todo Limit select to detection ID and other needed data.
    my $sql="SELECT
             detections.*
             FROM
             detections ,
             derivedobject_attrib
             INNER JOIN tracklet_attrib ON detections.det_id = tracklet_attrib.det_id
             AND tracklet_attrib.tracklet_id = derivedobject_attrib.tracklet_id
             WHERE derivedobject_attrib.derivedobject_id='$do'";

    my $dbh=$inst->dbh();
    my $sth=$dbh->prepare($sql);
    $sth->execute();
    my $cols;
    my @det_set;
    while($cols=$sth->fetchrow_arrayref()){
        my $c=$cols->[0];
        push(@det_set, modcd_retrieve($inst,detId=>$c));
    }
    return \@det_set;
}

# Iterate over pending DOs. Export orbits and detections.
# Orbits and Detection are written to text files.
#
# File naming scheme components
# (data type) (dash) (3-digit ID) (dash) (year) (month) (day) (underscore) (hour) (minute) (second) (extension)
#
# The 3-digit ID is used to export multiple files for large data sets.
# Each file will contain a fixed maximum number of data lines.
# Date and time refer to the last date/time of the pending revision records.
#
# Examples
# orbits-001-20071005_120000.txt
# detections-500-20071006_231901.txt
#
# @param inst DB instance.
# @return None.
sub export_pending_mops_objects {
    my ($inst)=@_;

    my $dbh=$inst->dbh();
    my $sql;
    my $sth;

    # Get all pending revision records.
    $sql="SELECT do_export_id,
                 rev_time,
                 revision_id,
                 orbit_id,
                 object_name,
                 ssm_id
          FROM export, derivedobjects
          WHERE export_status='".PENDING."' AND do_export_id=derivedobjects.derivedobject_id
          ORDER by rev_time";

    $sth=$dbh->prepare($sql);
    $sth->execute();
    my $revision_records=$sth->fetchall_arrayref();

    if(scalar(@{$revision_records}) <= 0){
        return;
    }

    my $export_detections;
    my $do_id;
    my $orbit;

    # Database value mapping.
    my $last_time=$revision_records->[ scalar(@{$revision_records}) - 1 ]->[1];

    if($last_time=~s/\-//g){} # Remove dashes.
    if($last_time=~s/\://g){} # Remove colons.
    if($last_time=~s/\s/\_/g){} # Change space to underscore.
    my $detections_file_root="$last_time-detections";
    my $orbits_file_root="$last_time-orbits";

    my $detections_file_name="";
    my $orbits_file_name="";

    my $detections_file_id=1;
    my $orbits_file_id=1;

    my $fh_det=*DETECTIONS; # Filehandle.
    my $fh_orb=*ORBITS; # Filehandle.

    my $export_cnt=0;
    my $orbit_line_cnt=0;
    my $det_line_cnt=0;
    my $extension="txt";

    my $files_open=0;


    # Iterate over the revision records.
    # Note: The same DO may appear in multiple revision records.
    # Retrieve an orbit and retrieve the exportable detections.
    foreach my $r (@{$revision_records}){

        # Determine which file to write to. If a new file is needed (i.e. the
        # maximum data line count has been reach for the file), then open a
        # new file.

        if(!$files_open){
            $detections_file_name=sprintf("ps1-%s-%03d.%s",$detections_file_root,$detections_file_id,$extension);
            open($fh_det, ">$detections_file_name");
            $orbits_file_name=sprintf("ps1-%s-%03d.%s",$orbits_file_root,$orbits_file_id,$extension);
            open($fh_orb, ">$orbits_file_name");
            $files_open=1;
        }

        $export_cnt++;
        print STDERR "Exporting revision record ($export_cnt)...\n";

        # Map database columns.
        $do_id=$r->[0];
        my $revision_id=$r->[2];
        my $do_name=$r->[4];
        my $ssm_id=$r->[5];

        # Retrieve the set of detections associated with the revision record only.
        $sql="select det_export_id, rev_id, det_id, removal from export_dets where rev_id='$revision_id'";
        $sth=$dbh->prepare($sql);
        $sth->execute();
        my $rev_det_export=$sth->fetchall_arrayref(); # Revision detection records.

        # @note Retrieving whole set of export detections will not serve our purpose.

        # Create a hash to hold hashes.
        my %mops_det_to_export_det_hash;

        # Iterate over revision detection records.
        foreach my $rd (@{$rev_det_export}){

            # Map database columns.
            my $det_export_id=$rd->[0];
            my $det_id=$rd->[2];
            my $removal=$rd->[3];

            # Retrieve Detection matching the revision detection record.
            $mops_det_to_export_det_hash{$det_export_id}->{detection}=modcd_retrieve($inst,detId=>$det_id);
            if(lc($removal) eq "y"){
                $mops_det_to_export_det_hash{$det_export_id}->{removal}=1;
            }
            else { 
                $mops_det_to_export_det_hash{$det_export_id}->{removal}=0;
            }
        }

        my $export_ok=0;

        # @todo Handle case where export is not OK.

        my $synthetic_orbit=0;
        $synthetic_orbit=1 if $ssm_id; # @todo Verify this works.
                                       # i.e. will $ssm_id=0 when ssm_id is not present?

        # Export the orbit.
        $orbit=retrieve_orbit($inst, $do_id);
        if(write_orbit_line($fh_orb,$do_name,$orbit,$synthetic_orbit)){
            # Update revision record status.
            $orbit_line_cnt++;
            $export_ok=1;
        }
        else {
            $export_ok=0;
        }

        # Iterate over Detections to export.
        while ( (my $k, my $v) = each %mops_det_to_export_det_hash) {
            if($det_line_cnt >= DETECTION_LINE_MAXIMUM) {
                $det_line_cnt=0;
                close($fh_det);
                # Open a new file.
                $detections_file_id++;
                # @todo Error check detection file ID number.
                $detections_file_name=sprintf("ps1-%s-%03d.%s",$detections_file_root,$detections_file_id,$extension);
                open($fh_det, ">$detections_file_name");
            }

            my $synthetic_det=0;
            $synthetic_det=1 if $ssm_id; # @todo Verify this works.
            if(write_detection_line($fh_det,$do_name,$v->{detection},$v->{removal},$synthetic_det)){
                # Update revision record status.
                $det_line_cnt++;
                $export_ok=1
            }
            else {
                $export_ok=0;
            }
        }
        update_revision_record_status($inst,$revision_id,"E");
    }
    close($fh_det);
    close($fh_orb);
}

# @param inst PS-MOPS Instance.
# @param rev_id Revision record ID.
# @param status Status that record will be updated to.
sub update_revision_record_status {
    my ($inst,$rev_id,$status)=@_;
    if($status ne "E" && $status ne "P"){
        die "Status $status not recognized.\n";
    }
    my $dbh=$inst->dbh();
    my $sql;
    $sql="update export set export_status=?, export_time=current_timestamp() where revision_id=?";
    my $sth=$dbh->prepare($sql);
    $sth->execute($status,$rev_id) or die $dbh->errstr;
}

# Write data for a single Orbit as a line of text.
# @param fh Filehandle.
# @param do_id DerivedObject ID.
# @param orb Orbit.
# @return 1 for success, 0 for fail.
sub write_orbit_line {
    my ($fh,$do_id,$orb,$synthetic)=@_;

    my $orbit_id;
    if($synthetic){
        $orbit_id="S" . $orb->{orbitId};
    }
    else {
        $orbit_id=$orb->{orbitId}
    }

    # @todo Wrap print statements for error catching.
    print $fh "$do_id $orbit_id COM $orb->{q} $orb->{e} $orb->{i} $orb->{node} $orb->{argPeri} $orb->{timePeri} $orb->{hV} $orb->{epoch}";
    print $fh "\n";
    return 1;
}

# Write data for a single Detection as a line of text.
# @param fh Filehandle
# @param do_id DerivedObject ID.
# @param det Detection.
# @param removal 1 for a removal detection, 0 otherwise.
# @return 1 for success, 0 for fail.
sub write_detection_line {
    my ($fh,$do_id,$det,$removal,$synthetic)=@_;
    # @todo Wrap print statements for error catching.
    my $det_id;
    if($synthetic){
        $det_id="S" . $det->{detId};
    }
    else {
        $det_id=$det->{detId};
    }
    print $fh "$do_id $det_id $det->{epoch} O $det->{ra} $det->{dec} $det->{mag} $det->{filter} $det->{obscode} $det->{raSigma} $det->{decSigma} $det->{magSigma} $det->{s2n}";
    if ($removal){
        print $fh " RMV";
    }
    print $fh "\n";
    return 1;
}

# For a given DO, retrieve the an Orbit for it.
# @param MOPS instance.
# @param do_id DO ID.
# @return Orbit reference.
sub retrieve_orbit {
    my ($inst, $do_id)=@_;

    use PS::MOPS::DC::Orbit;

    my $dbh=$inst->dbh();
    my $sql;
    my $sth;
    $sql="select orbit_id from derivedobjects where derivedobject_id='$do_id'";
    $sth=$dbh->prepare($sql);
    $sth->execute();
    my $row=$sth->fetchrow_arrayref();
    my $orbit_id=$row->[0];
    my $orbit;
    if ($orbit_id) {
        $orbit=modco_retrieve($inst, orbitId=>$orbit_id);
    }

    return $orbit;
}

# Return Detections that are in A but not in B or (A - B).
# @param orig Array reference.
# @param compare Array reference.
# @return Array reference.
sub _get_detection_set_difference {
    my($inst, $orig, $compare)=@_;
    my %compare_set;

    # Create comparison hash.
    foreach my $c (@{$compare}){
        $compare_set{$c->{detId}}='';
    }

    my @result_set=();

    # Iterate over Detections.
    foreach my $d (@{$orig}){
        if(!exists($compare_set{$d->{detId}})){
            push(@result_set,$d);
        }
        my $orig_id=$d->{detId};
    }

    return \@result_set if @result_set;
    return 0;
}

# Given a DerivedObject and a set of Detections to be exported, create a
# revision record in table export and record the Detection IDs in export_dets.
#
# @param inst PS-MOPS instance.
# @param do ID of DO for Detections.
# @param detections Array reference to set of Detection objects.
# @return None.
sub insert_detections_for_export {

    # @todo Enable DB transactions.

    my ($inst, $do, $detections)=@_;

    print STDERR "Processing DO $do...\n";

    # Retrieve existing export detections for the given DO.
    my $export_dets=_retrieve_export_detections($inst, $do);

    my $mops_differences=0;
    my $export_differences=0;

    $mops_differences=_get_detection_set_difference($inst, $detections, $export_dets);
    $export_differences=_get_detection_set_difference($inst, $export_dets, $detections);

    # If differences exist between the MOPS detections and the existing export detections
    # then insert a revision record. Otherwise, return.
    if(!$mops_differences && !$export_differences){
        print STDERR "\tNo differences found in detections.\n";
        return;
    }

    # Write revision record for detection set.
    print STDERR "\tWriting revision record...\n";
    my $revision_id=_write_export_do($inst, $do);

    my $dbh=$inst->dbh();
    my $sql;
    my $sth;

    if($mops_differences){
        # Add the NEW detections from the MOPS database.
        foreach my $d (@{$mops_differences}){
            $sql="INSERT into export_dets(rev_id, det_id)
                  VALUES('$revision_id', '$d->{detId}')";
            $sth=$dbh->prepare($sql);
            $sth->execute();
        }
    }

    if($export_differences){
        # Add REMOVAL export detection records for detections that are in the MOPS database
        # but not associated with the the DO's export set.
        #
        # This is done instead of flipping the removal flag for existing export detections
        # so that it fits in with the time-based records for revisions.
        foreach my $d (@{$export_differences}){
            $sql="INSERT into export_dets(rev_id, det_id, removal)
              VALUES('$revision_id', '$d->{detId}', 'Y')";
            $sth=$dbh->prepare($sql);
            $sth->execute();
        }
    }

    # @note If a MOPS detection is deleted, it no longer exists in the export process.
}

# Retrieve existing export detection set for a given DO.
# Return the MOPS detections that correspond to the CURRENT export detection set.
# This requires performing removals in ascending time order.
# @param do_id DO ID.
# @return Array reference to array of Detections.
sub _retrieve_export_detections {
    my ($inst, $do_id)=@_;

    my $dbh=$inst->dbh();
    # First, retrieve all the revision records containing the DO.
    my $sql="SELECT revision_id, rev_time, do_export_id
             FROM export WHERE do_export_id='$do_id' ORDER BY rev_time ASC";
    my $sth;
    $sth=$dbh->prepare($sql);
    $sth->execute();
    my $revision_records;
    my $export_detection_rows;
    my @sum_export_dets;
    my %build_export_dets;
    my @export_detections;

    $revision_records=$sth->fetchall_arrayref();

    # Iterate over revision records in time order.
    # Perform summation of export detections.
    foreach my $rr (@{$revision_records}){
        $sql="select det_id, removal from export_dets where rev_id='$rr->[0]'";
        $sth=$dbh->prepare($sql);
        $sth->execute();
        $export_detection_rows=$sth->fetchall_arrayref();
        foreach my $ed (@{$export_detection_rows}){
            my $det_id=$ed->[0];
            my $removal=0;
            if(lc($ed->[1]) eq "y"){
                $removal=1;
            }
            if($removal){
                delete($build_export_dets{$det_id});
            }
            else {
                if(!exists($build_export_dets{$det_id})){
                    $build_export_dets{$det_id}='';
                }
            }
        }
    }

    # Retrieve MOPS Detections.
    while(my($k, $v)=each(%build_export_dets)){
        my $det=modcd_retrieve($inst, detId=>$k);
        push(@export_detections, $det);
    }
    return \@export_detections;
}

# Record export record.
# @param inst PS-MOPS instance.
# @param do_id Derived object ID.
# @return Revision ID.
sub _write_export_do {
    my ($inst, $do_id)=@_;
    my $revision_id;
    my $dbh=$inst->dbh();

    # @todo Verify do_id.

    # Export states: P = export pending
    #                E = exported

    my $sql="INSERT into export(do_export_id, rev_time, export_status) 
             VALUES('$do_id',current_timestamp(),'P')";
    my $sth=$dbh->prepare($sql);
    $sth->execute();
    $revision_id=$dbh->{mysql_insertid};
    return $revision_id;
}

# Unit test:
# With records in tracklet_attrib, change association of a tracklet and detection.
# Result should be one removal record added and one insert with new association added.

# Export formats
#
# DETECTIONS
#
# do_id det_id epoch ra dec mag filter obscode raSigma decSigma magSigma s2n removal
#
# ORBITS
#
# do_id orbit_id COM q e i node argPeri timePeri hV epoch

1;
