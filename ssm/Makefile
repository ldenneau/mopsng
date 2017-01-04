# $Id: Makefile 1058 2006-06-14 23:47:50Z fpierfed $

MOPS_BIN = ${MOPS_HOME}/bin
MOPS_LIB = ${MOPS_HOME}/lib
MOPS_INC = ${MOPS_HOME}/include
MOPS_SSM = ${MOPS_HOME}/data/ssm

INSTALL = /usr/bin/install
CP = /bin/cp

all: install

install:
	$(INSTALL) -d $(MOPS_SSM)
	$(CP) -r orbits $(MOPS_SSM)

  
