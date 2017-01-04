# $Id: Makefile 421 2005-06-29 03:52:36Z denneau $

PROGS = condor_submit condor_wait
MOPS_BIN = ${MOPS_HOME}/bin

$(PROGS):

install:
	/usr/bin/install $(PROGS) $(MOPS_BIN)

test:
	# Need a python test here.
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
