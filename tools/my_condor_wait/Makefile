# $Id: Makefile 421 2005-06-29 03:52:36Z denneau $

PROG = my_condor_wait
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	# Need a python test here.
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
