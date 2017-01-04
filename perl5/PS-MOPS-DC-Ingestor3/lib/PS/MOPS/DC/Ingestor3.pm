#!/usr/bin/env perl
package PS::MOPS::DC::Ingestor3;

=head1 NAME

PS::MOPS::DC::Ingestor3 - Ingest fields and detections into MOPS database

=head1 SYNOPSIS

use PS::MOPS::DC::Ingestor3

=head1 DESCRIPTION

This module contains routines for inserting collections of fields and
detections into a MOPS database.

=head1 FUNCTIONS

=head2 insert_ipp_fpa

Read a MOPS-IPP ICD-compliant FITS version 3 file into the MOPS database.

=head1 SEE ALSO

For detailed listing of the FITS column order, see PS::MOPS::FITS::IPP3

=cut

use strict;
use warnings;

use base qw(Exporter);

use FileHandle;
use File::Temp qw/tempdir tempfile/;
use File::Path;
use Data::Dumper;
use Cwd;

use Astro::SLA;
use Astro::Time;

use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib qw(:all);
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Field;
use PS::MOPS::DC::Detection;
use PS::MOPS::FITS::IPP3;               # IPP FITS definition
use PS::MOPS::IPPDB;                    # hand-query some stuff from IPP


our %EXPORT_TAGS = ( all => [ qw(
  insert_ipp_fpa
) ] );
our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );
our @EXPORT = qw(
  insert_ipp_fpa
);

use subs qw(
);

# Options.
our $VERSION = '0.01';
my $debug;
my $help;

# Field status management.
my $START_FIELD_STATUS = $FIELD_STATUS_INGESTED;
#my $START_FIELD_STATUS = $FIELD_STATUS_NEW;

# Convert PS1 MAG_SIGMA to S/N.
my $PS1_MAGSIG2SNR = 2.5 / log(10);     # approx 1.08574

my $SECONDS_IN_DAY = 86400;

# IPP flags.
our $PM_SOURCE_MODE2_ON_SPIKE = 0x8;        # flags2
our $PM_SOURCE_MODE2_OFF_CHIP = 0x8000_0000;# flags
our $DIFF_WITH_DOUBLE = 0x0000_0002;        # flags2
our $EXTENDED_FIT = 0x0080_0000;            # flags

# Removal codes.
our $REMOVE_SILLY = 'S';
our $REMOVE_NEARBRIGHT = 'B';
our $REMOVE_LOWS2N = 'L';
our $REMOVE_DIPOLE = 'D';
our $REMOVE_SPIKE = 'K';
our $REMOVE_DUPLICATE = '2';
our $REMOVE_IPPCLEAN = 'I';
our $REMOVE_BADOTA = 'O';

# Processing globals.
our $DIPOLE_DIST_THRESH_PX = 9.0;       # max dist to nearest bright star to be considered dipole


