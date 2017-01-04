# $Id: Makefile 177 2005-03-10 02:32:08Z denneau $

PROG = ephemSlave
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
