# $Id: Makefile 298 2005-04-06 01:35:46Z denneau $

PROG = compare_known_dets compare_known_tracklets eff_known eff_suite insert_eff
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/usr/bin/install $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
