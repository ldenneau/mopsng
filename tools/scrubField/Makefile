PROG = scrubField
MOPS_BIN = ${MOPS_HOME}/bin
INSTALL = /usr/bin/install


$(PROG):

install:
	$(INSTALL) $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
