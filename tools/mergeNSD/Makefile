PROGRAM = mergeNSD

CC=gcc
CFLAGS = -O3 -g -I${MOPS_HOME}/include -I/usr/local/include
#CFLAGS = -g -I${MOPS_HOME}/include -I/usr/local/include
LDFLAGS= -L/usr/local/lib64 -lsla -lm -lcfitsio 


all: mergeNSD

$(PROGRAM): mergeNSD.c
	$(CC) $(CFLAGS) -o $(PROGRAM) mergeNSD.c $(LDFLAGS) 

install: $(PROGRAM$(PROGRAM)
	/usr/bin/install $(PROGRAM) $(MOPS_HOME)/bin
 
clean:
	/bin/rm -f *.o $(PROGRAM)

.cpp.o:
	$(CC) $(CFLAGS) $<

