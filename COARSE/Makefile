# $Id: Makefile 379 2005-06-22 01:00:00Z denneau $

PROGS = coarse coarse_worker coarseEphemOORB
MOPS_BIN = ${MOPS_HOME}/bin

$(PROGS):

install:
	/usr/bin/install $(PROGS) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