sub insert_ipp_fpa {
    # Insert an IPP FITS file representing a full FPA.  This is a temporary
    # routine until the ICD stabilizes.
    my ($inst, $input_filename, $remove_filename) = @_;
    my $mops_config = $inst->getConfig;
    my $mops_logger = $inst->getLogger;

    my $limiting_s2n = $mops_config->{site}->{limiting_s2n};
    my $low_confidence_s2n = $mops_config->{site}->{low_confidence_s2n};
    my $lsd_archive_flag = $mops_config->{main}->{enable_lsd};
    my $modes = $mops_config->{ingest}->{forbidden_modes};
    my $galactic_avoidance_deg = $mops_config->{ingest}->{galactic_avoidance_deg} || 0;

    croak("couldn't get limiting_s2n") unless defined($limiting_s2n);
    croak("couldn't get low_confidence_s2n") unless defined($low_confidence_s2n);

    my $ingest_nn = $mops_config->{ingest}->{ingest_nn};    # allow only exposures from specified NN
    my $ingest_date = $mops_config->{ingest}->{ingest_date};    # allow only exposures from specified NN
    # If ingest_date is defined, compute the night num and write into ingest_nn
    if ($ingest_date) {
        if ($ingest_date !~ /^(\d\d\d\d)\D+(\d+)\D+(\d+)$/) {
            die "ingest_date must have format YYYY-MM-DD";
        }
        my ($ingest_mjd) = `cal2jd --mjd $1 $2 $3`;
        chomp $ingest_mjd;
        $ingest_nn = mopslib_mjd2nn($ingest_mjd, $mops_config->{site}->{gmt_offset_hours});
    }

    # If we have a remove file, read its contents, which is a list of rows to remove.
    my %remove_tbl = ();
    if ($remove_filename) {
        my $rfh = new FileHandle $remove_filename or die "can't open $remove_filename";
        my $rownum;
        while ($rownum = <$rfh>) {
            chomp $rownum;
            $remove_tbl{$rownum} = 1;       # mark this row as removed
        }
        $rfh->close();
    }

    # Read input data file
    my $table = new PS::MOPS::FITS::IPP3;
    $table->openfile($input_filename);

    # Get exposure time in MJD.
    my $exptime_mjd = $table->keyval('EXPTIME') / $SECONDS_IN_DAY;
    
    # Get midpoint time of exposure.
    my $epoch_mjd = undef;
    if ($mops_config->{ingest}->{date_format} =~ m/^tai$/i) {
        my $mjd_obs = $table->keyval('MJD-OBS');
        $epoch_mjd = (`tico --tai_in=$mjd_obs --utc_out`)[0];     # ugh
        $epoch_mjd =~ s/^\s+|\s+$//g;       # strip leading whitespace
    }
    else {
        $epoch_mjd = $table->keyval('MJD-OBS');
    }

    my $epsilon = .00001;       # in days

    # IPP provenance information.
    my $fpa_id;
    my $exp_id;
    my $diff_id;

    if ($mops_config->{ingest}->{fpa_id_is_epoch_mjd}) {
        $fpa_id = sprintf("%.7f", $table->keyval('MJD-OBS'));   # override to handle bogus FPA_ID
    }
    else {
        $fpa_id = $table->keyval('EXP_NAME');
        $exp_id = $table->keyval('EXP_ID');
        $diff_id = $table->keyval('DIFF_ID');

    }

    my $survey_mode = $table->keyval('CMMTOBS');
    if ($survey_mode) {
        $survey_mode =~ s/\s.*$//;
    }
    else {
        $survey_mode = '';
    }
    my $obs_mode = $table->keyval('OBS_MODE');

    # Calculate the zero point magnitude 
    my $zero_point = $table->keyval('MAGZP');

    # Get pixel scale if available. If not then set it to a default value.
    my $pixel_scale = $table->keyval('PLTSCALE');
    if ($pixel_scale eq "" || $pixel_scale eq "NaN") {
        $pixel_scale = 0.25;
    }

    # Check to see if obs_mode of the exposure is in the list of forbidden modes.
    my $forbidden_mode = undef;
    if ($obs_mode && $modes) {
        if ($modes =~ m/$obs_mode/i) {
            $forbidden_mode = 'true';
        }
    }              

    # Convert diff type to an integer so that it can be stored in the de4 column.
    my $diff_type = $table->keyval('DIFFTYPE');
    
    if    ($diff_type =~ /WW/i) {$diff_type = $DIFF_TYPE_WW;} # warp - warp diff
    elsif ($diff_type =~ /WS/i) {$diff_type = $DIFF_TYPE_WS;} # warp - stack diff
    elsif ($diff_type =~ /SW/i) {$diff_type = $DIFF_TYPE_SW;} # stack - warp diff
    elsif ($diff_type =~ /SS/i) {$diff_type = $DIFF_TYPE_SS;} # stack - stack diff
    else                        {$diff_type = $DIFF_TYPE_UNSUPPORTED;} # Unsupported diff_mode
   
    my $pa_deg = $table->keyval('ROTANGLE') || 0.0;
    my $scale_factor = 1;
    my $field = PS::MOPS::DC::Field->new($inst,
        epoch => $epoch_mjd,
        ra => Astro::Time::str2deg($table->keyval('RA'), 'H'),
        dec => Astro::Time::str2deg($table->keyval('DEC'), 'D'),
        surveyMode => $survey_mode,
        timeStart => $epoch_mjd-0.5*$exptime_mjd,
        timeStop => $epoch_mjd+0.5*$exptime_mjd,
        status => $START_FIELD_STATUS,
        filter => substr($table->keyval('FILTER'), 0, 1),
        limitingMag => $table->keyval('LIMITMAG'),
        raSigma => 0.0,
        decSigma => 0.0,
        obscode => $table->keyval('OBSCODE'),
        # de1 - seeing(fwhm), de2 - RMS of astrometric fit, de3 - zero point as magnitude
        # de4 - diff type, de5 - avg sky background, de6 - S/N scale factor 
        # de7 - unused de8 - unused, de9 - unused, de10 - pixel/plate scale
        de => [ $table->keyval('SEEING'), $table->keyval('ASTRORMS'), $zero_point, 
            $diff_type, $table->keyval('SKY'), $scale_factor, $exptime_mjd, 
            $table->keyval('DE8'), $table->keyval('DE9'), $pixel_scale ],
        ocnum => 0.0,
        fpaId => $fpa_id,
        expId => $exp_id,
        diffId => $diff_id,
        pa_deg => $pa_deg,
    );

    my $found = modcf_retrieve($inst, begin_mjd => $epoch_mjd - $epsilon, end_mjd => $epoch_mjd + $epsilon);

    if (defined($ingest_nn) and $field->nn != $ingest_nn) {
        $mops_logger->warn("not ingesting $fpa_id due to ingest_nn=$ingest_nn");
    }
    elsif ($forbidden_mode) {
        # Observation mode of the exposure does not match any of the modes 
        # specified by the survey mode configuration parameter.
        $mops_logger->warn("not ingesting $fpa_id as it is from the $obs_mode survey which is forbidden; skipping");
    }
    elsif ($galactic_avoidance_deg && abs($field->gb_deg) < $galactic_avoidance_deg) {
        $mops_logger->warn(sprintf "$fpa_id has galactic lat < %.1f; skipping", $galactic_avoidance_deg);
    }
    elsif (!$found or !$found->next()) {
        my $num_added;
        my $dbh = $inst->dbh();
        $dbh->{AutoCommit} = 0;
        $inst->atomic($dbh, sub {
            eval {
                $field->insert();
    
                if (1 and $mops_config->{main}->{enable_lsd}) {
                    # If LSD process is enabled, use C executables to push the LSDs
                    # around, as Perl is too slow.
                    $table->closefile();        # will re-open in C-land.
                    $num_added = do_field_C_land($field, $input_filename, $inst, 
                        $limiting_s2n, $low_confidence_s2n, $lsd_archive_flag,
                        $mops_config->{ingest},
                        $field->epoch, $field->filter); 
                }
                else {
                    $num_added = do_ipp_fpa($field, $table, $inst, 
                        $limiting_s2n, $low_confidence_s2n, $lsd_archive_flag,
                        $mops_config->{ingest}, \%remove_tbl);
                    $table->closefile();
                }
    
                # XXX Let's test DB tx fail here.
                # die "aggh!"
            };
            if ($@) {
                $dbh->rollback();
                $mops_logger->logdie($@) if $@;
            }
            else {
                $dbh->commit();
            }
        });

#        if ($num_added > 0 and !$debug)  {
#            $mops_logger->info(sprintf "Ingested field %d.",
#                $field->fieldId, $num_added);
#        }
        $mops_logger->info(sprintf "INGEST: created %s %s", $field->fpaId, $field->surveyMode);
    }
    else {
        $mops_logger->warn("INGEST: already found field for epoch $epoch_mjd; skipping");
    }

    return {
        RESULTS => 0,
        NN => $field->nn,
    };
}


