/******************************************************************************
* Id: $Id$                       *
*                                                                             *
* Alberto Montañola Lacort                                                    *
*                                                                             *
* Xarxes II                                                                   *
* Màster en Enginyeria de Programari Lliure                                   *
*                                                                             *
*                                                                             *
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

//Some debug stuff
#define __DEBUG__

#ifdef __DEBUG__
#define DBG(a,...) { fprintf(stderr,"DBG[%08X]%i:%s:%s:%i> ",(unsigned int)getpid(),a,__FILE__,__FUNCTION__,__LINE__);\
fprintf(stderr, __VA_ARGS__); fflush(stderr); }
#else
#define DBG(a,...)
#endif

//Data types
//If we have an architecture whose data type does not match the corresponding
// number of bytes, we must change it here.
typedef unsigned char Byte;
typedef unsigned short int U16;
typedef unsigned int U32;

Byte __state_running=1; //Is the server running?

/// Show server usage information
/// @param pgname Program name
void usage(char * pgname) {
	printf("Usage: %s [-p 8000]\n\
 -p <port>: Select the listening port\n\
 -D: Daemon mode\n\
\n",pgname);
}

struct mconfig {
	Byte daemon;
	Byte backlog;
	U16 port;
};

/// Set defaults configuration settings
/// @param config Address of the Configuration struct
void set_config_defaults(struct mconfig * config) {
	memset(config,0,sizeof(struct mconfig));
	config->backlog=10;
	config->port=8000;
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
			config->port=atoi(argv[i]);
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
				exit(0);
			}
			DBG(1,"Shutting down server...\n");
			__state_running=0;
			signal(s,s_handler);
			break;
		case SIGCHLD:
			while(waitpid(-1,NULL,WNOHANG) > 0);
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
	signal(SIGCHLD, s_handler);
}

/// Client structures
struct sclient {
	int sock; ///<! Client Socket
	// Address pair
	sockaddr_in server ///<! Server address
	sockaddr_in client ///<! Client address
};

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
	server.sin_port = htons(config->port); //Set the port
	server.sin_addr.s_addr = htonl(INADDR_ANY); //Any address
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
	
	//Server Main Loop
	int csock;
	struct sockaddr_in client;
	int client_size=sizeof(struct sockaddr);
	struct sockaddr_in query_server;
	int qsize=sizeof(struct sockaddr);

	while(__state_running) {
		//blocking call
		if((csock=accept(sock,(struct sockaddr *)&client,&client_size))==-1) {
			perror("accept");
			continue;
		}

		// Determine allocated address
		if(getsockname(csock,(struct sockaddr *)&query_server,&qsize) == -1) {
			perror("getsockname");
			close(csock);
			continue;
		}
		DBG(1,"Got Connection from %s:%i to %s:%i\n",
		inet_ntoa(client.sin_addr),ntohs(client.sin_port),
		inet_ntoa(query_server.sin_addr),ntohs(query_server.sin_port));
		
		// Fork
		if(!fork()) {
			// Child
			close(sock);
			// Process Child Petition
			if (send(csock,"Error 500 Service Unavailable\n",30,0) == -1) {
				perror("send");
			}
			sleep(10);
			close(csock);
			exit(0);
		}
		close(csock);

	}
	close(sock);
	return 0;
}

/// Main
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



