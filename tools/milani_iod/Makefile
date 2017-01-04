
PROG = milani_iod milani_iod.twopass milani_iod.pass1 milani_iod.pass2 mopsiod
MOPS_BIN = ${MOPS_HOME}/bin
INSTALL = /usr/bin/install


$(PROG):

install:
	$(INSTALL) $(PROG) $(MOPS_BIN)

test:
	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
