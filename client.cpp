/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <string>
#include <iostream>

using namespace std;

#define PORT 8642 // the port client will be connecting to 
#define UDP_PORT 7766 // udp p2p port

#define MAXDATASIZE 100 // max number of bytes we can get at once 
#define BUFFER_LEN 1024

#define AQ { printf("Estoy aquí: %s,%i,%s\n",__FILE__,__LINE__,__FUNCTION__); }


/// Process recived data
int processdata(char * buf) {
	int len;
	len=strlen(buf)-1;
	while (buf[len]=='\n' || buf[len]=='\r') {
		buf[len--]='\0';
	}
	//printf("Processing request: %s<-\n",buf);
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
			int sockfdudp; /* descriptor a usar con el socket */
			struct sockaddr_in their_addr_udp; /* almacenara la direccion IP y numero de puerto del servidor */
			struct hostent *he_udp; /* para obtener nombre del host */
			int numbytes; /* conteo de bytes a escribir */
			/* convertimos el hostname a su direccion IP */
			if ((he_udp=gethostbyname(ip)) == NULL) {
				herror("gethostbyname");
				exit(1);
			}
		
			/* Creamos el socket */
			if ((sockfdudp = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
				perror("socket");
				exit(1);
			}
		
			/* a donde mandar */
			their_addr_udp.sin_family = AF_INET; /* usa host byte order */
			their_addr_udp.sin_port = htons(UDP_PORT); /* usa network byte order */
			their_addr_udp.sin_addr = *((struct in_addr *)he_udp->h_addr);
			bzero(&(their_addr_udp.sin_zero), 8); /* pone en cero el resto */
		
			/* enviamos el mensaje */
			if ((numbytes=sendto(sockfdudp,ip,strlen(ip),0,(struct sockaddr *)&their_addr_udp, sizeof(struct sockaddr))) == -1) {
				perror("sendto");
				exit(1);
			}
			cout<<"Missatge enviat correctament."<<endl;
		} else {
			cout<<"No s'ha pogut enviar el privat. El client no existeix."<<endl;
		}
	} /*else if ((client->isHallowed() || client->isRegistered()) && nick!=NULL && cmd==protocol::register2) { //Registre
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
	}*/

	return 0;
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
	printf("Creando socket ....\n");
	if (bind(sockudp, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	return sockudp;
}

int main(int argc, char *argv[]) {
	int fdmax;
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct hostent *he;
	struct sockaddr_in their_addr; // connector's address information 

	unsigned char keep_running=1;
	int n,sn;
	fd_set readfs;
	fd_set master;


	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(-1);
	}	

	if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info 
	    perror("gethostbyname");
	    exit(-1);
	}

	struct sockaddr_in their_addr_udp; /* direccion IP y numero de puerto del cliente */
	/* addr_len contendra el tamanio de la estructura sockadd_in y numbytes el
	 * numero de bytes recibidos
	 */
	int sockudp;
	char buffer[BUFFER_LEN]; /* Buffer de recepci� */
	sockudp = startupc();


	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	    perror("socket");
	    exit(-1);
	}

	their_addr.sin_family = AF_INET;    // host byte order 
	their_addr.sin_port = htons(PORT);  // short, network byte order 
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

	//Connectar al servidor
	if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
	    perror("connect");
	    exit(-1);
	}

//Objetivo:
//	1- Utilizar el select para detectar cunado se han introducido datos por teclado
//	2- Mostra los datos introducidos
//	HECHO

	FD_ZERO(&readfs); //Inicializar
	FD_ZERO(&master);

	FD_SET(0,&master); //Fijar la entrada estandar
	FD_SET(sockfd,&master); //Fijar el socket
	FD_SET(sockudp,&master);

	fdmax = sockfd;

	while(keep_running) {
		readfs=master;
		if(select(fdmax+1,&readfs,NULL,NULL,NULL)==-1) {
			perror("select");
			exit(-1);
		}

		if(FD_ISSET(0,&readfs)) {
			//Datos recibidos por la entrada estandar
			n=read(0,(void *)buf,MAXDATASIZE-1);
			if(n==0) {
				fprintf(stderr,"Error, se leyeron 0 datos?\n");
				exit(-1);
			} else if(n==-1) {
				perror("read");
				exit(-1);
			}
			buf[n]=0;
			sn=send(sockfd,(const void *)buf,n,0);
			if(sn==-1) {
				perror("send\n");
				exit(-1);
			} else if(sn!=n) {
				fprintf(stderr,"Error, no se enviaron todos los datos?\n");
				exit(-1);
			}
			//printf("Dades: %s\n",buf);
			if(!strncmp(buf,"800\n",n)) {
				keep_running=0;
			}
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
			processdata(buf);
			buf[numbytes] = '\0';
			//printf("Received: %s",buf);
			
		}  else if(FD_ISSET(sockudp,&readfs)) {
			/* Se reciben los datos (directamente, UDP no necesita conexi�) */
			socklen_t addr_len = sizeof(struct sockaddr);
			printf("Esperando datos ....\n");
			if ((numbytes=recvfrom(sockudp, buffer, BUFFER_LEN, 0, (struct sockaddr *)&their_addr_udp, &addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
			}
		
			/* Se visualiza lo recibido */
			//printf("paquete proveniente de : %s\n",inet_ntoa(their_addr_udp.sin_addr));
			printf("longitud del paquete en bytes : %d\n",numbytes);
			buffer[numbytes] = '\0';
			printf("el paquete contiene : %s\n", buffer);
		} else {
			cout<<"Datos recibidos por otro descriptor, o se produjo alguna señal."<<endl;
		}

	}

	close(sockfd);

	return 0;
}

