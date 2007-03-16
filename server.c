/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define MYPORT 8642 // the port users will be connecting to

#define BACKLOG 10 // how many pending connections queue will hold

#define MAXDATASIZE 100 // max number of bytes we can get at once 


int main(void) {
	int numbytes;
	int fdmax;
	char buf[MAXDATASIZE];

	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct sockaddr_in my_addr;	// my address information
	struct sockaddr_in their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	int i,sn;

	unsigned char keep_running=1;
	fd_set readfs;
	fd_set master;
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;		 // host byte order
	my_addr.sin_port = htons(MYPORT);	 // short, network byte order
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); // automatically fill with my IP
	memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	FD_ZERO(&readfs); //Inicializar
	FD_ZERO(&master);

	FD_SET(sockfd,&master); //Fijar el socket (listenner)

	fdmax=sockfd;

	while(keep_running) {  // main accept() loop
		readfs=master;
		if(select(fdmax+1,&readfs,NULL,NULL,NULL)== -1) {
			perror("select");
			exit(-1);
		}

		//Buscar el descriptor
		int found;
		found=-1;
		for(i=0;i<=fdmax;i++) {
			//printf("FD_ISSET() i:%i\n",i);
			if(FD_ISSET(i,&readfs)) {
				found=i;
				break;
			}
		}
		//printf("found:%i, sockfd is:%i\n",found,sockfd);
		if(found!=-1) {
			if(found==sockfd) { //accept (Se aceptó una nueva conexión)
				//printf("found=sockfd??? %i=%i\n",found,sockfd);
				//printf("accept new connection\n");
				sin_size = sizeof(struct sockaddr_in);
				if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
					perror("accept");
					continue;
				}
				printf("server: got connection from %s , %d\n",inet_ntoa(their_addr.sin_addr),new_fd);

				//Broadcast to all clients
				for(i=0; i<=fdmax; i++) {
					//printf("checking i:%i, found:%i, sockfd:%i, FD_ISSET(i):%i\n",i,found,sockfd,FD_ISSET(i,&master));
					if(i!=found && i!=sockfd && FD_ISSET(i,&master)) {
						//printf("enviant dades a %i fdmax:%i\n",i,fdmax);
						sprintf(buf,"server: got connection from %s , %d\n",inet_ntoa(their_addr.sin_addr),new_fd);
						numbytes=strlen(buf);
						sn=send(i,(const void *)buf,numbytes,0);
						if(sn==-1) {
							perror("send\n");
							continue;
						} else if(sn!=numbytes) {
							fprintf(stderr,"Error, no se enviaron todos los datos?\n");
							continue;
						}
					}
				}
				sprintf(buf,"Welcome %s\n",inet_ntoa(their_addr.sin_addr));
				numbytes=strlen(buf);
				sn=send(i,(const void *)buf,numbytes,0);
				if(sn==-1) {
					perror("send\n");
					continue;
				} else if(sn!=numbytes) {
					fprintf(stderr,"Error, no se enviaron todos los datos?\n");
					continue;
				}

				FD_SET(new_fd,&master);
				fdmax=new_fd>fdmax ? new_fd : fdmax;
				printf("fdmax is now: %i\n",fdmax);
			} else { //Datos recibidos del cliente
				//printf("Data was recieved\n");
				if ((numbytes=recv(found, buf, MAXDATASIZE-1, 0)) == -1) {
					perror("recv");
					exit(1);
				}
				if(numbytes==0) {
					printf("El cliente cerro la conexión %i\n",found);
					close(found);
					FD_CLR(found,&master);
					//Actualizar fdmax
					int new_fdmax=0;
					if(found==fdmax) {
						for(i=0; i<fdmax; i++) {
							if(FD_ISSET(i,&master)) {
								new_fdmax= i>new_fdmax ? i : new_fdmax;
							}
						}
						fdmax=new_fdmax;
						printf("fdmax is now: %i\n",fdmax);
					}
				}
				buf[numbytes] = '\0';
				//printf("Received: %s",buf);

				//Broadcast to all clients
				for(i=0; i<=fdmax; i++) {
					//printf("checking i:%i, found:%i, sockfd:%i, FD_ISSET(i):%i\n",i,found,sockfd,FD_ISSET(i,&master));
					if(i!=found && i!=sockfd && FD_ISSET(i,&master)) {
						//printf("enviant dades a %i fdmax:%i\n",i,fdmax);
						sn=send(i,(const void *)buf,numbytes,0);
						if(sn==-1) {
							perror("send\n");
							continue;
						} else if(sn!=numbytes) {
							fprintf(stderr,"Error, no se enviaron todos los datos?\n");
							continue;
						}
					}
				}
			}
		} else {
			printf("what the hell is going on\n");
		}
	}

	return 0;
}

