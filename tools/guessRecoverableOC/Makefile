# $Id: Makefile 706 2005-10-11 08:09:28Z denneau $

PROG = guessRecoverableOC
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
