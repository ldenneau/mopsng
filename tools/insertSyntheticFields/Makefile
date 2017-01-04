# $Id: Makefile 800 2006-01-14 01:55:38Z denneau $

PROG = insertSyntheticFields
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
