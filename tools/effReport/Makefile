# $Id: Makefile 1393 2006-12-01 02:14:13Z denneau $

PROG = effReport
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
