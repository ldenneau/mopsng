# $Id: Makefile 170 2005-03-04 05:52:28Z denneau $

PROG = analyzeLinkTracklets
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
