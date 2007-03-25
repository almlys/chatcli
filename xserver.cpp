
#include <iostream>
#include <string>
#include <exception>
#include <queue>

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "xserver.h"

//I really hate this, pero no buscamos finezas
#define MAXDATASIZE 4096
//End hate list


errorException::errorException(const char * in) {
	_errno=errno;
	_in=in;
}

const char * errorException::what() const throw() {
	std::string errortext;
	char buf[100];
	sprintf(buf,"%s: %i",_in,_errno);
	errortext = buf;
	errortext += " ";
	errortext += strerror(_errno);
	return errortext.c_str();
}

selectInterface::selectInterface() {
	FD_ZERO(&_readfs);
	FD_ZERO(&_master);
	_fdmax=0;
}

void selectInterface::register2(int fdesc) {
	FD_SET(fdesc,&_master);
	_fdmax = fdesc>_fdmax ? fdesc : _fdmax;
}

void selectInterface::unregister(int fdesc) {
	FD_CLR(fdesc,&_master);
	if(_fdmax == fdesc) {
		int i;
		for(i=fdesc-1; i>3; i++) {
			if(FD_ISSET(i,&_master)) {
				_fdmax=i;
				break;
			}
		}
	}
}

std::queue<int> & selectInterface::wait() {
	while(!_descs.empty()) _descs.pop();

	_readfs = _master;
	if(select(_fdmax+1,&_readfs,NULL,NULL,NULL) == -1) {
		if(errno==EINTR) return _descs; //A signal was cauch
		throw errorException("select");
	}

	int i;
	for(i=0; i<=_fdmax; i++) {
		if(FD_ISSET(i,&_readfs)) {
			_descs.push(i);
		}
	}
	return _descs;
}

sessionMGR::sessionMGR(server * parent) {
	_parent=parent;
}

void sessionMGR::add(int sock) {
	_parent->_select.register2(sock);
}

void sessionMGR::remove(int sock) {
	if(close(sock)!=0) throw errorException("close");
	_parent->_select.unregister(sock);
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


/// Send a message to the client
int server::sendall(const int csock,const char * msg) {
	int size=strlen(msg);
	int total=0;
	int sn;
	while(total < size) {
		sn=send(csock,msg+total,size-total,0);
		if(sn==-1) throw errorException("send");
		total += sn;
	}
	return 0;
}

int server::sendok(const int csock,const char * msg) {
	std::string buf = "100 ";
	buf += msg;
	buf += "\n";
	return sendall(csock,buf.c_str());
}

int server::senderror(const int csock,const char * msg) {
	std::string buf = "200 ";
	buf += msg;
	buf += "\n";
	return sendall(csock,buf.c_str());
}

/// Process a client request
int server::proccessRequest(const int csock,const char * buf) {
	printf("Processing request: %s\n",buf);

	//struct sockaddr_in client;
	//socklen_t client_size=sizeof(struct sockaddr);

	/*//Get client address
	if(getsockname(csock,(struct sockaddr *)&client,&client_size) == -1) {
		perror("getsockname");
		return -1;
	}

	DBG(1,"Got Connection from %s:%i\n",
		inet_ntoa(client.sin_addr),ntohs(client.sin_port));
	*/

	if(!strncmp(buf,"HELO\n",5)) {
		//HELO message
		//enviar OK
		sendok(csock,"OK");
	} else if (!strncmp(buf,"REGISTER ",9)) {
		sendok(csock,"OK");
	} else if (!strncmp(buf,"QUERY ",9)) {
		senderror(csock,"ERROR\n");
	} else if (!strncmp(buf,"BCAST ",9)) {
		sendok(csock,"OK");
	} else if (!strncmp(buf,"EXIT ",9)) {
		sendok(csock,"OK");
	} else {
		senderror(csock,"ERROR\n");
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
			_clients->add(csock);
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
			printf("Received: %s",buf);
			try {
				proccessRequest(client,buf);
			} catch(std::exception &e) {
				std::cout<<"Notice: Exception proccessig a request: "<<e.what()<<std::endl;
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

