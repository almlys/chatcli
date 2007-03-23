/******************************************************************************
* Id: $Id$                       *
*                                                                             *
* Alberto Montañola Lacort                                                    *
*                                                                             *
* Xarxes II                                                                   *
* Màster en Enginyeria de Programari Lliure                                   *
*                                                                             *
* Pràctica I                                                                  *
*                                                                             *
******************************************************************************/

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

//Some debug stuff
#define __DEBUG__

#ifdef __DEBUG__
#define DBG(a,...) { fprintf(stderr,"DBG[%08X]%i:%s:%s:%i> ",(unsigned int)getpid(),a,__FILE__,__FUNCTION__,__LINE__);\
fprintf(stderr, __VA_ARGS__); fflush(stderr); }
#else
#define DBG(a,...)
#endif

#define MAXDATASIZE 100

//Data types
typedef unsigned char Byte;
typedef unsigned short int U16;
typedef unsigned int U32;

//Global vars
Byte __state_running=1; //Is the server running?
//End global vars list

/// Show server usage information
/// @param pgname Program name
void usage(char * pgname) {
	printf("Usage: %s [-p 8642] [-b 10] [-D]\n\
 -p <port>: Select the listening port\n\
 -b <backlog>: Set how many pending connections the queue will hold\n\
 -D: Daemon mode\n\
\n",pgname);
}

/// Configuration struct
struct mconfig {
	Byte daemon;
	U32 backlog;
	U16 port;
};

/// Set defaults configuration settings
/// @param config Address of the Configuration struct
void set_config_defaults(struct mconfig * config) {
	memset(config,0,sizeof(struct mconfig));
	config->backlog=10;
	config->port=8642;
}

/// Parse command line args
/// @param config Address of the destination Configuration struct
/// @param argc Number of args
/// @param argv Args vector
/// @returns 0 if everything went well, non-zero on error
char parse_argv(struct mconfig * config, int argc, char * argv[]) {
	int i;
	for (i=1; i<argc; i++) {
		if(!strcmp(argv[i],"-h")) {
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
			usage(argv[0]); return -1;
		}
	}
	return 0;
}

/// Server Signal Handler
/// @param s Signal number
void s_handler(int s) {
	DBG(1,"Recieved signal %i\n",s);
	switch(s) {
		case SIGTERM:
		case SIGINT:
			if(__state_running==0) {
				DBG(1,"Server killed!\n");
				exit(-1);
			}
			DBG(1,"Shutting down server...\n");
			__state_running=0;
			signal(s,s_handler);
			break;
		default:
			DBG(1,"Error: Unexpected signal recieved!\n");
	}
}

/// Install Signal Handlers
void install_handlers() {
	signal(SIGTERM, s_handler);
	signal(SIGINT, s_handler);
}

/// Client structures
/*struct sclient {
	int sock; ///<! Client Socket
	// Address pair
	sockaddr_in server ///<! Server address
	sockaddr_in client ///<! Client address
};*/


/// Process a client request

int mysend(int csock,char * msg) {
	int size=strlen(msg);
	int sn;
	sn=send(csock,(const void *)msg,size,0);
	if(sn==-1) {
		perror("send\n");
		return -1;
	} else if(sn!=size) {
		fprintf(stderr,"Error, no se enviaron todos los datos?\n");
		return -1;
	}
	return 0;
}

int proccess_request(int csock, char * buf, int n) {
	printf("Processing request: %s\n",buf);

	if(!strncmp(buf,"HELO\n",5)) {
		//HELO message
		//enviar OK
		mysend(csock,"200 OK\n");
	} else {
		mysend(csock,"400 ERROR\n");
	}

	return 0;
}

/// Server Loop
/// @param config Address of the configuration struct
/// @returns 0 if everything went well, non-zero on error
int server_loop(struct mconfig * config) {
	//Startup
	//Create the socket (TCP)
	int sock;
	if((sock=socket(PF_INET,SOCK_STREAM,0)) == -1) {
		perror("socket");
		return -1;
	}
	// Set socket options
	// Set Reuse Address
	int reuse=1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
		perror("setsockopt");
		return -1;
	}
	
	// Bind the socket
	struct sockaddr_in server; //Local server address
	memset(&server,0,sizeof(struct sockaddr));
	server.sin_family = PF_INET; //Ipv4
	server.sin_port = htons(config->port); //Set the port in Network Byte order
	server.sin_addr.s_addr = htonl(INADDR_ANY); //Any address in Network Byte order
	if(bind(sock,(struct sockaddr *)&server,sizeof(struct sockaddr))<0) {
		perror("bind");
		return -1;
	}
	
	// Listen
	if(listen(sock,(int)config->backlog) == -1) {
		perror("listen");
		return -1;
	}
	
	DBG(1,"Listening to incoming TCP connections on port %i\n",config->port);
	
	// ** Server Main Loop **
	char buf[MAXDATASIZE+1];
	fd_set readfs;
	fd_set master;
	int fdmax, csock, numbytes; //, sn;
	int i,e,found;
	struct sockaddr_in client;
	socklen_t client_size=sizeof(struct sockaddr);

	//Setup select structures
	FD_ZERO(&readfs);
	FD_ZERO(&master);

	FD_SET(sock,&master); //Set the server socket (listenner)
	
	fdmax = sock;

	while(__state_running) {
		// main accept() loop

		//refresh select structures
		readfs = master;
		if(select(fdmax+1,&readfs,NULL,NULL,NULL) == -1) {
			if(errno==EINTR) break; //A signal was cauch
			perror("select");
			return -1;
		}

		//Find those descriptors where data is available to be read
		found=-1;
		for(e=0;e<=fdmax;e++) {
			//printf("FD_ISSET() i:%i\n",i);
			if(FD_ISSET(e,&readfs)) {
				found=e;
				// We found one of them

				if(found==sock) { //accept (Se aceptó una nueva conexión)
					//printf("found=sockfd??? %i=%i\n",found,sock);
					//printf("accept new connection\n");
					client_size = sizeof(struct sockaddr_in);
					//blocking call
					if ((csock = accept(sock, (struct sockaddr *)&client, &client_size)) == -1) {
						perror("accept");
						continue;
					}
					printf("server: got connection from %s , %d\n",inet_ntoa(client.sin_addr),csock);
	
					FD_SET(csock,&master);
					fdmax=csock>fdmax ? csock : fdmax;
					printf("fdmax is now: %i\n",fdmax);
				} else { //Datos recibidos del cliente
					//printf("Data was recieved\n");
					if ((numbytes=recv(found, buf, MAXDATASIZE, 0)) == -1) {
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
							for(i=3; i<fdmax; i++) {
								if(FD_ISSET(i,&master)) {
									new_fdmax= i>new_fdmax ? i : new_fdmax;
								}
							}
							fdmax=new_fdmax;
							printf("fdmax is now: %i\n",fdmax);
						}
					}
					buf[numbytes] = '\0';
					printf("Received: %s",buf);
					proccess_request(found,buf,numbytes);
				}
			}
		}
	}

	//Close all connections
	for(i=3; i<=fdmax; i++) {
		if(i!=sock && FD_ISSET(i,&master)) {
			if(close(i)!=0) {
				perror("close");
				continue;
			}
		}
	}
	if(close(sock)!=0) {
		perror("close");
	}
	return 0;
}

/// Main Program
int main(int argc, char * argv[]) {
	struct mconfig config;
	set_config_defaults(&config);
	if(parse_argv(&config,argc,argv)) { return -1; }
	install_handlers();
	//Daemon mode?
	if(config.daemon) {
		daemon(1,0);
	}
	return(server_loop(&config));
}



