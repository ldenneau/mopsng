# $Id: Makefile 298 2005-04-06 01:35:46Z denneau $

PROG = mitiPlot
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
