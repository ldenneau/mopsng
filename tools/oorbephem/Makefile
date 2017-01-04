# $Id: Makefile 3504 2009-03-27 22:14:35Z denneau $

PROGS = oorbephem
MOPS_BIN = ${MOPS_HOME}/bin

$(PROGS):

install:
	/usr/bin/install $(PROGS) $(MOPS_BIN)

test:
	env PATH=`pwd`:$$PATH perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
