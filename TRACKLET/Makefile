# $Id: Makefile 350 2005-05-24 21:42:26Z denneau $

PROGS = tracklet tracklet_worker makeTracklet
MOPS_BIN = ${MOPS_HOME}/bin

$(PROG):

install:
	install $(PROGS) $(MOPS_BIN)

test:
	env PATH=`pwd`:$$PATH perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