sub do_ipp_fpa {
    # Read in IPP detections from a FITS file.  Set aside all detections
    # below the configured low-significance S/N limit.  If $lsd_archive_flag
    # is enabled, then write these detections to the archive for later processing.
    #
    my ($field, $table, $inst, $limiting_s2n, $lsd_s2n, $lsd_archive_flag, $extra_options, $remove_href) = @_;
    my $row_num;
    my $num_dets_inserted = 0;
    my $mops_config = $inst->getConfig;
    my $mops_logger = $inst->getLogger();
    my $do_trailfit = $mops_config->{main}->{enable_trailfit} || 0;
    my @hc_dets;
    my @lc_dets;                       # low-significance detections

    # Scan our extra options for other processing rules.
    my $skip_zero_fluxes;
    if ($extra_options->{skip_zero_fluxes}) {
        $skip_zero_fluxes = 1;
    }

    # Astrometry ingest setup.
    my $astro_floor_arcsec = $extra_options->{astrometry}->{floor_arcsec};
    my $astro_max_arcsec = $extra_options->{astrometry}->{max_arcsec};
    my $astro_force_arcsec = $extra_options->{astrometry}->{force_arcsec};
    my $astro_floor_deg;
    my $astro_max_deg;
    my $astro_force_deg;

    # If this is set, keep all detections in DB, even removed ones.  They're
    # indicated as removed using a non-"F" status.
    my $keep_detections = $extra_options->{keep_detections};

    # If force_arcsec is not defined (shouldn't be ever eventually), then make
    # sure floor/max are defined.
    if (!defined($astro_force_arcsec)) {
        die "floor_arcsec not defined" unless defined($astro_floor_arcsec);
        die "max_arcsec not defined" unless defined($astro_max_arcsec);
        $astro_floor_deg = $astro_floor_arcsec / 3600;
        $astro_max_deg = $astro_max_arcsec / 3600;
    }
    else {
        $astro_force_deg = $astro_force_arcsec / 3600;
    }


    eval {
        my @all_detections;
        my %remove_tbl;

        my ($fra_rad, $fdecl_rad);  # false RA, DEC in RAD
        my ($fra_deg, $fdecl_deg);  # false RA, DEC in DEG
        my ($field_dra_rad, $field_ddecl_rad);
        my $det;
        my $flux;
        my $flux_sigma;
        my $mag;
        my $mag_sigma;
        my $S2N;
        my $ra_sigma_deg;
        my $dec_sigma_deg;
        my $flags;
        my $starpsf;
        my $obscode;
        my $diff_pos;
        my $orient_deg;
        my $orient_sig_deg;
        my $length_deg;
        my $length_sig_deg;
        my $det_status;
        my $proc_id;
        my $rawattr; # extended raw attribute data
        my $cmfver; # version of fits file
        my $pixel_scale;

        # Array index values for accessing extended raw attributes.
        my $INDEX = 0;
        my $RA = 1;
        my $RA_ERR = 2;
        my $DEC = 3;
        my $DEC_ERR = 4;
        my $PSF_MAJOR = 4;
        my $MAG = 5;
        my $PSF_MINOR = 5;
        my $MAG_ERR = 6;
        my $PSF_THETA = 6;
        my $EXT_SIGNIFICANCE = 7;
        my $FLAGS = 8;
        my $DIFF_SKYFILE_ID = 9;
        my $RAWATTR_V3 = 10;
        my $PSF_CHI2 = 0;
        my $PSF_DOF = 1;
        my $PSF_QUALITY = 7;
        my $CAL_PSF_MAG = 27;
        my $CAL_PSF_MAG_SIG = 28;
        my $PSF_INST_FLUX = 19;
        my $EXT_WIDTH_MAJ = 51;
        my $EXT_WIDTH_MIN = 52;
        my $EXT_WIDTH_MAJ_ERR = 54;
        my $EXT_WIDTH_MIN_ERR = 55;
        my $RA_EXT = 57; 
        my $RA_EXT_SIGMA = 58;
        my $DEC_EXT = 59;
        my $DEC_EXT_SIGMA = 60;        
        my $EXT_THETA = 53;
        my $EXT_THETA_ERR = 56;
        my $EXT_FLUX = 62;
        my $EXT_CAL_MAG = 63;
        my $EXT_MAG_SIG = 64;
        my $EXT_CHISQ = 65;
        my $EXT_NDOF = 66;       
        
        $field_dra_rad = 0.0;
        $field_ddecl_rad = 0.0;

        $obscode = $table->keyval('OBSCODE');
        $diff_pos = $table->keyval('DIFF_POS') || 'T';
        
        # Get pixel scale if available. If not then set it to a default value.
        $pixel_scale = $table->keyval('PLTSCALE');
        if ($pixel_scale eq "" || $pixel_scale eq "NaN") {
            $pixel_scale = 0.25;
        }

        # Determine version of fits file. 
        $cmfver = $table->keyval('HIERARCH CMFVERSION');
        
        while (my @data = $table->readrecord()) {
            push @all_detections, \@data;
        }
        $mops_logger->info(sprintf "Read %d raw detections for %s.", scalar @all_detections, $field->fpaId);



        # We have all detections in memory.  Perform various stages of filtering, in 
        # the following order:
        #
        # 0. Bad OTAs (with "too many" detections, indicating astrometry failure)
        # 1. Silly removal
        # 2. Low S/N removal
        # 3. Dipole removal
        # 4. Large-scale density filtering to clean bad skycells
        # 5. Small-scale density filtering to clean starwing crud
        #
        # After each filtering stage we will maintain a list of detections to remove,
        # and after all filtering is done, we will insert either filtered detections
        # or all detections + filter reason.
        unless ($extra_options->{no_ipp_filter}) {
            if ($extra_options->{remove_bad_otas}) {
                my $max_dets_per_ota = $extra_options->{max_dets_per_ota} || 1000;
                remove_bad_otas($mops_logger, $max_dets_per_ota, $field, \@all_detections, \%remove_tbl);
            }
            remove_silly($mops_logger, \@all_detections, \%remove_tbl);
            remove_nearbright($mops_logger, \@all_detections, \%remove_tbl) if $extra_options->{remove_nearbright};
            remove_s2n($mops_logger, \@all_detections, \%remove_tbl, $limiting_s2n);
            dedupe($mops_logger, \@all_detections, \%remove_tbl);
            remove_dipoles($mops_logger, \@all_detections, \%remove_tbl) if $extra_options->{remove_dipoles};
            ipp_clean($mops_logger, \@all_detections, \%remove_tbl, $extra_options) if $extra_options->{density_filter};  # density filtering
        }


        # Now we can insert.
        foreach my $detref (@all_detections) {
            my @data = @{$detref};
            $row_num = $data[$INDEX];
            next if (!$keep_detections && $remove_tbl{$row_num});   # remove table contains rownum, so skip

            $rawattr = $data[$RAWATTR_V3];
            # Determine if extended fit parameters were generated, by checking
            # the EXTENDED_FIT flag
            $flags = $data[$FLAGS];
            if (($do_trailfit && $flags) & $EXTENDED_FIT) {
                # Extended fit was done so use extended parameters.
                $fra_deg = $rawattr->[$RA_EXT];
                $ra_sigma_deg = $rawattr->[$RA_EXT_SIGMA];
                $fdecl_deg = $rawattr->[$DEC_EXT];
                $dec_sigma_deg = $rawattr->[$DEC_EXT_SIGMA];
                $mag = $rawattr->[$EXT_CAL_MAG];
                $mag_sigma = $rawattr->[$EXT_MAG_SIG];
                $starpsf = $data[$EXT_SIGNIFICANCE];
                $orient_deg = Astro::Time::rad2deg($rawattr->[$EXT_THETA]) + 90;
                $orient_sig_deg = Astro::Time::rad2deg($rawattr->[$EXT_THETA_ERR]);
                $length_deg = $rawattr->[$EXT_WIDTH_MAJ] * $pixel_scale / 3600;
                $length_sig_deg = $rawattr->[$EXT_WIDTH_MAJ_ERR] * $pixel_scale / 3600;
                $rawattr->[$CAL_PSF_MAG_SIG] = $rawattr->[$EXT_MAG_SIG];
                $rawattr->[$CAL_PSF_MAG] = $rawattr->[$EXT_CAL_MAG];
                $rawattr->[$PSF_THETA] = $rawattr->[$EXT_THETA];
                $rawattr->[$PSF_INST_FLUX] = $rawattr->[$EXT_FLUX];
            }
            else {
                # Extended fit was NOT done so use non-extended parameters.
                $fra_deg = $data[$RA];
                $ra_sigma_deg = $data[$RA_ERR];
                $fdecl_deg = $data[$DEC];
                $dec_sigma_deg = $data[$DEC_ERR];
                $mag = $data[$MAG];
                $mag_sigma = $data[$MAG_ERR];
                $starpsf = $data[$EXT_SIGNIFICANCE];
                
                # Provided only when extended fit is done.
                $orient_deg = 0;
                $orient_sig_deg = 0;
                $length_deg = 0;
                $length_sig_deg = 0;
            }
            $fra_deg += 360 if ($fra_deg < 0);  # wrap if < 0
            $orient_deg += 360 if ($orient_deg < 0);

            $proc_id = ($diff_pos eq 'T') ? $data[$DIFF_SKYFILE_ID] : -$data[$DIFF_SKYFILE_ID];      # flip sign if negative diff
            $det_status = $remove_tbl{$row_num} || $DETECTION_STATUS_FOUND;     # removal status if removed, or FOUND

            # Rewrite sigmas to nominal values.
            if (defined($astro_force_deg)) {
                $ra_sigma_deg = $astro_force_deg;
                $dec_sigma_deg = $astro_force_deg;
            }
            else {
                # Add reported sigmas to floor in quadrature.
                $ra_sigma_deg = sqrt($astro_floor_deg * $astro_floor_deg + $ra_sigma_deg * $ra_sigma_deg);
                $ra_sigma_deg = $astro_max_deg if $ra_sigma_deg > $astro_max_deg;
                $dec_sigma_deg = sqrt($astro_floor_deg * $astro_floor_deg + $dec_sigma_deg * $dec_sigma_deg);
                $dec_sigma_deg = $astro_max_deg if $dec_sigma_deg > $astro_max_deg;
                next if $ra_sigma_deg > $astro_max_deg or $dec_sigma_deg > $astro_max_deg;
            }

            $mag_sigma = $mag_sigma != 0 ? $mag_sigma : 0.001;  #Prevents divide by zero errors
            $S2N = $PS1_MAGSIG2SNR / $mag_sigma;
            $det = PS::MOPS::DC::Detection->new(
                $inst,
                ra => $fra_deg,
                dec => $fdecl_deg,
                epoch => $field->epoch,
                mag => $mag,
                refMag => $mag,
                filter => $field->filter,
                s2n => $S2N,
                isSynthetic => 0,
                orient_deg => $orient_deg,
                length_deg => $length_deg,
                raSigma => $ra_sigma_deg,
                decSigma => $dec_sigma_deg,
                magSigma => $mag_sigma,
                obscode => $obscode,
                status => $det_status,
                objectName => $MOPS_NONSYNTHETIC_OBJECT_NAME,
                detNum => $row_num,         # IPP FITS det number
                procId => $proc_id,
                rawattr_v2 => $rawattr,
            );
            push @hc_dets, $det;

        }   # while
        
        # Calculate and save s2n scale factor which will be used to calculate the
        # s/n of any synthetics inserted into the field.
        calc_scale_factor($field, \@hc_dets, $inst);
        
        # Write all to database.
        $field->addDetections(@hc_dets);

        # Archive LSDs.
#        if ($lsd_archive_flag and @lc_dets) {
        if ($lsd_archive_flag) {
            use PS::MOPS::LSD;
            PS::MOPS::LSD::ArchiveNSDField($field, \@lc_dets);      # write all LSDs to archive
            PS::MOPS::LSD::ArchiveNSDField($field, \@hc_dets);      # append HSDs
        }
    };

    warn $@ if $@;
    return scalar @hc_dets;
}

