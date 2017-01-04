# $Id: Makefile 715 2005-10-13 09:51:42Z denneau $

PROGS = buildOSS ingestOSS
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROGS) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
