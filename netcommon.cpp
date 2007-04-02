/******************************************************************************
* $Revision$                                                            *
*                                                                             *
*    Copyright (C) 2007 Ignasi Barri Vilardell                                *
*    Copyright (C) 2007 Alberto Montañola Lacort                              *
*    Copyright (C) 2007 Josep Rius Torrento                                   *
*    See the file AUTHORS for more info                                       *
*                                                                             *
*    This program is free software; you can redistribute it and/or modify     *
*    it under the terms of the GNU General Public License as published by     *
*    the Free Software Foundation; either version 2 of the License, or        *
*    (at your option) any later version.                                      *
*                                                                             *
*    This program is distributed in the hope that it will be useful,          *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
*    GNU General Public License for more details.                             *
*                                                                             *
*    You should have received a copy of the GNU General Public License        *
*    along with this program; if not, write to the Free Software              *
*    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA            *
*    02110-1301,USA.                                                          *
*                                                                             *
*    Please see the file COPYING for the full license.                        *
*                                                                             *
*******************************************************************************
*                                                                             *
* Xarxes II                                                                   *
* Màster en Enginyeria de Programari Lliure                                   *
*                                                                             *
* Pràctica I                                                                  *
*                                                                             *
******************************************************************************/
#include <iostream>
#include <queue>

#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "netcommon.h"

extern bool _debug;

selectInterface::selectInterface() {
	FD_ZERO(&_readfs);
	FD_ZERO(&_master);
	_fdmax=0;
}

void selectInterface::register2(int fdesc) {
	if(_debug) printf("registe: %i\n",fdesc);
	FD_SET(fdesc,&_master);
	_fdmax = fdesc>_fdmax ? fdesc : _fdmax;
}

void selectInterface::unregister(int fdesc) {
	FD_CLR(fdesc,&_master);
	if(_fdmax == fdesc) {
		int i;
		for(i=fdesc-1; i>3; i--) {
			if(FD_ISSET(i,&_master)) {
				_fdmax=i;
				break;
			}
		}
	}
}

std::queue<int> & selectInterface::wait() {
	while(!_descs.empty()) _descs.pop();

	_readfs = _master;
	if(select(_fdmax+1,&_readfs,NULL,NULL,NULL) == -1) {
		if(errno==EINTR) return _descs; //A signal was cauch
		throw errorException("select");
	}

	int i;
	for(i=0; i<=_fdmax; i++) {
		if(FD_ISSET(i,&_readfs)) {
			_descs.push(i);
			//printf("%i", i);
		}
	}
	return _descs;
}

