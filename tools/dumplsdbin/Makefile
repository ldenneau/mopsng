# $Id$

PROG = dumpbin dumpidx dumpused dumpslice
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	env PATH=`pwd`:$$PATH perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t
