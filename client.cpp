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

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "protocol.h"

using namespace std;

#define PORT 8642 // the port client will be connecting to 

#define MAXDATASIZE 1025 // max number of bytes we can get at once 

/// Show client prompt
void writePrompt(){
	cout<<"cliente > ";
	fflush(stdout);
}

/// Show client usage information
void help(void) {
        printf("USO DEL CLIENTE:\n\
 <usr>: <msg>  -  Se manda el mensage 'msg' al usuairo 'usr'.\n\
 todos: <msg>  -  Se manda el mensage 'msg' a todos los usuarios conecados.\n\
 ayuda  -  Muestra este menu.\n\
 salir  -  Finalizamos la ejecucion del programa. \n\
\n");
	writePrompt();
}

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

/// Send a message to the server with tpc connection
int sendmsg(int sock, const char * buf){
	int n, sn;
	n=strlen(buf);
	if((sn=send(sock,(const void *)buf,n,0))==-1) {
		perror("send\n");
		exit(-1);
	} else if(sn!=n) {
		fprintf(stderr,"Error, no se enviaron todos los datos?\n");
		exit(-1);
	}
	return 0;
}
/// Process recived message
int processmsg(int sock, char * buf){
	int sn;
	memset(buf,0,sizeof(buf));
	if ((sn=recv(sock, buf, MAXDATASIZE-1, 0)) == -1) {
		perror("recv");
		exit(-1);
	}
	if(sn==0) {
		cout<<"El servidor cerro la conexion"<<endl;
	}
	buf[sn] = '\0';
	return 0;
}

