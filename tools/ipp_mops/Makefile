# $Id: Makefile 298 2005-04-06 01:35:46Z denneau $

PROG = ipp_mops_query ipp_mops_query_lap mirror_ipp_mops mirror_ipp_mops_lap do-mirror-lap chunk_status.sh ipp_flags.py ps1_observations_on_date.py

MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/usr/bin/install $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
