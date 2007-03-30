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

#include "server.h"

//I really hate this, pero no buscamos finezas
#define MAXDATASIZE 4096
//End hate list


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

/// Send a message to the client
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


sessionMGR::sessionMGR(server * parent) {
	_parent=parent;
}

sessionMGR::~sessionMGR() {

}

std::map<U32,clientSession *> & sessionMGR::getAllClients() {
	return _clients;
}

void sessionMGR::add(int sock,U32 addr) {
	clientSession * cli = new clientSession(sock,addr);
	if(_clients.find(addr) != _clients.end()) {
		printf("Found! deleting...\n");
		try {
			_clients[addr]->senddif("Sorry, only one connection per ip is accepted!");
		} catch(errorException & e) {
			std::cout<<"Exception sending data..."<<e.what()<<std::endl;
		}
		remove(_clients[addr]);
	}
	_clients[addr]=cli;
	_sockets[sock]=cli;
	_parent->_select.register2(sock);
}

void sessionMGR::remove(clientSession * client) {
	if(close(client->fileno())!=0) throw errorException("close");
	_parent->_select.unregister(client->fileno());
	if(client->getName()!=NULL) {
		std::string out=client->getName();
		out+=" has left the chat";
		_parent->broadcast(out.c_str(),client);
		_nicks.erase(client->getName());
	}
	_clients.erase(client->getIp());
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


server::server(const std::string lhost,const U16 lport,const Byte backlog) {
	_keep_running = true;
	setBindAddress(lhost,lport);
	_backlog = backlog;
	_clients = new sessionMGR(this);
}

server::~server() {
	std::cout<<"destructor"<<std::endl;
	delete _clients;
}

void server::setBindAddress(const std::string lhost,const U16 lport) {
	_bindAddr = lhost;
	_bindPort = lport;
}

void server::startOp() {
	//installSignalHandlers(); TODO
	//Startup
	//Create the socket (TCP)
	if((_socket=socket(PF_INET,SOCK_STREAM,0)) == -1) {
		throw errorException("socket");
	}
	// Set socket options
	// Set Reuse Address
	int reuse=1;
	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
		throw errorException("setsockopt");
	}
	
	// Bind the socket
	struct sockaddr_in server; //Local server address
	memset(&server,0,sizeof(struct sockaddr));
	server.sin_family = PF_INET; //Ipv4
	server.sin_port = htons(_bindPort); //Set the port in Network Byte order
	//TODO convert _bindAddr to an ip address
	server.sin_addr.s_addr = htonl(INADDR_ANY); //Any address in Network Byte order
	if(bind(_socket,(struct sockaddr *)&server,sizeof(struct sockaddr))<0) {
		throw errorException("bind");
	}
	
	// Listen
	if(listen(_socket,_backlog) == -1) {
		throw errorException("listen");
	}

	std::cout<<"DBG: Listening to incoming connections on tcp port "<<_bindPort<<std::endl;
	_select.register2(_socket);
}

void server::stopOp() {
	if(close(_socket)!=0) {
		throw errorException("close");
	}
}

void server::broadcast(const char * msg,const clientSession * client) {
	std::map<U32,clientSession *> clients;
	clients=_clients->getAllClients();
	std::map<U32,clientSession *>::iterator iter;
	for(iter = clients.begin(); iter!=clients.end(); iter++) {
		std::cout<<"Key: "<<iter->first<<std::endl;
		if((client==NULL || iter->second->fileno()!=client->fileno()) && iter->second->isRegistered()) {
			std::string out = protocol::bcast;
			out+= " ";
			out+=msg;
			out+=protocol::sep;
			try {
				iter->second->sendall(out.c_str());
			} catch(errorException & e) {
				std::cout<<"Exception sendind data... "<<e.what()<<std::endl;
			}
		}
	}
}


/// Process a client request
int server::proccessRequest(clientSession * client,char * buf) {
	int len;
	len=strlen(buf)-1;
	while (buf[len]=='\n' || buf[len]=='\r') {
		buf[len--]='\0';
	}
	//printf("Processing request: %s<-\n",buf);
	std::string req=buf;
	std::string cmd;
	std::string data;
	std::string::size_type pos;
	pos=req.find(" ", 0);
	if(pos==std::string::npos) {
		cmd=req;
	} else {
		cmd=req.substr(0,pos);
		data=req.substr(pos+1);
	}
	std::cout<<"Processing request, command: "<<cmd<<",data: "<<data<<std::endl;
	const char * nick=NULL;
	if(data!="") nick=data.c_str();

	if(cmd==protocol::helo && !client->isHallowed()) { //Identificacio
		client->sendOk("OK");
		client->rcvHello();
	} else if ((client->isHallowed() || client->isRegistered()) && nick!=NULL && cmd==protocol::register2) { //Registre
		_clients->register2(client,nick);
		client->sendOk("OK");
	} else if (client->isRegistered() && nick!=NULL && cmd==protocol::query) { //Pregunta
		client->sendanswer(_clients->findAddress(nick));
	} else if (client->isRegistered() && cmd==protocol::bcast) { //Difusio
		std::string msg;
		msg+=client->getName();
		msg+=" says: ";
		msg+=data;
		broadcast(msg.c_str(),client);
		client->sendOk("OK - Enviem a tothom");
	} else if (cmd==protocol::exit) { //Sortir
		//client->sendok("OK - Sortim"); La versió 2.0 no especifica si cal enviar OK o no al rebre la comanda sortir, que fen, ens ho jugen a cara o creu??
		_clients->remove(client);
	} else {
		client->senderror("ERROR");
		_clients->remove(client);
	}

	return 0;
}

void server::requestLoop() {
	std::queue<int> read;
	read = _select.wait();
	while(!read.empty()) {
		int client=read.front();
		read.pop();
		std::cout<<"Event on "<<client<<std::endl;
		if(client==_socket) {
			// accept (A new connection was accepted)
			struct sockaddr_in client;
			socklen_t client_size=sizeof(struct sockaddr_in);
			int csock;
			if ((csock = accept(_socket, (struct sockaddr *)&client, &client_size)) == -1) {
				throw errorException("accept");
				//continue;
			}
			printf("server: got connection from %s , %d\n",inet_ntoa(client.sin_addr),csock);
			_clients->add(csock,ntohl(client.sin_addr.s_addr));
		} else {
			// Data was recieved from a client
			char buf[MAXDATASIZE];
			int num;
			if ((num=recv(client, buf, MAXDATASIZE, 0)) == -1) {
				throw errorException("recv");
			}
			if(num==0) {
				printf("El cliente cerro la conexión %i\n",client);
				_clients->remove(client);
				continue;
			}
			buf[num] = '\0';
			//printf("Received: %s",buf);
			try {
				proccessRequest(_clients->find(client),buf);
			} catch(std::exception &e) {
				std::cout<<"Notice: Exception proccessig a request: "<<e.what()<<std::endl;
				_clients->find(client)->senderror("error");
				_clients->remove(client);
			}
		}
	}
}

void server::run() {
	startOp();
	while(_keep_running) {
		requestLoop();
	}
	stopOp();
}

/// Main Program
int main(int argc, char * argv[]) {

	try {
		server * srv = new server();
		srv->run();
		delete srv;
	} catch(std::exception & e) {
		std::cout<<"Exception: "<<e.what()<<std::endl;
	}

	return 0;
}

