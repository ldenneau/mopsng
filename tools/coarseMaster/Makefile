# $Id: Makefile 379 2005-06-22 01:00:00Z denneau $

PROG = coarseMaster
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
