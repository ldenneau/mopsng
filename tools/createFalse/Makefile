CC=gcc

#CFLAGS= -O3 -I${MOPS_HOME}/include -I/usr/local/include
CFLAGS= -g -I${MOPS_HOME}/include -I/usr/local/include
LDFLAGS= -L/usr/local/lib64 -lsla -lm -lcfitsio 

createFalse: main.c
	$(CC) $(CFLAGS) -o createFalse main.c $(LDFLAGS) 

install: createFalse
	/usr/bin/install createFalse $(MOPS_HOME)/bin
 
clean:
	/bin/rm -f *.o createFalse

.cpp.o:
	$(CC) $(CFLAGS) $<

