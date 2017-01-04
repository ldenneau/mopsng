# $Id: Makefile 3656 2009-06-05 02:26:33Z denneau $

PROGS = mpcheck mpcheckfield
MOPS_BIN = ${MOPS_HOME}/bin

$(PROGS):

install:
	/usr/bin/install $(PROGS) $(MOPS_BIN)

test:
	env PATH=`pwd`:$$PATH perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t
