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
#include <map>

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sessionMGR.h"
#include "server.h"

//BEGIN clientSession

clientSession::clientSession(int socket,U32 addr) {
	_socket=socket;
	_addr=addr;
	_status=kNew;
}

int clientSession::fileno() const {
	return _socket;
}

const char * clientSession::getName() const {
	if(strlen(_name.c_str())==0) {
		return NULL;
	}
	return _name.c_str();
}

void clientSession::setName(const char * name) {
	_name=name;
}

const char * clientSession::getAddr() const {
	in_addr cip;
	cip.s_addr=htonl(_addr);
	return inet_ntoa(cip);
}

U32 clientSession::getIp() const {
	return _addr;
}

void clientSession::rcvHello() {
	if(_status!=kNew) throw protocolViolation("Hello was already sent");
	_status=kIdent;
}

void clientSession::rcvRegister() {
	if(_status==kNew) throw protocolViolation("Hello was not sent");
	_status=kRegister;
}


bool clientSession::isHallowed() const {
	return _status==kIdent;
}

bool clientSession::isRegistered() const {
	return _status==kRegister;
}

void clientSession::sendall(const char * msg) {
	int size=strlen(msg);
	int total=0;
	int sn;
	while(total < size) {
		sn=send(_socket,msg+total,size-total,0);
		if(sn==-1) throw errorException("send");
		total += sn;
	}
}

void clientSession::sendOk(const char * msg) {
	std::string buf = protocol::ok;
	//buf += msg;
	buf += protocol::sep;
	sendall(buf.c_str());
}

void clientSession::senderror(const char * msg) {
	std::string buf = protocol::error;
	//buf += msg;
	buf += protocol::sep;
	sendall(buf.c_str());
}

void clientSession::senddif(const char * msg) {
	std::string buf = protocol::bcast;
	buf += " ";
	buf += msg;
	buf += protocol::sep;
	sendall(buf.c_str());
}

void clientSession::sendanswer(const char * msg) {
	std::string buf = protocol::answer;
	buf += " ";
	buf += msg;
	buf += protocol::sep;
	sendall(buf.c_str());
}

//END clientSession

//BEGIN sessionMGR

sessionMGR::sessionMGR(server * parent) {
	_parent=parent;
}

sessionMGR::~sessionMGR() {

}

std::map<int,clientSession *> & sessionMGR::getAllClients() {
	return _sockets;
}

void sessionMGR::add(int sock,U32 addr) {
	clientSession * cli = new clientSession(sock,addr);
	/*if(_clients.find(addr) != _clients.end()) {
		printf("Found! deleting...\n");
		try {
			_clients[addr]->senddif("Sorry, only one connection per ip is accepted!");
		} catch(errorException & e) {
			std::cout<<"Exception sending data..."<<e.what()<<std::endl;
		}
		remove(_clients[addr]);
	}*/
	//_clients[addr]=cli;
	if(_sockets.find(sock) != _sockets.end()) {
		//This should not happen never!!!
		std::cout<<"Something very strange and rare is happending!!!"<<std::endl;
		remove(_sockets[sock]);
	}
	_sockets[sock]=cli;
	_parent->_select.register2(sock);
}

void sessionMGR::remove(clientSession * client,bool closeme) {
	if(closeme && close(client->fileno())!=0) throw errorException("close");
	std::cout<<"Client "<<client->getAddr()<<" disconnected"<<std::endl;
	_parent->_select.unregister(client->fileno());
	if(client->getName()!=NULL) {
		std::string out=client->getName();
		out+=" has left the chat";
		_parent->broadcast(out.c_str(),client);
		_nicks.erase(client->getName());
	}
	//_clients.erase(client->getIp());
	_sockets.erase(client->fileno());
	delete client;
}


void sessionMGR::remove(int sock) {
	remove((clientSession *)find(sock));
}

clientSession * sessionMGR::find(int sock) {
	return _sockets[sock];
}

void sessionMGR::register2(clientSession * client,const char * nick) {
	if(_nicks.find(nick) != _nicks.end()) {
		throw protocolViolation("NickAlreadyExists!");
	}
	std::string msg;
	if(client->getName()!=NULL && _nicks.find(client->getName()) != _nicks.end()) {
		_nicks.erase(client->getName());
		msg=client->getName();
		msg+=" is now know as ";
		msg+=nick;
		_parent->broadcast(msg.c_str(),client);
	} else {
		msg=nick;
		msg+=" has joined the chat";
		_parent->broadcast(msg.c_str(),client);
	}
	client->rcvRegister();
	client->setName(nick);
	_nicks[client->getName()]=client;
}

const char * sessionMGR::findAddress(const char * nick) {
	if(_nicks.find(nick) != _nicks.end()) {
		return _nicks[nick]->getAddr();
	} else {
		return "null";
	}
}

//END sessionMGR
