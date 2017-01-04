use strict;
use warnings;
use Test::More tests => 1;

use PS::MOPS::Constants qw(:all);

#########################

sub status_descs {
    return (
        $DETECTION_STATUS_DESCS{$DETECTION_STATUS_UNFOUND} and
        $DETECTION_STATUS_DESCS{$DETECTION_STATUS_FOUND} and

        $TRACKLET_STATUS_DESCS{$TRACKLET_STATUS_UNATTRIBUTED} and
        $TRACKLET_STATUS_DESCS{$TRACKLET_STATUS_ATTRIBUTED} and
        $TRACKLET_STATUS_DESCS{$TRACKLET_STATUS_KILLED} and
        $TRACKLET_STATUS_DESCS{$TRACKLET_STATUS_KNOWN} and

        $DERIVEDOBJECT_STATUS_DESCS{$DERIVEDOBJECT_STATUS_NEW} and
        $DERIVEDOBJECT_STATUS_DESCS{$DERIVEDOBJECT_STATUS_MERGED} and

        $EONQUEUE_STATUS_DESCS{$EONQUEUE_STATUS_NEW} and
        $EONQUEUE_STATUS_DESCS{$EONQUEUE_STATUS_IDENTIFIED} and
        $EONQUEUE_STATUS_DESCS{$EONQUEUE_STATUS_PRECOVERED} and
        $EONQUEUE_STATUS_DESCS{$EONQUEUE_STATUS_POSTFIT} and
        $EONQUEUE_STATUS_DESCS{$EONQUEUE_STATUS_REPORTED} and
        $EONQUEUE_STATUS_DESCS{$EONQUEUE_STATUS_RETIRED} and

        $EONQUEUE_STATUS_NEXT{$EONQUEUE_STATUS_NEW} and
        $EONQUEUE_STATUS_NEXT{$EONQUEUE_STATUS_IDENTIFIED} and
        !defined($EONQUEUE_STATUS_NEXT{$EONQUEUE_STATUS_RETIRED}) and

        $MOPS_EFF_DESCS{$MOPS_EFF_UNFOUND} and
        $MOPS_EFF_DESCS{$MOPS_EFF_CLEAN} and
        $MOPS_EFF_DESCS{$MOPS_EFF_MIXED} and
        $MOPS_EFF_DESCS{$MOPS_EFF_BAD} and
        $MOPS_EFF_DESCS{$MOPS_EFF_NONSYNTHETIC} and

        $MOPS_EFF_ORBIT_DESCS{$MOPS_EFF_ORBIT_IODFAIL} and
        $MOPS_EFF_ORBIT_DESCS{$MOPS_EFF_ORBIT_DIFFAIL} and
        $MOPS_EFF_ORBIT_DESCS{$MOPS_EFF_ORBIT_FAIL} and
        $MOPS_EFF_ORBIT_DESCS{$MOPS_EFF_ORBIT_OK}
    )
}

ok(status_descs, 'status_descs');
