# $Id: Makefile 1375 2006-11-15 01:34:08Z denneau $

PROG = prep_orbit_test_data
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	/bin/cp $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
