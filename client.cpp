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

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "protocol.h"

using namespace std;

#define PORT 8642 // the port client will be connecting to 
#define UDP_PORT 7766 // udp p2p port

#define MAXDATASIZE 1025 // max number of bytes we can get at once 

/// Show client usage information
void help(void) {
        printf("USO DEL CLIENTE:\n\
 usr: msg  -  Se manda el mensage 'msg' al usuairo 'usr'.\n\
 all: msg  -  Se manda el mensage 'msg' a todos los usuarios conecados.\n\
 ayuda  -  Muestra este menu.\n\
 salir  -  Finalizamos la ejecucion del programa. \n\
\n");
}

/// Send message with udp
int sendudp(char * buf, char * msg,int sockfdudp) {
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
	cout<<"Processing request, command: "<<cmd<<",data: "<<data<<endl;
	const char * ip=NULL;
	if(data!="") ip=data.c_str();
	
	if(cmd=="500") { //Identificacio
		cout<<"Intentat enviar missatge privat..."<<endl;
		if (data!="null") {
			struct sockaddr_in their_addr_udp; /* almacenara la direccion IP y numero de puerto del servidor */
			struct hostent *he_udp; /* para obtener nombre del host */
			int numbytes; /* conteo de bytes a escribir */
			/* convertimos el hostname a su direccion IP */
			if ((he_udp=gethostbyname(ip)) == NULL) {
				herror("gethostbyname");
				exit(1);
			}
		
			/* Creamos el socket */
		/*	if ((sockfdudp = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
				perror("socket");
				exit(1);
			}*/
		
			/* a donde mandar */
			their_addr_udp.sin_family = AF_INET; /* usa host byte order */
			their_addr_udp.sin_port = htons(UDP_PORT); /* usa network byte order */
			their_addr_udp.sin_addr = *((struct in_addr *)he_udp->h_addr);
			bzero(&(their_addr_udp.sin_zero), 8); /* pone en cero el resto */
		
			/* enviamos el mensaje */
			if ((numbytes=sendto(sockfdudp,msg,strlen(msg),0,(struct sockaddr *)&their_addr_udp, sizeof(struct sockaddr))) == -1) {
				perror("sendto");
				exit(1);
			}
			cout<<"Missatge enviat correctament."<<endl;
		} else {
			cout<<"No s'ha pogut enviar el privat. El client no existeix."<<endl;
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
char * processmsg(int sock, char * buf){
	int sn;
	memset(buf,0,sizeof(buf));
	if ((sn=recv(sock, buf, MAXDATASIZE-1, 0)) == -1) {
		perror("recv");
		exit(-1);
	}
	if(sn==0) {
		printf("El servidor cerró la conexión\n");
	}
	buf[sn] = '\0';
	return buf;
}

/// Process recived data of the user
unsigned char processdata(int sock, char * buf,int sockudp) {
	int len;
	len=strlen(buf)-1;
	while (buf[len]=='\n' || buf[len]=='\r') {
		buf[len--]='\0';
	}
	//printf("Processing request: %s<-\n",buf);
	string req=buf;
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
	cout<<"Processing request, command: "<<str1<<",data: "<<str2<<endl;

	if(str1=="ayuda") { //Identificacio
		help();
	} else if(str1=="salir"){
		cout<<"Cerrando..."<<endl;
		return 0;
	} else if(str1=="todos"){
		cout<<"Enviamos a todos..."<<endl;
		sprintf(buf,"700 %s", str2.c_str());
		sendmsg(sock,buf);
	} else { //DUBTE
		cout<<"Enviamos el privado..."<<endl;
		memset(buf,0,sizeof(buf));
		sprintf(buf,"400 %s", str1.c_str());
		sendmsg(sock, buf);
		buf=processmsg(sock, buf);
		cout<<buf<<endl;
		sendudp(buf, (char *) str2.c_str(),sockudp);
	}
	return 1;
}


int startupc(){
	int sockudp; /* descriptor para el socket */
	struct sockaddr_in my_addr; /* direccion IP y numero de puerto local */
	
	/* se crea el socket */
	if ((sockudp = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	/* Se establece la estructura my_addr para luego llamar a bind() */
	my_addr.sin_family = AF_INET; /* usa host byte order */
	my_addr.sin_port = htons(UDP_PORT); /* usa network byte order */
	my_addr.sin_addr.s_addr = INADDR_ANY; /* escuchamos en todas las IPs */
	bzero(&(my_addr.sin_zero), 8); /* rellena con ceros el resto de la estructura */

	/* Se le da un nombre al socket (se lo asocia al puerto e IPs) */
	if (bind(sockudp, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	return sockudp;
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
	printf("Esperando un identificador de usuario: ");
	cin>>user;
	printf("Esperando la direccion de un servidor: ");
	cin>>ips;
	cout<<"Conexion, identificacion y registro, serv: "<<ips<<", usuario: "<<user<<endl;
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

	sockudp = startupc();

	their_addr.sin_family = AF_INET;    // host byte order 
	their_addr.sin_port = htons(PORT);  // short, network byte order 
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

	//Connectar al servidor
	if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
	    perror("connect");
	    exit(-1);
	}

	cout<<"Identifikem"<<endl;
	string out = protocol::helo;
	out+=protocol::sep;
/*	memset(buf,0,sizeof(buf));
	sprintf(buf,"HOLA");*/
	sendmsg(sockfd, out.c_str());
	strcpy(buf,processmsg(sockfd,buf));
	if(!strncmp(buf,"100",3)){
		printf("Identificats correctament\n");
	}

	cout<<"Registrem"<<endl;
	memset(buf,0,sizeof(buf));
	sprintf(buf,"300 %s", user.c_str());
	sendmsg(sockfd, buf);
	strcpy(buf,processmsg(sockfd,buf));
	if(!strncmp(buf,"100",3)){
		cout<<"Registrats correctament"<<endl;
	}
	
	FD_ZERO(&readfs); //Inicializar
	FD_ZERO(&master);

	FD_SET(0,&master); //Fijar la entrada estandar
	FD_SET(sockfd,&master); //Fijar el socket
	FD_SET(sockudp,&master);

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
			keep_running = processdata(sockfd, buf,sockudp);
			memset(buf,0,sizeof(buf));
		} else if(FD_ISSET(sockfd,&readfs)) {
			// Datos recibidos por el socket del servidor
			if ((numbytes=recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
				exit(-1);
			}
			if(numbytes==0) {
				printf("El servidor cerró la conexión\n");
				keep_running=0;
				continue;
			}
			buf[numbytes] = '\0';
		
		}  else if(FD_ISSET(sockudp,&readfs)) {
			/* Se reciben los datos (directamente, UDP no necesita conexi�) */
			socklen_t addr_len = sizeof(struct sockaddr);
			printf("Esperando datos ....\n");
			if ((numbytes=recvfrom(sockudp, buf, MAXDATASIZE-1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
			}
			/* Se visualiza lo recibido */
			buf[numbytes] = '\0';
			printf("%s: %s\n",inet_ntoa(their_addr.sin_addr), buf);
		} else {
			cout<<"Datos recibidos por otro descriptor, o se produjo alguna señal."<<endl;
		}
	}

	close(sockfd);
	close(sockudp);

	return 0;
}

