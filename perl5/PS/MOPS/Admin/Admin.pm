#!/usr/bin/env perl

=head1 NAME

PS::MOPS::Admin - collection of functions used by the MOPS web frontend 
                  for managing MOPS.

=head1 SYNOPSIS

  use PS::MOPS::Admin;

=head1 DESCRIPTION

This module contains functions that:
	1. queries the IPP and MOPS databases for information on the exposures 
	   stored in the databases.
	   
	2. modifies the status of chunks in MOPS to indicate that they are ready
	   for processing by MOPS.
	   
	3. queries MOPS for the status of chunks. 
	
=head1 Functions

=cut

package PS::MOPS::Admin;

use Exporter;
use strict;
use warnings;
use Carp;
use constant false => 0;
use constant true  => 1;


use PS::MOPS::IPPDB;
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Field;
use PS::MOPS::Constants qw(:all);

our $VERSION = '1.00';

our @ISA = qw(Exporter);

our %EXPORT_TAGS = ('all' => [qw(
	list_mops_fields 
	list_ipp_fields 
	ipp_last_exp 
	list_missing_exposures 
	change_chunk_status 
	chunk_has_status 
	get_run_id 
	update_runs_table
	list_processed_chunks
	change_chunk_czar_status
)]);

our @EXPORT_OK = (@{$EXPORT_TAGS{'all'}});
our @EXPORT = ();

my $EXPOSURE_BLACK_LIST = "'ENGINEERING', 'M31', 'CAL', 'Unknown', 'STS', 'SS', 'STS2A', 'STS2B', 'CNP', 'PI', 'STD', 'Footprint', 'SAS2', 'PR201108'";

=head2 list_mops_fields($instance, $nn)

Lists the chunks ingested into the specified simulation for the given night
number.  

Arguments

=item instance

Name of simulation to interogate.

=item night number


Night number 

=head3 Returns

Hash reference that is keyed on chunk name (i.e. 148.3PI.00.ANW3.J.r ) 
containing the number of fields that have been ingested for the chunk.

=cut
sub list_mops_fields {
	my ($instance_name, $nn) = @_;
	my %results;
	my @modes;
	my $i;
	my $inst = PS::MOPS::DC::Instance->new(DBNAME => $instance_name);
	
	@modes = PS::MOPS::DC::Field::modcf_retrieveSurveyModes($inst, nn => $nn);
	foreach $i (@modes) {
		$results{$i->[0]} = $i->[1];
	}    
    return \%results;
}

=head2 list_ipp_fields($nn)

List the chunks downloaded from the summit into the IPP database for the given
night number.

Arguments

=item night number

Night number 

=head3 Returns

Hash reference that is keyed on chunk name (i.e. 148.3PI.00.ANW3.J.r ) 
containing the number of fields that have been downloaded from the telescope
into IPP.

=cut
sub list_ipp_fields {
	my ($nn) = @_;
	my $dbh = PS::MOPS::IPPDB::dbh();
	my %results;
	my $href;

	# Set up query to get the chunks downloaded from the summit into IPP.
	# Please note that exposures not downloaded will not appear in the list.
	my $tjdfrag = 'o' . (int($nn - 50000) + 1);
	my $sql = <<"SQL";
SELECT substring_index(comment, ' ', 1) AS chunk_name, count(*) AS exposures,
MAX(dateobs) AS epoch
FROM rawExp 
WHERE exp_name regexp('^$tjdfrag') AND 
obs_mode NOT IN ($EXPOSURE_BLACK_LIST) 
GROUP BY  substring_index(comment, ' ', 1)
SQL

    my $sth = $dbh->prepare($sql) or die $dbh->errstr;
    
    # Execute the query and store results in a hash keyed on chunk name storing
    # the number of exposures found for each chunk.
    $sth->execute() or die $sth->errstr;
	while ($href = $sth->fetchrow_hashref) {
		$results{$href->{chunk_name}} = {NEXP => $href->{exposures},
			EPOCH_MJD => $href->{epoch} };
    }
    $sth->finish();
    return \%results;
}

