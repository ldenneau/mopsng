# $Id: Makefile 421 2005-06-29 03:52:36Z denneau $

PROG = dateinfo
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/usr/bin/install $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
