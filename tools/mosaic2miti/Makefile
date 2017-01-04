# $Id: Makefile 525 2005-08-02 18:19:41Z denneau $

PROG = mosaic2miti
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
