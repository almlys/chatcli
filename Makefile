#
# $Revision$
#
#    Copyright (C) 2007 Ignasi Barri Vilardell
#    Copyright (C) 2007 Alberto Montañola Lacort
#    Copyright (C) 2007 Josep Rius Torrento
#    See the file AUTHORS for more info
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
#    USA.
#
#    Please see the file COPYING for the full license.
#
CC=gcc
CCFLAGS=-Wall -g -O0
CPP=g++
CPPFLAGS=-Wall -g -O0

BINS=server client
COMMON_OBJ=protocol.o netcommon.o partialDataReader.o
SERVER_OBJ=server.o ${COMMON_OBJ} sessionMGR.o
CLIENT_OBJ=client.o ${COMMON_OBJ}

BUILD=${CC} ${CCFLAGS}
BUILDCPP=${CPP} ${CPPFLAGS}

%.o: %.cpp $.h
	${BUILDCPP} -c $<

#%.o: %.c
#	${BUILD} -c $<

all: ${BINS}

client: ${CLIENT_OBJ}
	${BUILDCPP} -o client ${CLIENT_OBJ}

server: ${SERVER_OBJ}
	${BUILDCPP} -o server ${SERVER_OBJ}

clean:
	rm -rf *~ *.o ${BINS} ${BINS2} ${XBINS}
