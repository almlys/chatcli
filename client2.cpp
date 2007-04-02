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
#include <sstream>
#include <queue>

#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "protocol.h"
#include "client.h"

//I really hate this, pero no buscamos finezas
#define MAXDATASIZE 4096 // max number of bytes we can get at once
//End hate list

using namespace std;


#ifdef FIXME
/// Send message with udp
int sendudp(char * buf, char * msg, int sockudp, string user) {
	int len;
	len=strlen(buf)-1;
	while (buf[len]=='\n' || buf[len]=='\r') {
		buf[len--]='\0';
	}
	string req=buf;
	string cmd;
	string data;
	string::size_type pos;
	pos=req.find(" ", 0);
	if(pos==std::string::npos) {
		cmd=req;
	} else {
		cmd=req.substr(0,pos);
		data=req.substr(pos+1);
	}
	//cout<<"Processing request, command: "<<cmd<<",data: "<<data<<endl;
	string ip, port;
	
	if(cmd=="500") { //Identificacio
		//cout<<"Intentat enviar missatge privat..."<<endl;
		if (data!="null") {
			pos=data.rfind(" ");
			ip=data.substr(0,pos);
			port=data.substr(pos+1);
			struct sockaddr_in their_addr_udp; /* almacenara la direccion IP y numero de puerto del servidor */
			struct hostent *he_udp; /* para obtener nombre del host */
			int numbytes; /* conteo de bytes a escribir */
			/* convertimos el hostname a su direccion IP */
			if ((he_udp=gethostbyname(ip.c_str())) == NULL) {
				herror("gethostbyname");
				exit(1);
			}
		
			/* a donde mandar */
			std::istringstream i(port);
   			int p;
			i >> p;
			their_addr_udp.sin_family = AF_INET; /* usa host byte order */
			their_addr_udp.sin_port = htons(p); /* usa network byte order */
			their_addr_udp.sin_addr = *((struct in_addr *)he_udp->h_addr);
			bzero(&(their_addr_udp.sin_zero), 8); /* pone en cero el resto */
		
			/* enviamos el mensaje */
			string message=msg;
			string buf=protocol::pm;
			buf+=" "+user+" says: "+message+protocol::sep;
			if ((numbytes=sendto(sockudp,buf.c_str(),strlen(buf.c_str()),0,(struct sockaddr *)&their_addr_udp, sizeof(struct sockaddr))) == -1) {
				perror("sendto");
				exit(1);
			}
			writePrompt();
			//cout<<"Missatge enviat correctament."<<endl;
		} else {
			cout<<"No s'ha pogut enviar el privat. El client no existeix."<<endl;
			writePrompt();
		}
	}

	return 0;
}

/// Process recived data of the server
unsigned char prodataserver(int sock, char * buf) {
	int len;
	len=strlen(buf)-1;
	while (buf[len]=='\n' || buf[len]=='\r') {
		buf[len--]='\0';
	}
	//printf("Processing request: %s<-\n",buf);
	string req=buf, out;
	string str1;
	string str2;
	string::size_type pos;
	pos=req.find(" ", 0);
	if(pos==std::string::npos) {
		str1=req;
	} else {
		str1=req.substr(0,pos);
		str2=req.substr(pos+1);
	}
	//cout<<"Rebut del servidor, command: "<<str1<<", data: "<<str2<<endl;
	if(str1==protocol::ok){
		;
	} else if (str1==protocol::error){
		cout<<"ERROR"<<endl;
		out=protocol::exit;
		out+=protocol::sep;
		sendmsg(sock,out.c_str());
		return 0;
	}else if (str1==protocol::pm){
		cout<<str2<<endl;
	}else if (str1==protocol::bcast){
		cout<<endl<<str2<<endl;
	}else {
		cout<<"ERROR"<<endl;
	}
	writePrompt();
	return 1;
}