=head2 ipp_last_exp()

Returns the truncated julian date of the last exposure downloaded to IPP that is 
not in the exposure black list.

=head3 Returns

String with the truncated julian date of the last downloaded MOPS exposure.

=cut
sub ipp_last_exp {
	my $dbh = PS::MOPS::IPPDB::dbh();
	my $tjd;
	my $href;

	# Set up query to get the chunks downloaded from the summit into IPP.
	# Please note that exposures not downloaded will not appear in the list.
	my $sql = <<"SQL";
SELECT substring(exp_name, 2, 4) AS tjd 
FROM rawExp 
WHERE obs_mode NOT IN ($EXPOSURE_BLACK_LIST) 
ORDER BY dateobs DESC
LIMIT 1
SQL

    my $sth = $dbh->prepare($sql) or die $dbh->errstr;
    
    # Execute the query and store results in a hash keyed on chunk name storing
    # the number of exposures found for each chunk.
    $sth->execute() or die $sth->errstr;
	($tjd) = $sth->fetchrow_array;
    $sth->finish();
    return $tjd;
}

=head2 list_missing_exposures($instace, $nn)

List the exposures that are present in the IPP database but is missing in the
MOPS database.

Arguments

=item instance

Name of simulation to interogate.

=item night number

Night number 

=head3 Returns

Hash reference that is keyed on chunk name (i.e. 148.3PI.00.ANW3.J.r ) 
containing an array that lists all of the missing exposures for the chunk.

=cut
sub list_missing_exposures {
	my ($instance_name, $nn) = @_;
	my %results;
	my %ipp_exposures;
	my %mops_exposures;
	my $href;
	my $inst = PS::MOPS::DC::Instance->new(DBNAME => $instance_name);
	my $mops_dbh = $inst->dbh();
	my $ipp_dbh = PS::MOPS::IPPDB::dbh();
	
	# Set up query to get the exposures downloaded from the summit into IPP.
	# Please note that exposures not downloaded will not appear in the list.
	my $tjdfrag = 'o' . (int($nn - 50000) + 1);
	my $sql = <<"SQL";
SELECT substring_index(comment, ' ', 1) AS chunk_name, exp_name FROM rawExp 
WHERE exp_name regexp('^$tjdfrag') AND 
obs_mode NOT IN 
('ENGINEERING', 'M31', 'CAL', 'Unknown', 'STS', 'SS', 'STS2A', 'STS2B', 'CNP', 'PI', 'STD', 'Footprint', 'SAS2', 'PR201108') 
SQL
	
    my $sth = $ipp_dbh->prepare($sql) or die $ipp_dbh->errstr;

    # Execute the query and store IPP results in a hash keyed on exp_name 
    # storing chunk names as values.
    $sth->execute() or die $sth->errstr;
    while ($href = $sth->fetchrow_hashref) {
		$ipp_exposures{$href->{exp_name}} = $href->{chunk_name};
    }	
    $sth->finish();
    
 	# Set up query to get the exposures imported into MOPS.
	$sql = <<"SQL";
SELECT fpa_id FROM fields 
WHERE nn = $nn 
SQL
    $sth = $mops_dbh->prepare($sql) or die $mops_dbh->errstr;
    
    # Execute the query and store results in a hash keyed on fpa_id.
    $sth->execute() or die $sth->errstr;
    my %ingested_exposures;
    
	while ($href = $sth->fetchrow_hashref) {
		$mops_exposures{$href->{fpa_id}} = 1; # Array of ingested exposures
    }
    $sth->finish();  
    
    # Compare the list of exposures in IPP with the list of exposures imported
    # into MOPS and record those that are not in MOPS.
    my $exposure;
    foreach $exposure (keys(%ipp_exposures)) {
    	unless (exists $mops_exposures{$exposure}) {
    		push(@{ $results{$ipp_exposures{$exposure}} }, $exposure);
    	}
    }  
    return \%results;
}

=head2 change_chunk_status($instance, $nn)