sub calc_scale_factor{
    # Calculate the average S/N scaling factor used when generating a S2N 
    # for synthetics
    my ($field, $dets_aref, $inst) = @_;
    my $s2n_calc = 0;
    my $scale_factor = 0;
    my $count = 0;
    my $PSF_QUALITY_CUTOFF = 0.9;
    my $PSF_CHISQ_DOF_LOW = 0.9;
    my $PSF_CHISQ_DOF_HIGH = 1.1;
    my $PSF_CHISQ_DOF = 0;
    my $PSF_CHI2 = 0;
    my $PSF_DOF = 1;
    my $PSF_QUALITY = 7;

    foreach my $det (@{$dets_aref}) {
        # Use only detections that have been flagged as found for generating scale factor.
        next if ($det->status ne $DETECTION_STATUS_FOUND);
        
        # Use only high quality detections to generate scale factor.
        next if ($det->rawattr_v2->[$PSF_QUALITY] < $PSF_QUALITY_CUTOFF);
        
        $PSF_CHISQ_DOF = $det->rawattr_v2->[$PSF_CHI2] / $det->rawattr_v2->[$PSF_DOF];
        next if ($PSF_CHISQ_DOF < $PSF_CHISQ_DOF_LOW or $PSF_CHISQ_DOF > $PSF_CHISQ_DOF_HIGH);
        $s2n_calc = mopslib_photoS2N5(mag => $det->mag, 
            zero_point => $field->de->[2], sky => $field->de->[4], 
            seeing => $field->de->[0]);
        $scale_factor += $det->s2n / $s2n_calc;
        $count++;                
    }
    if ($count == 0 ) {
        $scale_factor = 1;
    } else {
        $scale_factor /= $count;
    }
    
    # Write scale factor to database.
    my $dbh = $inst->dbh();
    $dbh->do(<<"SQL", undef, $scale_factor, $field->fieldId) or croak $dbh->errstr;
update `fields` set de6=? where field_id=?
SQL
}
    
