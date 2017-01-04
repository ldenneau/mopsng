# $Id: Makefile 1507 2007-02-19 18:50:19Z fpierfed $

PROG = fake_diffcor
PROG2 = fake_diffcor.py
MOPS_BIN = ${MOPS_HOME}/bin
INSTALL = /usr/bin/install

$(PROG):

install:
	$(INSTALL) $(PROG) $(MOPS_BIN)
	$(INSTALL) $(PROG2) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
