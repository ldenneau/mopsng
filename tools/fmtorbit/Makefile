# $Id: Makefile 267 2005-03-31 22:08:12Z denneau $

PROG = fmtorbit
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	env PATH=`pwd`:$$PATH perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
