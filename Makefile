CC=gcc
CCFLAGS=-Wall -g -O0
CPP=g++
CPPFLAGS=-Wall -g -O0
BINS=cserver cclient
BINS2=server client
XBINS=xserver
COMMON_OBJ=common.o
CSERVER_OBJ=cserver.o
CCLIENT_OBJ=cclient.o
SERVER_OBJ=server.o
CLIENT_OBJ=client.o
XSERVER_OBJ=xserver.o

BUILD=${CC} ${CCFLAGS}
BUILDCPP=${CPP} ${CPPFLAGS}

%.o: %.cpp $.h
	${BUILDCPP} -c $<

%.o: %.c
	${BUILD} -c $<

all: ${BINS2} ${BINS} ${XBINS}

client: ${CLIENT_OBJ}
	${BUILD} -o client ${CLIENT_OBJ}

server: ${SERVER_OBJ}
	${BUILD} -o server ${SERVER_OBJ}

cserver: ${CSERVER_OBJ}
	${BUILD} -o cserver ${CSERVER_OBJ}

cclient: ${CSERVER_OBJ}
	${BUILD} -o cserver ${CSERVER_OBJ}

xserver: ${XSERVER_OBJ}
	${BUILDCPP} -o xserver ${XSERVER_OBJ}

clean:
	rm -rf *.o ${BINS} ${BINS2} ${XBINS}
