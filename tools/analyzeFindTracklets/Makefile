# $Id: Makefile 167 2005-02-26 02:05:21Z denneau $

PROG = analyzeFindTracklets
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