sub do_field_C_land {
    # Invoke a C program to snarf the FITS file as fast as possible.  This program will 
    # archive all detections to the LSD archive, and split off the high-confidence detections
    # into a private packed binary file so that we can insert them into the DB.
    my ($field, $input_filename, $inst, $limiting_s2n, $lsd_s2n, $lsd_archive_flag, $extra_options, $epoch_mjd, $filter) = @_;
    my $det_num = 0;    # night-unique NSD ID

    # Scan our extra options for other processing rules.
    my $skip_zero_fluxes;
    if ($extra_options->{skip_zero_fluxes}) {
        $skip_zero_fluxes = 1;
    }

    # Astrometry ingest setup.
    my $astro_floor_arcsec = $extra_options->{astrometry}->{floor_arcsec};
    my $astro_max_arcsec = $extra_options->{astrometry}->{max_arcsec};
    my $astro_force_arcsec = $extra_options->{astrometry}->{force_arcsec};
    my $astro_floor_deg;
    my $astro_max_deg;
    my $astro_force_deg;

    # If force_arcsec is not defined (shouldn't be ever evantually), then make
    # sure floor/max are defined.
    if (!defined($astro_force_arcsec)) {
        die "floor_arcsec not defined" unless defined($astro_floor_arcsec);
        die "max_arcsec not defined" unless defined($astro_max_arcsec);
        $astro_floor_deg = $astro_floor_arcsec / 3600;
        $astro_max_deg = $astro_max_arcsec / 3600;
    }
    else {
        $astro_force_deg = $astro_force_arcsec / 3600;
    }

    # Photometry.
    my $photo_min_mag = $extra_options->{photometry}->{min_mag};
    die "photo_min_mag not defined" unless defined($photo_min_mag);
    my $photo_max_mag = $extra_options->{photometry}->{max_mag};
    die "photo_max_mag not defined" unless defined($photo_max_mag);


    # Get LSD archive information.
    my $ingest_path;
    if ($lsd_archive_flag) {
        my $ingest_dir = $inst->makeNNDir(NN => $field->nn, SUBSYS => 'lsdingest');
        $ingest_path = $ingest_dir . '/' . $field->fieldId . '.nsd.src.bin';
    }

    # Execute our C program.
    my $hc_tempnam = File::Temp::tempnam('/tmp', $inst->dbname . '.hc.XXXXXXX');
    my @cmd = (
        'splitIPP', 
        '--field_id', $field->fieldId,
        '--hc_file', $hc_tempnam, 
        '--lc_file', $ingest_path, 
        '--hc_cutoff', $limiting_s2n,
        '--lc_cutoff', $lsd_s2n,
        '--epoch_mjd', $epoch_mjd,
        '--filter', $filter,
        $input_filename,
    );
    $inst->getLogger()->info('EXEC ' . join(' ', @cmd));
    system(@cmd) == 0 or die "$?: @cmd";
    die "LC file was not created" unless -f $ingest_path;


    # Now read the HC detections written by the C program and unpack.
    my $ra_deg;
    my $ra_sigma_deg;
    my $dec_deg;
    my $dec_sigma_deg;
    my $det;
    my $mag;
    my $mag_sigma;
    my $s2n;
    my $orient_deg;
    my $orient_sig;
    my $length_deg;
    my $length_sig;
    my $proc_id;
    my $obscode = $field->obscode;
    my @hc_dets;

    # This format must match splitIPP's HC_ROW_T.
    my $hc_struct = 'dddddddddddqq';   # ra ra_sig dec dec_sig mag mag_sig s2n ang ang_sig len len_sig det_num proc_id
    my $hc_struct_len = (length $hc_struct) * 8;
    my $file_offset;
    my $data;
    my $hc_filesize = -s $hc_tempnam;
    open FOO, $hc_tempnam or die "can't open $hc_tempnam";
    my $num_read = read FOO, $data, $hc_filesize;
    close FOO;
    unlink $hc_tempnam or warn "couldn't unlink $hc_tempnam";
    die "strange file length for $hc_tempnam" unless ($hc_filesize % $hc_struct_len) == 0;
    my $hc_num_dets = $hc_filesize / $hc_struct_len;
    my $n = 0;

    while ($n < $hc_num_dets) {
        (
            $ra_deg, $ra_sigma_deg,
            $dec_deg, $dec_sigma_deg,
            $mag, $mag_sigma, $s2n,
            $orient_deg, $orient_sig,
            $length_deg, $length_sig,
            $det_num, $proc_id
        ) = unpack $hc_struct, substr $data, $n * $hc_struct_len, $hc_struct_len;
        $n++;
  
        next if $ra_sigma_deg eq 'nan' or $dec_sigma_deg eq 'nan' or $mag_sigma eq 'nan';
        next if $ra_sigma_deg <= 0 or $dec_sigma_deg <= 0 or $mag_sigma <= 0;
        next if $mag < $photo_min_mag or $mag > $photo_max_mag;

        # Rewrite sigmas to nominal values.
        if (defined($astro_force_deg)) {
            $ra_sigma_deg = $astro_force_deg;
            $dec_sigma_deg = $astro_force_deg;
        }
        else {
            # Add reported sigmas to floor in quadrature.
            $ra_sigma_deg = sqrt($astro_floor_deg * $astro_floor_deg + $ra_sigma_deg * $ra_sigma_deg);
            $ra_sigma_deg = $astro_max_deg if $ra_sigma_deg > $astro_max_deg;
            $dec_sigma_deg = sqrt($astro_floor_deg * $astro_floor_deg + $dec_sigma_deg * $dec_sigma_deg);
            $dec_sigma_deg = $astro_max_deg if $dec_sigma_deg > $astro_max_deg;
            next if $ra_sigma_deg > $astro_max_deg or $dec_sigma_deg > $astro_max_deg;
        }

        push @hc_dets, PS::MOPS::DC::Detection->new(
            $inst,
            ra => $ra_deg,
            dec => $dec_deg,
            epoch => $field->epoch,
            mag => $mag,
            refMag => $mag,
            filter => $field->filter,
            s2n => $s2n,
            isSynthetic => 0,
            orient_deg => $orient_deg,
            length_deg => $length_deg,
            raSigma => $ra_sigma_deg,
            decSigma => $dec_sigma_deg,
            magSigma => $mag_sigma,
            obscode => $obscode,
            objectName => $MOPS_NONSYNTHETIC_OBJECT_NAME,
            detNum => $det_num,         # IPP FITS det number
            procId => $proc_id,
        );
    }   # for

    # Write all to database.
    $field->addDetections(@hc_dets);
    return $hc_num_dets;
}


