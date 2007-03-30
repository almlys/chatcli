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

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server.h"

//I really hate this, pero no buscamos finezas
#define MAXDATASIZE 4096
//End hate list


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

/// Show server usage information
/// @param pgname Program name
void usage(char * pgname) {
	printf("Usage: %s [-p 8642] [-b 10] [-D]\n\
 -p <port>: Select the listening port\n\
 -b <backlog>: Set how many pending connections the queue will hold\n\
 -D: Daemon mode\n\
\n",pgname);
}

/// Parse command line args
/// @param config Address of the destination Configuration struct
/// @param argc Number of args
/// @param argv Args vector
/// @returns 0 if everything went well, non-zero on error
char parse_argv(struct mconfig * config, int argc, char * argv[]) {
	int i;
	for (i=1; i<argc; i++) {
		if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help")) {
			usage(argv[0]); return -1;
		} else if(!strcmp(argv[i],"-p") && argc>i+1) {
			i++;
			config->port=(U16)atoi(argv[i]);
		} else if(!strcmp(argv[i],"-b") && argc>i+1) {
			i++;
			config->backlog=(U32)atoi(argv[i]);
		} else if(!strcmp(argv[i],"-D")) {
			config->daemon=1;
		} else {
			//usage(argv[0]); return -1;
			std::cerr<<"\033[1;31mWarning:\033[0m Ignoring unknown command line param "<<argv[i]<<std::endl;
		}
	}
	return 0;
}

/// Set defaults configuration settings
/// @param config Address of the Configuration struct
void set_config_defaults(struct mconfig * config) {
	memset(config,0,sizeof(struct mconfig));
	config->backlog=10;
	config->port=8642;
}

/// Main Program
int main(int argc, char * argv[]) {
	struct mconfig config;
	set_config_defaults(&config);
	if(parse_argv(&config,argc,argv)) { return -1; }

	//Daemon mode?
	if(config.daemon) {
		daemon(1,0);
	}

	std::cout<<std::endl;
	std::cout<<"\033[0;36m/***************************************************************\\"<<std::endl;
	std::cout<<          "|                IgAlJo C++ MAD server starting...              |"<<std::endl;
	std::cout<<         "\\***************************************************************/\033[0m"<<std::endl;
	std::cout<<std::endl<<"Presh Ctrl+C to stop execution"<<std::endl;

	try {
		server * srv = new server();
		srv->run();
		delete srv;
	} catch(std::exception & e) {
		std::cout<<"Exception: "<<e.what()<<std::endl;
	}

	return 0;
}

