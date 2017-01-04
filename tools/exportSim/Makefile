# $Id: Makefile 160 2005-02-25 09:36:20Z denneau $

PROG = exportSim
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
