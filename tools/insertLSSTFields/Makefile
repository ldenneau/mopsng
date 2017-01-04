# $Id: Makefile 1076 2006-06-23 03:18:19Z fpierfed $

INSTALL = /usr/bin/install


PROG = insertLSSTFields
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	$(INSTALL) $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
