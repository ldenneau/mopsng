# $Id: Makefile 169 2005-03-04 03:47:17Z denneau $

PROG = linkTrackletsMP
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
