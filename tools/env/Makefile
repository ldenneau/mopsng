# $Id: Makefile 569 2005-08-15 20:45:17Z denneau $

PROG = mops_env mops_dev mops_exp
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
