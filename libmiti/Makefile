# $Id: Makefile 425 2005-06-30 08:36:42Z denneau $

TARGET_LIB = libmiti.a 
TARGET_TEST = testmiti
HEADERS = miti.h

CCOPTS = -g

MOPS_LIB = ${MOPS_HOME}/lib
MOPS_INCLUDE = ${MOPS_HOME}/include

all: $(TARGET_LIB) $(TARGET_TEST)

clean:
	/bin/rm $(TARGET_LIB) $(TARGET_TEST)

$(TARGET_LIB): miti.c miti.h
	gcc $(CCOPTS) -c miti.c -o libmiti.a

$(TARGET_TEST): testmiti.c libmiti.a miti.h
	gcc $(CCOPTS) -L. testmiti.c -o testmiti -lmiti


install:
	/bin/cp $(TARGET_LIB) $(MOPS_LIB)
	/bin/cp $(HEADERS) $(MOPS_INCLUDE)
