# $Id: Makefile 4675 2011-09-29 18:46:45Z denneau $

PROG = dx mobsfilt dxlt2iodreq mpc2des
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	env PATH=`pwd`:$$PATH perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