int main2(int argc, char *argv[]) {
///
	receivemsg(sockfd,buf);
	if(!strncmp(buf,"200",3)){
		cout<<"ERROR Identificacio"<<endl;
	}

	out=protocol::register2;
	out+=" "+user+" "+port+" "+protocol::sep;
	sendmsg(sockfd, out.c_str());
	receivemsg(sockfd,buf);
	if(!strncmp(buf,"200",3)){
		cout<<"ERROR Registre"<<endl;
	}
	cout<<"Conexion, identificacion y registro, serv: "<<ips<<", usuario: "<<user<<endl;
}
#endif

client::client(const string login,const string server_addr,U16 server_port,const string lhost,U16 lport) {
	_nick = login;
	_server_addr=server_addr;
	_server_port=server_port;
	_bindAddr=lhost;
	_bindPort=lport;
}

void client::startOp() {
	installSignalHandlers();
	struct sockaddr_in my_addr; /* direccion IP y numero de puerto local */
	
	/* se crea el socket */
	if ((_sockudp = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		throw errorException("socket");
	}

	/* Se establece la estructura my_addr para luego llamar a bind() */
	my_addr.sin_family = AF_INET; /* usa host byte order */
	my_addr.sin_port = htons(_bindPort); /* assginem port aleatori */
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* escuchamos en todas las IPs */
	bzero(&(my_addr.sin_zero), 8); /* rellena con ceros el resto de la estructura */

	/* Se le da un nombre al socket (se lo asocia al puerto e IPs) */
	if (bind(_sockudp, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		throw errorException("bind");
	}
	
	//Get port udp
	socklen_t my_addr_size=sizeof(struct sockaddr);
	if(getsockname(_sockudp,(struct sockaddr *)&my_addr,&my_addr_size) == -1) {
		throw("getsockname");
	}
	_udpport=ntohs(my_addr.sin_port);
	cout<<"DBG: Listening to incoming datagrams on port udp "<<_udpport<<endl;
	_select.register2(0); //stdin
	_select.register2(_sockudp);
	writePrompt();
}


void client::connect2() {
	struct hostent *he;
	struct sockaddr_in their_addr; // connector's address information 

	if ((he=gethostbyname(_server_addr.c_str())) == NULL) {  // get the host info 
		throw errorException("gethostbyname");
	}

	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		throw errorException("socket");
	}

	their_addr.sin_family = AF_INET;    // host byte order 
	their_addr.sin_port = htons(_server_port);  // short, network byte order 
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

	//Connectar al servidor
	if (connect(_sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
		throw errorException("connect");
	}

	_select.register2(_sockfd);
	sendHello();
}

void client::stopOp() {
	close(_sockfd);
	close(_sockudp);
}

void client::sendall(const char * msg) {
	int size=strlen(msg);
	int total=0;
	int sn;
	while(total < size) {
		sn=send(_sockfd,msg+total,size-total,0);
		if(sn==-1) throw errorException("send");
		total += sn;
	}
}

void client::sendCmd(const string cmd) {
	string out=cmd;
	out+=protocol::sep;
}

void client::sendHello() {
	_command_stack.push(protocol::ok);
	sendCmd(protocol::helo);
}

void client::requestLoop() {
	queue<int> readfs;
	readfs = _select.wait();

	char buf[MAXDATASIZE+1];
	int numbytes;

	while(!readfs.empty()) {
		int client=readfs.front();
		readfs.pop();
		//cout<<"Event on "<<client<<endl;
		if(client==0) {
			//Datos recibidos por la entrada estandar
			numbytes=read(0,(void *)buf,MAXDATASIZE);
			if(numbytes==0) {
				// The user has sent a EOF (Ctrl+D), and now stdin is closed
				usershutdown();
				continue;
			} else if(numbytes==-1) {
				perror("read");
				exit(-1);
			}
			buf[numbytes]=0;
			string stripped=buf;
			string::size_type pos;
			string stripchars=" \t";
			stripchars+=protocol::sep;
			pos = stripped.find_first_not_of(stripchars);
			stripped.erase(0,pos);
			pos = stripped.find_last_not_of(stripchars);
			stripped.erase(pos+1);
			processUserInput(stripped);
		} else if(client==_sockfd) {
			// Datos recibidos por el socket del servidor
			if ((numbytes=recv(client, buf, MAXDATASIZE, 0)) == -1) {
				throw errorException("recv");
			}
			if(numbytes==0) {
				cout<<"El servidor cerro la conexion"<<endl;
				_keep_running=false;
				if(close(client)!=0) throw errorException("close");
				continue;
			}
			buf[numbytes] = '\0';
			//printf("Received: %s",buf);
			queue<string> reql = _partialData.feed(buf);
			while(!reql.empty()) {
				//cout<<"Processing request: "<<reql.front()<<endl;
				proccessServerResponse(reql.front());
				reql.pop();
			}
		} else if(client==_sockudp) {
			/* Se reciben los datos (directamente, UDP no necesita conexi�) */
			struct sockaddr_in their_addr;
			socklen_t addr_len = sizeof(struct sockaddr_in);
			//printf("Esperando datos ....\n");
			if ((numbytes=recvfrom(_sockudp, buf, MAXDATASIZE, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
				throw errorException("recvfrom");
			}
			/* Se visualiza lo recibido */
			buf[numbytes]=0;
			string::size_type pos;
			string udpmsg=buf;
			string token=protocol::pm;
			token+=" ";
			pos=udpmsg.find(token);
			if(pos!=string::npos) {
				udpmsg=udpmsg.substr(pos+1);
				token=" \t";
				token+=protocol::sep;
				pos = udpmsg.find_first_not_of(token);
				udpmsg.erase(0,pos);
				pos = udpmsg.find_last_not_of(token);
				udpmsg.erase(pos+1);
				printf("\nPrivate from %s:%i: ",inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port));
				cout<<udpmsg<<endl;
				writePrompt();
			}
		}
	}
}

void client::run() {
	startOp();
	connect2();
	while(_keep_running) {
		requestLoop();
	}
	stopOp();
}

void client::usershutdown() {
	cout<<endl;
	cout<<"Cerrando..."<<endl;
	sendCmd(protocol::exit);
	_keep_running=false;
}

void client::help(void) {
        printf("USO DEL CLIENTE:\n\
 <usr>: <msg>  -  Se manda el mensage 'msg' al usuairo 'usr'.\n\
 todos: <msg>  -  Se manda el mensage 'msg' a todos los usuarios conecados.\n\
 ayuda  -  Muestra este menu.\n\
 salir  -  Finalizamos la ejecucion del programa. \n\
\n");
}

void client::processUserInput(const string req) {
//unsigned char processdata(int sock, char * buf,int sockudp, string user)
	//printf("Processing request: %s<-\n",req.c_str());
	string out;
	string str1;
	string str2;
	string::size_type pos;
	pos=req.find(":", 0);
	if(pos==std::string::npos) {
		str1=req;
	} else {
		str1=req.substr(0,pos);
		str2=req.substr(pos+1);
		string strip_chars=" \t";
		pos = str2.find_first_not_of(strip_chars);
		str2.erase(0,pos);
		pos = str2.find_last_not_of(strip_chars);
		str2.erase(pos+1);
	}
	//cout<<"Processing request, command: "<<str1<<", data: "<<str2<<endl;

	if(str2=="") {
		if(str1=="") {
			//Noop
		} else if(str1=="ayuda") {
			help();
		} else if(str1=="salir"){
			usershutdown();
		} else {
			cout<<"Comando desconocido. 'ayuda' pera mostra los comandos posibles."<<endl;
		}
	} else if(_state == kRegister) {
		if(str1=="todos"){
			//cout<<"Enviamos a todos..."<<endl;
			sendBcastMsg(str2);
		} else {
			//cout<<"Enviamos el privado..."<<endl;
			sendp2pMsg(str1,str2);
		}
	} else {
		cout<<"Error, Cannot send a message, because the client is still not registered"<<endl;
	}
	writePrompt();
}

void client::sendBcastMsg(const string msg) {
	_command_stack.push(protocol::ok);
	string out=protocol::bcast;
	out+=" "+msg;
	sendCmd(out);
}

void client::sendp2pMsg(const string nick,const string msg) {
	_command_stack.push(protocol::answer);
	string out=protocol::query;
	out+=" "+nick;
	sendCmd(out);
	_pm_stack.push(msg);
}

void client::writePrompt(){
	cout<<"cliente > ";
	fflush(stdout);
}

//We should use a Singleton signal handler, but this will complicate things, we
// have enough with this piece of crap.
client * obj = NULL;
void s_handler(int s) {
	if(obj!=NULL) obj->signalHandler(s);
}
//end piece of big crap

void client::signalHandler(int s) {
	switch(s) {
		case SIGTERM:
		case SIGINT:
			if(_keep_running==0) {
				cout<<"killed!"<<endl;
				exit(-1);
			}
			cout<<"Clossing client..."<<endl;
			_keep_running=0;
			signal(s,s_handler);
			break;
		default:
			cout<<"Error: Unexpected signal recieved!"<<endl;
	}
}

void client::installSignalHandlers() {
	signal(SIGTERM, s_handler);
	signal(SIGINT, s_handler);
}

/// Show server usage information
/// @param pgname Program name
void usage(char * pgname) {
	printf("Usage: %s [-p 8642] [-b 10] [-D]\n\
 -p <port>: Select the listening udp port (default 0)\n\
\n",pgname);
}

/// Parse command line args
/// @param config Address of the destination Configuration struct
/// @param argc Number of args
/// @param argv Args vector
/// @returns 0 if everything went well, non-zero on error
char parse_argv(struct mconfig * config, int argc, char * argv[]) {
	int i;
	bool host_parsed=false;
	for (i=1; i<argc; i++) {
		if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help")) {
			usage(argv[0]); return -1;
		} else if(!strcmp(argv[i],"-p") && argc>i+1) {
			i++;
			config->port=(U16)atoi(argv[i]);
		} else {
			string cmd=argv[i];
			//usage(argv[0]); return -1;
			if(host_parsed)
				cerr<<"\033[1;31mWarning:\033[0m Ignoring unknown command line param "<<argv[i]<<endl;
			else {
				string::size_type pos;
				pos=cmd.find("@");
				if(pos!=string::npos) {
					config->login = cmd.substr(0,pos);
					cmd = cmd.substr(pos+1);
				}
				pos=cmd.find(":");
				if(pos!=string::npos) {
					config->server_addr = cmd.substr(0,pos);
					config->server_port = atoi(cmd.substr(pos+1).c_str());;
				} else config->server_addr = cmd;
				host_parsed=true;
			}
		}
	}
	return 0;
}

/// Set defaults configuration settings
/// @param config Address of the Configuration struct
void set_config_defaults(struct mconfig * config) {
	//Don't do this ->memset(config,0,sizeof(struct mconfig));
	config->port=0;
	config->login="";
	config->server_addr="";
	config->server_port=8642;
}


/// Main Program
int main(int argc, char * argv[]) {
	struct mconfig config;
	set_config_defaults(&config);
	if(parse_argv(&config,argc,argv)) { return -1; }

	string user, ips;
	int n=0;
	while (config.login=="" && !cin.eof()) {
		cout<<"cliente > "<<"Esperando un identificador de usuario: ";
		//cin>>user; No m'agrada el comportament que té
		getline(cin,config.login);
	}
	while(config.server_addr=="" && !cin.eof()) {
		cout<<"cliente > "<<"Esperando la direccion de un servidor: ";
		//cin>>ips; 3/4 de lo mismo
		getline(cin,config.server_addr);
	}
	if(cin.eof()) return -1; //El usuario cerro la entrada estandar

	try {
		client * cli = new client(config.login,config.server_addr,config.server_port,"",config.port);
		obj=cli;
		cli->run();
		delete cli;
		obj=cli=NULL;
	} catch(exception & e) {
		cout<<"Exception: "<<e.what()<<endl;
	}

	return 0;
}