sub remove_silly {
    my ($ml, $dref, $rref) = @_;     # data ARRAYREF, remove HASHREF
    my $start = scalar keys %{$rref};
    my $photo_min_mag = 0;
    my $photo_max_mag = 40;
    my $silly;

    foreach my $row (@{$dref}) {
        my $row_num = $row->[0];
        next if exists ${$rref}{$row_num};      # already removed

        # See IPP-FITS3 for column references.
        my ($mag, $ra_sigma_deg, $dec_sigma_deg, $mag_sigma, $flags, $rawattr) = @{$row}[5, 2, 4, 6, 8, 10];

        # Filter on flags.  If the detection fails due to dipole parameters, the failure
        # is handled in readrecord() and $flags ($data[11]) is set.
        # 0x3888 is a IPP-recommended flag and corresponds to the following conditions.
        #   Fit (non-linear) failed (non-converge, off-edge, run to zero)
        #   Source model peak is above saturation (saturated star)
        #   Source is thought to be a defect
        #   Source is thought to be saturated pixels (bleed trail)
        #   Source has crNsigma above limit (source is a cosmic ray)
        $silly = (
            ($flags & 0x3888) 
            || ($ra_sigma_deg eq 'nan' || $dec_sigma_deg eq 'nan' || $mag_sigma eq 'nan') 
            || ($ra_sigma_deg <= 0 || $dec_sigma_deg <= 0 || $mag_sigma <= 0) 
            || (($mag < $photo_min_mag) || ($mag > $photo_max_mag)) 
        );

        if (!$silly && $rawattr) {
            # Look at extended parameters as well.  See PS::MOPS::FITS::IPP3
            my ($ap_mag, $mxx, $mxy, $myy, $psf_qf_perfect) = @{$rawattr}[21, 9, 10, 11, 31];
            $silly = (
                ($ap_mag eq 'nan')
                || (!$mxx || !$myy || !$mxy)
#                || ($psf_qf_perfect < .01)     # apparently we can no longer rely on this
            );
        }

        $rref->{$row_num} = $REMOVE_SILLY if $silly;
    }

    $ml->info(sprintf "%d silly detections removed, %d remain.", (scalar keys %{$rref}) - $start, (scalar @{$dref}) - (scalar keys %{$rref}));
}


