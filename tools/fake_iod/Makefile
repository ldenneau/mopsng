# $Id: Makefile 1335 2006-09-19 19:11:39Z fpierfed $

PROG = fake_iod
PROG2 = fake_iod.py
MOPS_BIN = ${MOPS_HOME}/bin
INSTALL = /usr/bin/install

$(PROG):

install:
	$(INSTALL) $(PROG) $(MOPS_BIN)
	$(INSTALL) $(PROG2) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
