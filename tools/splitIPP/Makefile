PROGRAM = splitIPP

CC = gcc
#CFLAGS = -I${MOPS_HOME}/include -I/usr/local/include
CFLAGS = -g -I${MOPS_HOME}/include -I/usr/local/include
LDFLAGS= -L/usr/local/lib64 -lsla -lm -lcfitsio 


all: splitIPP

splitIPP: main.c
	$(CC) $(CFLAGS) -o $(PROGRAM) main.c $(LDFLAGS) 

install: $(PROGRAM$(PROGRAM)
	/usr/bin/install $(PROGRAM) $(MOPS_HOME)/bin
 
clean:
	/bin/rm -f *.o splitIPP

.cpp.o:
	$(CC) $(CFLAGS) $<

