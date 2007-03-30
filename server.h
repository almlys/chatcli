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
#ifndef SERVER_H
#define SERVER_H

#include "protocol.h"
#include "netcommon.h"
#include "sessionMGR.h"

/// The server
class server {
private:
	Byte _keep_running;
	int _socket;
	int _backlog;
	std::string _bindAddr;
	U16 _bindPort;
	sessionMGR * _clients;
public:
	//Porqueries de les que Odia en JMG ;)
	selectInterface _select;
public:
	/// Constructor
	/// @param lhost Bind host (NOT IMPLEMENTED) FIXME, it will just use 0, indiferently of its value
	/// @param lport Bind port (default 8642)
	/// @param backlog Backlog (default 10)
	server(const std::string lhost="",const U16 lport=8642,const Byte backlog=10);
	
	/// Destructor
	~server();

	/// Sets the bind address
	/// @param lhost Bind host
	/// @param lport Bind port
	void setBindAddress(const std::string lhost,const U16 lport);
	
	/// Netcore startup stuff
	void startOp();

	/// Netcore end sequence
	void stopOp();
	
	/// Request loop stuff, Main aplication loop
	void requestLoop();
	
	/// Runs the Main aplication loop
	void run();

	/// Process a single request
	/// @param client Pointer to the associated client session
	/// @param buf Buffer with data to process
	int proccessRequest(clientSession * client,char * buf);

	/// Sends a broadcast message
	/// @param msg The message to send
	/// @param client The source client
	void broadcast(const char * msg,const clientSession * client=NULL);
};

/// Configuration struct
struct mconfig {
	Byte daemon;
	U32 backlog;
	U16 port;
};

#endif
