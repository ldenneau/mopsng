# $Id: Makefile 282 2005-04-02 07:37:58Z denneau $

PROG = ephemMaster ephemMaster2
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
