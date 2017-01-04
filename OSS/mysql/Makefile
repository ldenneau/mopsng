# $Id: Makefile 4955 2012-04-13 03:15:26Z dgreen $

INSTALL = /usr/bin/install
MOPS_SCHEMA = ${MOPS_HOME}/schema


FILES = \
	create_oss \
    detection_region.sql \
	tracklet_oss.sql 

install:
	$(INSTALL) $(FILES) $(MOPS_SCHEMA)

test:
	env PATH=`pwd`:$$PATH perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
