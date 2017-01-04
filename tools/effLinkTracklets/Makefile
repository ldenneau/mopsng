# $Id: Makefile 1063 2006-06-15 02:55:02Z fpierfed $

PROG = effLinkTracklets
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