sub remove_nearbright {
    my ($ml, $dref, $rref) = @_;     # data ARRAYREF, remove HASHREF
    my $start = scalar keys %{$rref};

    foreach my $row (@{$dref}) {
        my $row_num = $row->[0];
        next if exists ${$rref}{$row_num};      # already removed

        # See IPP-FITS2 for column references.
        my ($mag, $ra_sigma_deg, $dec_sigma_deg, $mag_sigma, $flags, $rawattr) = @{$row}[5, 2, 4, 6, 8, 10];
        my $remove;
        my $dipole;

        # Filter on flags.  If the detection fails due to dipole parameters, the failure
        # is handled in readrecord() and $flags ($data[11]) is set.
        if (!$remove && $rawattr) {
            # Look at extended parameters as well.  See PS::MOPS::FITS::IPP3
            my ($diff_r_m, $diff_sn_m) = @{$rawattr}[40, 41];
            $remove = ($diff_sn_m ne 'nan' && $diff_r_m ne 'nan' && ($diff_sn_m > 5 && $diff_r_m > .01 && $diff_r_m < 5));
        }
        $rref->{$row_num} = 'B' if $remove;
    }

    $ml->info(sprintf "%d near-bright detections removed, %d remain.", (scalar keys %{$rref}) - $start, (scalar @{$dref}) - (scalar keys %{$rref}));
}


sub remove_s2n {
    my ($ml, $dref, $rref, $min_s2n) = @_;     # data ARRAYREF, remove HASHREF
    my $lows2n;
    my $start = scalar keys %{$rref};

    foreach my $row (@{$dref}) {
        my $row_num = $row->[0];
        next if exists ${$rref}{$row_num};      # already removed

        my ($mag, $mag_sigma) = @{$row}[5, 6];
        my $s2n = $PS1_MAGSIG2SNR / $mag_sigma;
        $rref->{$row_num} = $REMOVE_LOWS2N if $s2n < $min_s2n;
    }

    $ml->info(sprintf "%d detections removed with S/N < %.2f, %d remain.", (scalar keys %{$rref}) - $start, $min_s2n, (scalar @{$dref}) - (scalar keys %{$rref}))
}


sub remove_dipoles {
    my ($ml, $dref, $rref) = @_;     # data ARRAYREF, remove HASHREF
    my $dipole;
    my $start = scalar keys %{$rref};
    #exclude PSF_QF < 0.9
    #exclude PSF_QF_PERFECT < 0.9 (or maybe 0.95)
    #exclude (DIFF_R_P < 20) && finite
    #exclude (DIFF_R_M < 20) && finite

    foreach my $row (@{$dref}) {
        my $row_num = $row->[0];
        next if exists ${$rref}{$row_num};      # already removed

        my ($mag_sigma, $rawattr) = @{$row}[6, 10];
        my $s2n = $PS1_MAGSIG2SNR / $mag_sigma;

        if ($rawattr) {
            my ($diff_r_p, $diff_sn_p, $diff_r_m, $diff_sn_m, $flags2) = @{$rawattr}[38, 39, 40, 41, 42];
            $dipole = (
                ($flags2 & $DIFF_WITH_DOUBLE)
                and (
                    ($diff_r_p ne 'nan' and $diff_r_p > 0 
                     and $diff_r_p < $DIPOLE_DIST_THRESH_PX 
                     and $diff_sn_p > (5 * $s2n)
                    )
                    and 
                    ($diff_r_m ne 'nan' and $diff_r_m > 0 
                     and $diff_r_m < $DIPOLE_DIST_THRESH_PX 
                     and $diff_sn_m > (5 * $s2n)
                    )
                )
            );
            $rref->{$row_num} = $REMOVE_DIPOLE if $dipole;
        }
        else {
            # V1 format unsupported.
        }
    }

    $ml->info(sprintf "%d dipole detections removed, %d remain.", (scalar keys %{$rref}) - $start, (scalar @{$dref}) - (scalar keys %{$rref}))
}


sub dedupe {
    my ($ml, $dref, $rref) = @_;     # data ARRAYREF, remove HASHREF
    my $start = scalar keys %{$rref};
    scrub($dref, $rref, $REMOVE_DUPLICATE, 1, .00027);        # dedupe params, want <= 1 det per .00027deg radius
    $ml->info(sprintf "%d duplicate detections removed, %d remain.", (scalar keys %{$rref}) - $start, (scalar @{$dref}) - (scalar keys %{$rref}));
}


sub ipp_clean {
    my ($ml, $dref, $rref, $extra_options) = @_;     # data ARRAYREF, remove HASHREF
    my $density_perdeg2 = $extra_options->{density_perdeg2} || 50000;
    my $density_radius_deg = $extra_options->{density_radius_deg} || 0.01;
    my $start = scalar keys %{$rref};
    $ml->info(sprintf "Cleaning detections with density %.2f and radius %.2f deg.", $density_perdeg2, $density_radius_deg);
    scrub($dref, $rref, $REMOVE_IPPCLEAN, $extra_options->{density_perdeg2} || 50000, $extra_options->{density_radius_deg} || 0.01);
    $ml->info(sprintf "%d detections cleaned, %d remain.", (scalar keys %{$rref}) - $start, (scalar @{$dref}) - (scalar keys %{$rref}));
}


