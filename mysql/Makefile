# $Id: Makefile 5713 2015-07-17 02:30:24Z svnuser $

INSTALL = /usr/bin/install
MOPS_SCHEMA = ${MOPS_HOME}/schema


FILES = \
	create_psmops \
    config.sql \
	derivedobject_attrib.sql \
	derivedobjects.sql \
	detections.sql \
	deteff.sql \
	det_rawattr_v2.sql \
	det_rawattr_v3.sql \
	aqueue.sql \
	eon_queue.sql \
	moid_queue.sql \
	fields.sql \
	filter_info.sql \
	history.sql \
	history_attributions.sql \
	history_derivations.sql \
	history_identifications.sql \
	history_precoveries.sql \
	history_removals.sql \
	ingest_status.sql \
	known.sql \
	orbits.sql \
	residuals.sql \
	runtime.sql \
	shapes.sql \
	ssm.sql \
	ssm_desc.sql \
	tracklet_attrib.sql \
	tracklets.sql \
	locks.sql \
	runs.sql \
	alerts.sql \
	v_alerts.sql \
	email_alerts.sql \
	alert_users.sql \
    mpc-import-export.sql \
	manual_detections.sql

install:
	mkdir -p $(MOPS_SCHEMA)
	$(INSTALL) $(FILES) $(MOPS_SCHEMA)

test:
	env PATH=`pwd`:$$PATH perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
