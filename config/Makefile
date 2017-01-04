# $Id: Makefile 4113 2010-09-18 01:45:43Z denneau $

FILES = backend.cf cluster.cf master.cf version orbfit.opt oorb.conf oorb.conf.neo_probability
MOPS_CONFIG = ${MOPS_HOME}/config

$(FILES):

install:
	rsync -avx --delete $(FILES) $(MOPS_CONFIG)

#test:
#	perl -MTest::Harness -e 'runtests(@ARGV)' t/*.t    
