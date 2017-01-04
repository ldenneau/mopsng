# $Id: Makefile 1855 2007-08-29 04:00:49Z denneau $

PROGS = register_fileset \
	create_detectability_fileset \
	fakedq

MOPS_BIN = ${MOPS_HOME}/bin

$(PROGS):

install:
	/usr/bin/install $(PROGS) $(MOPS_BIN)

test:
	env PATH=`pwd`:$$PATH perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
