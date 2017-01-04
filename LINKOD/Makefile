# $Id: Makefile 3403 2009-03-12 20:45:13Z denneau $

PROGS = linkod linkod_worker
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/usr/bin/install $(PROGS) $(MOPS_BIN)

test:
	env PATH=`pwd`:$$PATH perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
