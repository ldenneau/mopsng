# $Id: Makefile 1217 2006-08-03 19:06:10Z denneau $

PROG = analyze_timing
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
