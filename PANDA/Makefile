# $Id: Makefile 2140 2007-12-09 11:59:38Z fpierfed $

PROG = panda
WORKER_PROG = panda_worker
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	install $(PROG) $(MOPS_BIN)
	install $(WORKER_PROG) $(MOPS_BIN)

test:
	env PATH=`pwd`:$$PATH perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