Change the status of fields that make up the chunk(s) from $FIELD_STATUS_INGESTED 
to $FIELD_STATUS_READY.

Arguments

=item instance

Name of simulation to interogate.

=item night number

Night number 

=item chunks

The name of one or more chunks whose status is to be updated.

=head3 Returns

Number of fields that were modified.

=cut
sub change_chunk_status {
	# Process parameters.
	my $instance_name = shift @_;
	my $nn = shift @_;
	my $chunks;
	my $chunk;

	foreach $chunk (@_) {
		$chunks .= "\'$chunk\', ";
	}
	$chunks = substr($chunks, 0, -2);	# Removes trailing comma and space.

	# Set up database connection to simulation containing chunks.
	my $inst = PS::MOPS::DC::Instance->new(DBNAME => $instance_name);
	my $dbh = $inst->dbh();
	
	# Set up query to update the chunks as ready for processing.
	my $sql = <<"SQL";
UPDATE fields SET status = "$FIELD_STATUS_READY"      
WHERE nn = $nn 
AND status = "$FIELD_STATUS_INGESTED" 
AND survey_mode in ($chunks)
SQL
	my $sth = $dbh->prepare($sql) or die $dbh->errstr;
    
	# Execute the query and update the chunk status.
	my $n = $sth->execute() or die $sth->errstr;
	$sth->finish;
	return $n;	
}

=head2 chunk_has_status

Checks to see if the specified chunk contains a field that has the status given.

Arguments

=item instance

Name of simulation to interogate.

=item night number

Night number 

=item chunk name

Name of the chunk whose status is to be checked.

=item status

Field status to search for.

=head3 Returns

True if chunk specified contains a field with the status specified. False 
otherwise

=cut
sub chunk_has_status {
	# Initialize parameters
	my $instance_name = shift @_;
	my $nn = shift @_;
	my $chunk = shift @_;
	my $status = shift @_;
	
	# Set up database connection to simulation containing chunks.
	my $inst = PS::MOPS::DC::Instance->new(DBNAME => $instance_name);
	my $dbh = $inst->dbh();
	
	# Set up query to update the chunks as ready for processing.
	my $sql = <<"SQL";
SELECT count(*)FROM fields WHERE nn = $nn AND survey_mode = '$chunk'
AND status = '$status'
SQL

	my $sth = $dbh->prepare($sql) or die $dbh->errstr;
	$sth->execute() or die $sth->errstr;
	my $aref = $sth->fetchrow_arrayref;
	$sth->finish;
	
	if ($aref->[0] > 0) {
		return true;
	} else {
		return false;
	}	
}

=head2 update_run_table

Updates the runs table with the field that just completed a mopper stage
(synth, tracklet, posttracklet, attrib, link).

Arguments

=item instance

Name of simulation that the processed fields came from.

=item run_id

Unique number that specifies the run. 

=item field_id

Name of the chunk whose status is to be checked.

=item status

The new status that was just assigned to the field.

=cut
sub update_runs_table {
    # Initialize parameters
    my ($inst, $run_id, $field_id, $status) = @_;
    
    my $count;
    my $ins_sql = "INSERT INTO runs (run_id, field_id, status) VALUES (?, ?, ?)";
    my $up_sql = "UPDATE runs SET status=? WHERE run_id=? AND field_id=?";
    my $sel_sql = "SELECT COUNT(*) FROM runs WHERE run_id=? AND field_id=?";
    
    my $dbh = $inst->dbh;
    
    # Determine if field was previously updated during the run
    my $sth = $dbh->prepare($sel_sql) or die $dbh->errstr;
    $sth->execute($run_id, $field_id) or die $dbh->errstr;
    ($count) = $sth->fetchrow_array();
    $sth->finish();
    
    # Update runs table
    if ($count > 0) {
        # Update run with new field status.
        $sth = $dbh->prepare($up_sql) or die $dbh->errstr;
        $sth->execute($status, $run_id, $field_id) or die $dbh->errstr;
    } else {
        # Insert new run into run table.
        $sth = $dbh->prepare($ins_sql) or die $dbh->errstr;
        $sth->execute($run_id, $field_id, $status) or die $dbh->errstr;
    }
    $sth->finish();
}