sub scrub {
    # Process the current set of detections in $stuff, send it to the cleaners, and
    # remove anything that was scrubbed by astroclean.
    my ($dref, $rref, $scrub_status, $density_perdeg2, $density_radius_deg) = @_;
    my $dir = tempdir(CLEANUP => 1, DIR => '/tmp');
    my ($ac_infh, $ac_infilename) = tempfile(DIR => $dir);
    my ($row_num, $ra_deg, $dec_deg, $mag_sigma, $rawattr);
    my $fom;
    my $row;
    my %d1_keep_stuff;

    my $fake_epoch_mjd = '0.0000';
    my $fake_obscode = 'PS1';

    # Note that we pass mag_sigma as the mag.  This is because astroclean selects the "brightest" 
    # in reducing density, meaning smaller mags.  But we want our FOM to be a quality selector,
    # so we'll select based on smallest mag_sigma.
    foreach $row (@{$dref}) {
        ($row_num, $ra_deg, $dec_deg, $mag_sigma, $rawattr) = @{$row}[0, 1, 3, 6, 10];
        if ($rawattr) {
            $fom = 1 / ($rawattr->[7] || .1);        # 7 => PSF_QUALITY; larger => worse 
        }
        else {
            $fom = $mag_sigma;
        }
        next if exists ${$rref}{$row_num};      # already removed
        print $ac_infh join(' ', $row_num, $fake_epoch_mjd, $ra_deg, $dec_deg, $fom, $fake_obscode), "\n";
    }
    $ac_infh->close();

    my $OUTTYPE_MITI = 1;
    my ($ac_outfh, $ac_outfilename) = tempfile(DIR => $dir);
    $ac_outfh->close();                         # just need filename, not fh

#astroclean file $ac_infilename outtype 1 density 1000 Dradius .0010 proxrad $proxrad_thresh_deg clean_file $ac_outfilename
    my $cmd = <<"EOF";
astroclean file $ac_infilename outtype $OUTTYPE_MITI density $density_perdeg2 Dradius $density_radius_deg clean_file $ac_outfilename > $dir/ac.log
EOF
    system($cmd) == 0 or die "command failed: $? : $cmd";

    # Read output file into table, return table.
    $ac_outfh = new FileHandle $ac_outfilename;
    my $line;
    my $remainder;
    my ($epoch_mjd, $obscode);
    my $prefix;
    while (defined($line = <$ac_outfh>)) {
        next if $line =~ /^(!|#)/;   # comment, skip
        chomp $line;
        ($row_num, $remainder) = split /\s+/, $line, 2;
        $d1_keep_stuff{$row_num} = 1;     # strip leading character to get det ID
    }
    $ac_outfh->close();

    unlink $ac_infilename;
    unlink $ac_outfilename;
    unlink "$dir/ac.log";
    rmdir $dir or die "can't remove dir $dir";

    # Now walk through our table of stuff and throw out anything that's not in the keep table.
    foreach $row (@{$dref}) {
        $row_num = $row->[0];
        next if exists ${$rref}{$row_num};      # already removed
        $rref->{$row_num} = $scrub_status unless exists($d1_keep_stuff{$row_num});
    }
}


sub remove_bad_otas {
    # Steps.
    # * Create temp dir
    # * write out RA, DEC to files
    # * call pscoords
    # * count per OTA
    # * filter
    my ($ml, $maxdets, $field, $dref, $rref) = @_;     # data ARRAYREF, remove HASHREF
    my $maxdets2 = $maxdets / 2;
    my $oldcwd = getcwd;
    my $start = scalar keys %{$rref};
    $ml->info("INGEST " . $field->fpaId . " max_dets_per_ota=$maxdets");
    eval {
        my $dir = tempdir('pscoordsXXXXX', CLEANUP => 1, DIR => '/tmp');
        chdir $dir or die "can't chdir to $dir";
        
        # Write all dets to file.
        my $infn = 'in.coords';
        my $fh = new FileHandle ">$infn" or die "can't create $dir/$infn";
        my $det;
        foreach $det (@{$dref}) {
            # my ($mag, $ra_sigma_deg, $dec_sigma_deg, $mag_sigma, $flags, $rawattr) = @{$row}[5, 2, 4, 6, 8, 10];
            print $fh join(' ', $det->[1], $det->[3]), "\n";
        }
        $fh->close();

        # pscoords it.
        my ($field_ra_deg, $field_dec_deg, $field_pa_deg) = ($field->ra, $field->dec, $field->pa_deg);
        my $pfh = new FileHandle "pscoords out=ota in=sky ra=$field_ra_deg dec=$field_dec_deg pa=$field_pa_deg dx=0 dy=0 dpa=0 optics=ps < $infn |" or die "can't create pscoords filehandle";
        my @res = <$pfh>;
        $pfh->close();

        # Count stuff and assign OTA # to each detection.
        my %otact;          # count of dets per OTA
        my @detota;         # OTA of dets by det num
        my %badota;         # simple table of bad OTAs
        my ($ota, $dummy);
        my $detn = 0;
        for my $row (@res) {
            ($ota, $dummy) = split /\s+/, $row, 2;      # each line is OTA X Y
            $otact{$ota}++;

            # Save the det-OTA association.  We have to keep a counter that matches each row of
            # output from pscoords to a detection in the input $dref list.
            $detota[$detn++] = $ota;
        }
        $ml->info("INGEST " . $field->fpaId . " OTAs " . (join ' ', map { "$_=$otact{$_}" } sort keys %otact));

        # Now look for badly-behaving OTAs.  The detection threshold count should configurable.
        foreach my $k (keys %otact) {
            if ($k eq '17' or $k eq '76') {
                # Calcite mounts.
                if ($otact{$k} > $maxdets2) {
                    $badota{$k} = 1;
                }
            }
            elsif ($otact{$k} > $maxdets) {
                $badota{$k} = 1;
            }
        }
        $ml->info("INGEST " . $field->fpaId . " BAD OTAs " . (join ' ', sort keys %badota));

        # Now re-scan the detetions and mark the ones to remove because they are in a bogus OTA.
        $detn = 0;
        my $row_num;
        for $det (@{$dref}) {
            if ($detota[$detn] && $badota{$detota[$detn]}) {
                # Reject det.
                $row_num = $det->[0];
                $rref->{$row_num} = $REMOVE_BADOTA;
            }
            $detn++;
        }
        $ml->info(sprintf "%d detections from bad OTAs removed, %d remain.", (scalar keys %{$rref}) - $start, (scalar @{$dref}) - (scalar keys %{$rref}));
    };
    if ($@) {
        $ml->warn("remove bad OTAs failed: $@");
        die $@;
    }
    chdir $oldcwd or die "can't chdir to $oldcwd";
    File::Temp::cleanup();
}
1;
