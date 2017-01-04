# $Id: Makefile 872 2006-02-23 01:30:05Z denneau $

PROG = insertNewDerivedObjects
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
