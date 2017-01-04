# $Id: Makefile 173 2005-03-08 20:43:55Z denneau $

PROG = guessTracklets
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