=head2 get_run_id

Retrieves the next available run id to be used with the export.runs table.

Arguments

None

=head3 Returns

Returns a new run id to be used for tracking when fields are processed.
=cut
sub get_run_id {
    my $inst = shift @_;
    my $run_id;
    
    my $dbh = $inst->dbh;
    my $sth = $dbh->prepare("SELECT MAX(run_id) FROM runs");
    
    $sth->execute();
    ($run_id) =  $sth->fetchrow_array();
    $sth->finish();
    if ($run_id) {
        return ($run_id + 1);
    } else {
        return 1;
    }
}

=head2 list_processed_chunks

Lists the chunks that have been processed so far for the night given. Chunks
will be grouped by run id.

Arguments

=item instance

Name of simulation to interogate.

=item nn

Night number which is used to restrict the chunks returned.

=head3 Returns

Hash reference that is keyed on run id which contains an array that lists the 
chunks processed during the given night number.

=cut
sub list_processed_chunks {
    # Initialize parameters
    my ($instance_name, $nn) = @_;
    my %results;    
    my %runs;       # Hash containing all of the runs done on the specified night.
    my $href;       # Reference to a row of data returned from the database.
    my $sth;        # SQL statement handle.
    my $rref;       # Reference to a 3 element array (survey mode, status, count)
                    # which represents a chunk that was processed during a run.
    
    # Connect to MOPS database.
	my $inst = PS::MOPS::DC::Instance->new(DBNAME => $instance_name);
	my $mops_dbh = $inst->dbh();
    
    # Create a sql query which will retrieve all of the chunks processed on the
    # given night number.
    my $sql = <<"SQL";
SELECT run_id, survey_mode, r.status, count(*) ct FROM runs r JOIN fields f
USING (field_id)
WHERE nn = $nn
GROUP BY run_id, survey_mode
SQL
    $sth = $mops_dbh->prepare($sql) or die $mops_dbh->errstr;
    
    # Execute the query and store results in a hash keyed on run_id
    $sth->execute() or die $sth->errstr;
    while ($href = $sth->fetchrow_hashref) {
        # Create a three element array containing survey mode, status, and count.
        $rref = [$href->{survey_mode}, $href->{status}, $href->{ct}];
        
        # Add array to the list of chunks processed for the run.
        push(@{$runs{$href->{run_id}}}, $rref);
    }
    
    # Return a reference to the runs hash which is keyed on run id and contains 
    # all of the chunks processed during the run.
    return \%runs;
}    

=head2
Sets the czared status in the runs table for a given runId and a given chunk to 1/true

Arguments:
  instance_name: name of the database
  nn: night number (unused)
  chunk: chunk name
  runId: the run_id
=cut
sub change_chunk_czar_status {
    my $instance_name = shift;
    my $nn = shift;
    my $chunk = shift;
    my $runId = shift;
    my @chunks = split(' ', $chunk);
    my @tmpChunks;
    for my $ch (@chunks) {
	push @tmpChunks, "'".$ch."'";
    }
    my $chunks = join(',', @tmpChunks);

    my $inst = PS::MOPS::DC::Instance->new(DBNAME => $instance_name);
    my $dbh = $inst->dbh();
    my $sql = <<"SQL";
UPDATE 
  runs 
  JOIN fields ON fields.field_id=runs.field_id
SET
  czared=1
WHERE 
  run_id=$runId
  AND
  survey_mode IN ($chunks)
SQL
my $sth = $dbh->prepare($sql) or die $dbh->errstr;
    my $n = $sth->execute() or die $sth->errstr;
    $sth->finish;
    return $n;
}

1;

__END__
=head1 AUTHOR

Denver Green, dgreen@ifa.hawaii.edu

=head1 COPYRIGHT AND LICENSE

Copyright 2012 Institute for Astronomy, University of Hawaii

=cut
