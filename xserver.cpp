
#include <iostream>
#include <string>
#include <exception>
#include <queue>
#include <map>

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "xserver.h"

//I really hate this, pero no buscamos finezas
#define MAXDATASIZE 4096
//End hate list

//Protocol stuff
const char * protocol::sep="\r\n";
// "\n" *nix systems
// "\r" Old Mac systems
// "\r\n" Hasecorp Hasefroch systems
const char * protocol::ok="100";
const char * protocol::error="200";
const char * protocol::helo="HOLA";
const char * protocol::register2="300";
const char * protocol::query="400";
const char * protocol::answer="500";
const char * protocol::pm="600";
const char * protocol::bcast="700";
const char * protocol::exit="800";


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
	printf("registe: %i\n",fdesc);
	FD_SET(fdesc,&_master);
	_fdmax = fdesc>_fdmax ? fdesc : _fdmax;
}

void selectInterface::unregister(int fdesc) {
	FD_CLR(fdesc,&_master);
	if(_fdmax == fdesc) {
		int i;
		for(i=fdesc-1; i>3; i--) {
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
			//printf("%i", i);
		}
	}
	return _descs;
}


clientSession::clientSession(int socket,U32 addr) {
	_socket=socket;
	_addr=addr;
	_status=kNew;
}

int clientSession::fileno() const {
	return _socket;
}

const char * clientSession::getName() const {
	if(strlen(_name.c_str())==0) {
		return NULL;
	}
	return _name.c_str();
}

U32 clientSession::getAddr() const {
	return _addr;
}

bool clientSession::isRegistered() const {
	return _status==kRegister;
}

/// Send a message to the client
void clientSession::sendall(const char * msg) {
	int size=strlen(msg);
	int total=0;
	int sn;
	while(total < size) {
		sn=send(_socket,msg+total,size-total,0);
		if(sn==-1) throw errorException("send");
		total += sn;
	}
}

void clientSession::sendOk(const char * msg) {
	std::string buf = protocol::ok;
	//buf += msg;
	buf += protocol::sep;
	sendall(buf.c_str());
}

void clientSession::senderror(const char * msg) {
	std::string buf = protocol::error;
	//buf += msg;
	buf += protocol::sep;
	sendall(buf.c_str());
}


sessionMGR::sessionMGR(server * parent) {
	_parent=parent;
}

sessionMGR::~sessionMGR() {

}

std::map<U32,clientSession *> & sessionMGR::getAllClients() {
	return _clients;
}

void sessionMGR::add(int sock,U32 addr) {
	clientSession * cli = new clientSession(sock,addr);
	if(_clients.find(addr) != _clients.end()) {
		printf("Found! deleting...\n");
		remove(_clients[addr]);
	}
	_clients[addr]=cli;
	_sockets[sock]=cli;
	_parent->_select.register2(sock);
}

void sessionMGR::remove(clientSession * client) {
	if(close(client->fileno())!=0) throw errorException("close");
	_parent->_select.unregister(client->fileno());
	if(client->getName()!=NULL) {
		std::string out=client->getName();
		out+=" has left the chat";
		_parent->broadcast(out.c_str(),client);
		_nicks.erase(client->getName());
	}
	_clients.erase(client->getAddr());
	_sockets.erase(client->fileno());
	delete client;
}


void sessionMGR::remove(int sock) {
	remove((clientSession *)find(sock));
}

const clientSession * sessionMGR::find(int sock) {
	return _sockets[sock];
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

void server::broadcast(const char * msg,const clientSession * client) {
	std::map<U32,clientSession *> clients;
	clients=_clients->getAllClients();
	std::map<U32,clientSession *>::iterator iter;
	for(iter = clients.begin(); iter!=clients.end(); iter++) {
		if(iter->second->fileno()!=client->fileno() && iter->second->isRegistered()) {
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

	if(!strncmp(buf,"HOLA\n",5)) { //Identificacio
		sendok(csock,"OK");
	} else if (!strncmp(buf,"300 ",4)) { //Registre
		sendok(csock,"OK - Es demanen per registrar");
	} else if (!strncmp(buf,"400 ",4)) { //Pregunta
		sendok(csock,"OK - Ens pregunten");
	} else if (!strncmp(buf,"600 ",4)) { //Privat
		sendok(csock,"OK - Enviem un privat");
	} else if (!strncmp(buf,"700 ",4)) { //Difusio
		sendok(csock,"OK - Enviem a tothom");
	} else if (!strncmp(buf,"800\n", 4)) { //Sortir
		sendok(csock,"OK - Sortim"); 
		//jo krek k aki hauriam de fer algo no? o deixem k aktui el propi while de requesLoop k ja donara de baixa el klient solet???
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

