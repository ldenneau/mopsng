use strict;
use warnings;
use Test::More tests => 1;

use PS::MOPS::Constants qw(:all);

#########################

sub all_symbols {
    return
        defined($PI)
        and defined($TWOPI)
        and defined($DEG_PER_RAD)
        and defined($SECONDS_PER_DAY)
        and defined($MINUTES_PER_DAY)
        and defined($MJD2JD_OFFSET)
        and defined($METERS_PER_AU)
        and defined($GM)
        and defined($B62_CHARS)
        and defined($OC_TREF_0HUT_JD)
        and defined($OC_SYNODIC_PERIOD)

        and defined($MOPS_EFF_UNFOUND)
        and defined($MOPS_EFF_CLEAN)
        and defined($MOPS_EFF_MIXED)
        and defined($MOPS_EFF_NONSYNTHETIC)
        and defined($MOPS_EFF_BAD)
        and defined($MOPS_EFF_ORBIT_FAIL)
        and defined($MOPS_EFF_ORBIT_IODFAIL)
        and defined($MOPS_EFF_ORBIT_DIFFAIL)
        and defined($MOPS_EFF_ORBIT_OK)

        and defined($DETECTION_STATUS_UNFOUND)
        and defined($DETECTION_STATUS_FOUND)
        and defined($DETECTION_STATUS_USED)

        and defined($INGEST_STATUS_NEW)
        and defined($INGEST_STATUS_INGESTED)

        and defined($FIELD_STATUS_INGESTED)
        and defined($FIELD_STATUS_READY)
        and defined($FIELD_STATUS_NEW)
        and defined($FIELD_STATUS_TRACKLETS_DONE)
        and defined($FIELD_STATUS_POSTTRACKLET)
        and defined($FIELD_STATUS_ATTRIBUTIONS)
        and defined($FIELD_STATUS_LINK1)
        and defined($FIELD_STATUS_LINKDONE)
        and defined($FIELD_STATUS_READYTOLINK)
        and defined($FIELD_STATUS_SKIP)

        and defined($FIELD_STATUS_SYNTH)
        and defined($FIELD_STATUS_TRACKLET)
        and defined($FIELD_STATUS_ATTRIB)

        and defined($TRACKLET_STATUS_UNATTRIBUTED)
        and defined($TRACKLET_STATUS_ATTRIBUTED)
        and defined($TRACKLET_STATUS_KILLED)
        and defined($TRACKLET_STATUS_KNOWN)

        and defined($DERIVEDOBJECT_STATUS_NEW)
        and defined($DERIVEDOBJECT_STATUS_MERGED)

        and defined($EONQUEUE_STATUS_NEW)
        and defined($EONQUEUE_STATUS_IDENTIFIED)
        and defined($EONQUEUE_STATUS_PRECOVERED)
        and defined($EONQUEUE_STATUS_POSTFIT)
        and defined($EONQUEUE_STATUS_REPORTED)
        and defined($EONQUEUE_STATUS_RETIRED)

        and $MOPS_NONSYNTHETIC_OBJECT_NAME eq 'NS'
        and $MOPS_EFF_ORBIT_UNLINKED == $MOPS_EFF_ORBIT_FAIL
        and defined($MOPS_EFF_ORBIT_REJECTED)
        and defined($MOPS_EFF_ORBIT_RESIDUALS)
}

ok(all_symbols, 'all_symbols');
