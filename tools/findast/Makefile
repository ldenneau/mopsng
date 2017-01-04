# $Id: Makefile 262 2005-03-31 09:00:21Z denneau $

PROG = findast
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/usr/bin/install $(PROG) $(MOPS_BIN)

test:
	env PATH=`pwd`:$$PATH perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
