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

//In order to understand this file you may need to read cprotocol.py and protocol.h
// Have fun!!

#include <iostream>
#include <string>
#include <exception>

#include <errno.h>

#include "protocol.h"

//Protocol stuff
const char * protocol::sep="\x0D\x0A";
// "\n" *nix systems
// "\r" Old Mac systems
// "\r\n" Hasecorp Hasefroch systems
// Justificació del nostre separador: El protocol HTTP, la propia aplicació de telnet, i altres emprent com a estandar l'enviament de \r\n com a separadors de comandes. Veure http://www.rfc-editor.org/EOLstory.txt, i
// l'empanada mental al fitxer cprotocol.py
const char * protocol::ok="100";
const char * protocol::error="200";
const char * protocol::helo="HOLA";
const char * protocol::register2="300";
const char * protocol::query="400";
const char * protocol::answer="500";
const char * protocol::pm="600";
const char * protocol::bcast="700";
const char * protocol::exit="800";


errorException::errorException(const char * in) {
	_errno=errno;
	_in=in;
}

const char * errorException::what() const throw() {
	std::string errortext;
	char buf[100];
	sprintf(buf,"%s: %i",_in,_errno);
	errortext = buf;
	errortext += " ";
	errortext += strerror(_errno);
	return errortext.c_str();
}

protocolViolation::protocolViolation(const char * in) {
	_in=in;
}

const char *protocolViolation::what() const throw() {
	return _in;
}

