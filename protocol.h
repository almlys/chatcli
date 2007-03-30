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
#ifndef PROTOCOL_H
#define PROTOCOL_H

/*
 Defines all the pràctica I chat protocol v2.0
 Please see cprotocol.py as a reference for reading this file
*/

//Some commonly used Data types
typedef unsigned char Byte;
typedef unsigned short int U16;
typedef unsigned int U32;

/// Chat Protocol v2.0
namespace protocol {
	extern const char * sep;
	extern const char * ok;
	extern const char * error;
	extern const char * helo;
	extern const char * register2;
	extern const char * query;
	extern const char * answer;
	extern const char * pm;
	extern const char * bcast;
	extern const char * exit;
};

///Define client states
enum ClientStatus { kNew=0, kIdent=1, kRegister=2 };


/// Encapsulate errors into this exception
class errorException: public std::exception {
private:
	int _errno;
	const char * _in;
public:
	/// Constructor
	/// @param in Some text (eg, function name)
	errorException(const char * in);
	/// Returns a pointer to a char buffer with some info about what the hell is going on
	/// @return Error descritive text
	virtual const char * what() const throw();
};

/// Defines a protocol Violation
class protocolViolation: public std::exception {
private:
	const char * _in;
public:
	/// Constructor
	/// @param in Name/Type of the violation
	protocolViolation(const char * in);
	/// Returns a pointer to a char buffer with some info about what the hell is going on
	/// @return Error descritive text
	virtual const char * what() const throw();
};

#endif
