# $Id: Makefile 162 2005-02-25 20:20:54Z denneau $

PROG = selectTracklets
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
