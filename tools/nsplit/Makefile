# $Id: Makefile 3397 2009-03-12 00:27:55Z denneau $

PROG = nsplit
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/usr/bin/install $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
