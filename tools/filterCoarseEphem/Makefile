# $Id: Makefile 1061 2006-06-14 23:55:56Z fpierfed $

PROG = filterCoarseEphem          # filterCoarseEphem2
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
