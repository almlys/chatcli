CC=gcc
CCFLAGS=-Wall -g -ansi
BINS=cserver
BINS2=server client
CSERVER_OBJ=cserver.o
SERVER_OBJ=server.o
CLIENT_OBJ=client.o

BUILD=${CC} ${CCFLAGS}

all: ${BINS2}

client: ${CLIENT_OBJ}
	${BUILD} -o client ${CLIENT_OBJ}

server: ${SERVER_OBJ}
	${BUILD} -o server ${SERVER_OBJ}

cserver: ${CSERVER_OBJ}
	${BUILD} -o cserver ${CSERVER_OBJ}

clean:
	rm -rf *.o ${BINS} ${BINS2}
