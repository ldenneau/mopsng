# $Id: Makefile 872 2006-02-23 01:30:05Z denneau $

PROG = insertResids orb2resids postfit analyze_tracklet_resids
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	install $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
