# $Id$

PROGRAMS = lsd lsdplot
MOPS_BIN = ${MOPS_HOME}/bin

$(PROGRAMS):

install:
	/usr/bin/install $(PROGRAMS) $(MOPS_BIN)

test:
	env PATH=`pwd`:$$PATH perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t
