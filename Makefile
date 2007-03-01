CC=gcc
CCFLAGS=-Wall -g -ansi
BINS=cserver
CSERVER_OBJ=cserver.o

BUILD=${CC} ${CCFLAGS}

cserver: ${CSERVER_OBJ}
	${BUILD} -o cserver ${CSERVER_OBJ}

clean:
	rm -rf *.o ${BINS}
