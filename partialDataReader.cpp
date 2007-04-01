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
#include <string>
#include <exception>
#include <queue>

#include "protocol.h"
#include "partialDataReader.h"

using namespace std;

bool _debug=true;

PartialDataReader::PartialDataReader(bool emptycmds) {
	_sep_state=0;
	_emptycmds=emptycmds;
}

std::queue<std::string> & PartialDataReader::feed(const char * idata) {

	while(!_outcmds.empty()) _outcmds.pop();

	if (_debug) printf("Partial data %s\n",idata);

	std::string data = _partialData;
	data+=idata;
	_partialData = "";
	bool splitting=true;
	string::size_type pos,pos2;
	int i,n;
	string req;
	char sep_state=0;
	const char * stripchars=" \t";

	while(splitting) {
		pos=data.size();
		n=strlen(protocol::sep);
		for(i=0; i<n; i++) {
			pos2=data.find(protocol::sep[i]);
			pos = pos2!=string::npos && pos2<pos ? pos2 : pos;
		}
		if (pos==data.size()) {
			if (_debug) printf("Notice: Not all data was recieved, waiting for protocol separator\n");
			_partialData = data;
			break;
		}
		//if (_debug) printf("pos is %i\n",pos);
		sep_state=data[pos];
		//if (_debug) printf("Sep_state %02X,%02X\n",_sep_state,sep_state);
		req = data.substr(0,pos);
		//if (_debug) std::cout<<"Request: "<<req<<std::endl;
		if (pos+1 < data.size()) {
			data = data.substr(pos+1);
		} else {
			splitting=false;
		}
		pos2 = req.find_first_not_of(stripchars);
		if (pos2!=string::npos) {
			req.erase(0,pos2);
			pos2 = req.find_last_not_of(stripchars);
			req.erase(pos2+1);
		} else req="";
		//if (_debug) std::cout<<"Request (end): "<<req<<std::endl;
		if (req.size()==0 && sep_state!=_sep_state && _sep_state!=0) continue;
		//if (_debug) printf("Sep_state %02X,%02X\n",_sep_state,sep_state);
		_sep_state = sep_state;
		if (_emptycmds && req.size()==0) continue;
		//if (_debug) std::cout<<"Request (push): "<<req<<std::endl;
		_outcmds.push(req);
	}
	return _outcmds;
}

