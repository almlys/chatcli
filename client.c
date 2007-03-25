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

#define PORT 8642 // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

#define AQ { printf("Estoy aquí: %s,%i,%s\n",__FILE__,__LINE__,__FUNCTION__); }

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
			buf[numbytes] = '\0';
			printf("Received: %s",buf);
		} // else Datos recibidos por otro descriptor, o se produjo alguna señal

	}

	close(sockfd);

	return 0;
}