/// Process recived data of the user
unsigned char processdata(int sock, char * buf,int sockudp, string user) {
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
	pos=req.find(": ", 0);
	if(pos==std::string::npos) {
		str1=req;
	} else {
		str1=req.substr(0,pos);
		str2=req.substr(pos+1);
	}
	//cout<<"Processing request, command: "<<str1<<", data: "<<str2<<endl;

	if(str2==""){
		if(str1=="ayuda") { //Identificacio
			help();
		} else if(str1=="salir"){
			cout<<"Cerrando..."<<endl;
			out=protocol::exit;
			out+=protocol::sep;
			sendmsg(sock,out.c_str());
			return 0;
		} else {
			cout<<"Comando desconocido. 'ayuda' pera mostra los comandos posibles."<<endl;
			writePrompt();
		}
	} else if(str1=="todos"){
		//cout<<"Enviamos a todos..."<<endl;
			out=protocol::bcast;
			out+=" "+str2+protocol::sep;
			sendmsg(sock,out.c_str());
	} else {
		//cout<<"Enviamos el privado..."<<endl;
		out=protocol::query;
		out+=" "+str1+protocol::sep;
		sendmsg(sock, out.c_str());
		processmsg(sock, buf);
		sendudp(buf, (char *) str2.c_str(),sockudp, user);
	}

	return 1;
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
		cout<<"OK"<<endl;
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

int * startudp(){
	int sockudp;
	struct sockaddr_in my_addr; /* direccion IP y numero de puerto local */
	
	/* se crea el socket */
	if ((sockudp = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	/* Se establece la estructura my_addr para luego llamar a bind() */
	my_addr.sin_family = AF_INET; /* usa host byte order */
	my_addr.sin_port = htons(0); /* assginem port aleatori */
	my_addr.sin_addr.s_addr = INADDR_ANY; /* escuchamos en todas las IPs */
	bzero(&(my_addr.sin_zero), 8); /* rellena con ceros el resto de la estructura */

	/* Se le da un nombre al socket (se lo asocia al puerto e IPs) */
	if (bind(sockudp, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	
	//Get port udp
	socklen_t my_addr_size=sizeof(struct sockaddr);
	if(getsockname(sockudp,(struct sockaddr *)&my_addr,&my_addr_size) == -1) {
		perror("getsockname");
	}
	static int aux[2];
	aux[0]=sockudp;
	aux[1]=ntohs(my_addr.sin_port);
	return aux;
}

int main(int argc, char *argv[]) {
	int sockfd, sockudp, fdmax, numbytes;
	char buf[MAXDATASIZE];
	struct hostent *he;
	struct sockaddr_in their_addr; // connector's address information 

	unsigned char keep_running=1;
	fd_set readfs;
	fd_set master;

	string user, ips;
	cout<<"cliente > "<<"Esperando un identificador de usuario: ";
	cin>>user;
	cout<<"cliente > "<<"Esperando la direccion de un servidor: ";
	cin>>ips;
	const char * ip=NULL;
	if(ips!="") ip=ips.c_str();

	if ((he=gethostbyname(ip)) == NULL) {  // get the host info 
	    perror("gethostbyname");
	    exit(-1);
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	    perror("socket");
	    exit(-1);
	}

	int * aux;
	aux = startudp();
	sockudp=aux[0];
	std::ostringstream o;
	o << aux[1];
	string port = o.str();

	their_addr.sin_family = AF_INET;    // host byte order 
	their_addr.sin_port = htons(PORT);  // short, network byte order 
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

	//Connectar al servidor
	if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
	    perror("connect");
	    exit(-1);
	}

	string out=protocol::helo;
	out+=protocol::sep;
	sendmsg(sockfd, out.c_str());
	processmsg(sockfd,buf);
	if(!strncmp(buf,"200",3)){
		cout<<"ERROR Identificacio"<<endl;
	}

	out=protocol::register2;
	out+=" "+user+" "+port+" "+protocol::sep;
	sendmsg(sockfd, out.c_str());
	processmsg(sockfd,buf);
	if(!strncmp(buf,"200",3)){
		cout<<"ERROR Registre"<<endl;
	}
	cout<<"Conexion, identificacion y registro, serv: "<<ips<<", usuario: "<<user<<endl;

	FD_ZERO(&readfs); //Inicializar
	FD_ZERO(&master);

	FD_SET(0,&master); //Fijar la entrada estandar
	FD_SET(sockfd,&master); //Fijar el socket
	FD_SET(sockudp,&master);

	writePrompt();

	fdmax = sockudp;
	while(keep_running) {
		readfs=master;
		if(select(fdmax+1,&readfs,NULL,NULL,NULL)==-1) {
			perror("select");
			exit(-1);
		}
		if(FD_ISSET(0,&readfs)) {
			//Datos recibidos por la entrada estandar
			numbytes=read(0,(void *)buf,MAXDATASIZE-1);
			if(numbytes==0) {
				fprintf(stderr,"Error, se leyeron 0 datos?\n");
				exit(-1);
			} else if(numbytes==-1) {
				perror("read");
				exit(-1);
			}
			buf[numbytes]=0;
			keep_running = processdata(sockfd, buf, sockudp, user);
			memset(buf,0,sizeof(buf));
		} else if(FD_ISSET(sockfd,&readfs)) {
			// Datos recibidos por el socket del servidor
			if ((numbytes=recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
				exit(-1);
			}
			if(numbytes==0) {
				cout<<"El servidor cerro la conexion"<<endl;
				keep_running=0;
				continue;
			}
			buf[numbytes]=0;
			keep_running = prodataserver(sockfd,buf);
			memset(buf,0,sizeof(buf));
		}  else if(FD_ISSET(sockudp,&readfs)) {
			/* Se reciben los datos (directamente, UDP no necesita conexi�) */
			socklen_t addr_len = sizeof(struct sockaddr);
			//printf("Esperando datos ....\n");
			if ((numbytes=recvfrom(sockudp, buf, MAXDATASIZE-1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
			}
			/* Se visualiza lo recibido */
			buf[numbytes]=0;
			printf("\nFrom %s:%i: ",inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port));
			keep_running = prodataserver(sockfd,buf);
			memset(buf,0,sizeof(buf));
		} else {
			cout<<"Datos recibidos por otro descriptor, o se produjo alguna señal."<<endl;
		}
	}

	close(sockfd);
	close(sockudp);

	return 0;
}

