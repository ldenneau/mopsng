# $Id: Makefile 399 2005-06-22 21:18:27Z denneau $

PROG = nssh
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	env PATH=`pwd`:$$PATH perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
