CC=gcc
CCFLAGS=-Wall -g -O0
BINS=cserver cclient
BINS2=server client
COMMON_OBJ=common.o
CSERVER_OBJ=cserver.o
CCLIENT_OBJ=cclient.o
SERVER_OBJ=server.o
CLIENT_OBJ=client.o

BUILD=${CC} ${CCFLAGS}

%.o: %.c
	${BUILD} -c $<

all: ${BINS2}

client: ${CLIENT_OBJ}
	${BUILD} -o client ${CLIENT_OBJ}

server: ${SERVER_OBJ}
	${BUILD} -o server ${SERVER_OBJ}

cserver: ${CSERVER_OBJ}
	${BUILD} -o cserver ${CSERVER_OBJ}

cclient: ${CSERVER_OBJ}
	${BUILD} -o cserver ${CSERVER_OBJ}

clean:
	rm -rf *.o ${BINS} ${BINS2}
