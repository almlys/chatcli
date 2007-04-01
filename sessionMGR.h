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
#ifndef SESSIONMGR_H
#define SESSIONMGR_H

#include "protocol.h"

/*
 Session Manager, please see reference at sessionMGR.py
*/

/// A client session
/// Holds client related data
class clientSession {
private:
	int _socket;
	U32 _addr;
	std::string _name;
	ClientStatus _status;
public:
	/// Session constructor
	/// @param socket The socket
	/// @param addr Host Byte order ip address
	clientSession(int socket,U32 addr);

	/// Returns the associated file descriptor
	/// @return file descriptor number
	int fileno() const;

	/// Send all implementation, ensures that the entire message is sent
	/// @param msg Message to send
	void sendall(const char * msg);

	/// Sends an error message
	/// @param msg Message to send (depreceated)
	void senderror(const char * msg);
	
	/// Sens an ok message
	/// @param msg Message to send (depreceated)
	void sendOk(const char * msg);
	
	/// Sends an answer message to a query message
	/// @param msg Ip address of the queried host
	void sendanswer(const char * msg);
	
	/// Sends a broadcast message
	/// @param msg Message to send
	void senddif(const char * msg);

	/// Hallows the client (notifies that Hello was sent)
	void rcvHello();

	/// Notifies that the client has been succesfully registered
	void rcvRegister();

	/// Sets this client nickname
	/// @param name
	void setName(const char * name);

	/// Gets this client name
	/// @return pointer to the string that conatins the name
	const char * getName() const;

	/// Ask wheter this client has or not sent the Hello message
	/// @return True if hello was sent
	bool isHallowed() const;
	
	/// Ask wheter this client has or not been registered
	/// @return True if the client is registered
	bool isRegistered() const;

	/// Gets the formated ip address as an pointer to an string
	const char * getAddr() const;

	/// Gets the numerical ip address
	U32 getIp() const;
};

//FOrward
class server;

/// Session manager
class sessionMGR {
private:
	server * _parent;
	//std::map<U32,clientSession *> _clients;
	std::map<std::string,clientSession *> _nicks;
	std::map<int,clientSession *> _sockets;
public:

	/// Constructor
	/// @param parent A server object, parent object with contains some methods required by this class
	sessionMGR(server * parent);

	/// Destructor
	~sessionMGR();

	/// Find a client session by socket
	/// @param sock The socket
	/// @return The pointer to this client session
	clientSession * find(int sock);

	/// Adds a new client to the session MGR
	/// @param sock The client socket
	/// @param addr Númerical ip address associated with this socket
	void add(int sock,U32 addr);

	/// Removes a client by its socket descriptor
	/// @param sock The socket
	void remove(int sock);

	/// Removes a client by its session object
	/// @param client The pointer to the client object
	/// @note This destroys (deletes) the client, working with client
	/// @note after this call, will evolve into a nice Segmentation Fault
	void remove(clientSession * client,bool closeme=true);

	/// Associates a nick to a client
	/// @param client The pointer to the client object
	/// @param name Name of the client
	void register2(clientSession * client,const char * name);

	/// Returns the address associated to a nickname
	/// @param name The nickname
	/// @return Formated ip address, prepared to be sent back to the client
	const char * findAddress(const char * name);

	/// Gets a list of all connected clients to this system
	/// @return a Map with all connected clients
	std::map<int,clientSession *> & getAllClients();
};

#endif
