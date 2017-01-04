# $Id: Makefile 973 2006-04-12 02:25:09Z denneau $

PROG = effFindAvailableTracks
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
