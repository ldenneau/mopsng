# $Id: Makefile 514 2005-07-26 04:05:13Z denneau $

PROG = eval_lodctl
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
