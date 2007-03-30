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

class clientSession {
private:
	int _socket;
	U32 _addr;
	std::string _name;
	ClientStatus _status;
public:
	clientSession(int socket,U32 addr);
	int fileno() const;
	void sendall(const char * msg);
	void senderror(const char * msg);
	void sendOk(const char * msg);
	void sendanswer(const char * msg);
	void senddif(const char * msg);
	void rcvHello();
	void rcvRegister();
	void setName(const char * name);
	const char * getName() const;
	bool isHallowed() const;
	bool isRegistered() const;
	const char * getAddr() const;
	U32 getIp() const;
};


class server;

class sessionMGR {
private:
	server * _parent;
	std::map<U32,clientSession *> _clients;
	std::map<std::string,clientSession *> _nicks;
	std::map<int,clientSession *> _sockets;
public:
	sessionMGR(server * parent);
	~sessionMGR();
	clientSession * find(int sock);
	void add(int sock,U32 addr);
	void remove(int sock);
	void remove(clientSession * client);
	void register2(clientSession * client,const char * name);
	const char * findAddress(const char * name);
	std::map<U32,clientSession *> & getAllClients();
};

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
	server(const std::string lhost="",const U16 lport=8642,const Byte backlog=10);
	~server();
	void setBindAddress(const std::string lhost,const U16 lport);
	void startOp();
	void stopOp();
	void requestLoop();
	void run();
	int proccessRequest(clientSession *,char * buf);
	/*int sendall(const int csock, const char * msg);
	int sendok(const int csock, const char * msg);
	int senderror(const int csock, const char * msg);*/
	void broadcast(const char * msg,const clientSession * client=NULL);
};

#endif
