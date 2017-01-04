# $Id: Makefile 657 2005-10-03 21:59:48Z denneau $

PROG = insertSpacewatchFields
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
