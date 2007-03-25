#ifndef XSERVER_H
#define XSERVER_H

//Data types
typedef unsigned char Byte;
typedef unsigned short int U16;
typedef unsigned int U32;

/// Encapsulate errors into this exception
class errorException: public std::exception {
private:
	int _errno;
	const char * _in;
public:
	errorException(const char * in);
	virtual const char * what() const throw();
};

class server;

class sessionMGR {
private:
	server * _parent;
public:
	sessionMGR(server * parent);
	void add(int sock);
	void remove(int sock);
};


class selectInterface {
private:
	fd_set _master;
	fd_set _readfs;
	int _fdmax;
	std::queue<int> _descs;
public:
	selectInterface();
	void register2(int fdesc);
	void unregister(int fdesc);
	std::queue<int> & wait();
};


class server {
private:
	Byte _keep_running;
	int _socket;
	int _backlog;
	std::string _bindAddr;
	U16 _bindPort;
	sessionMGR * _clients;
public:
	//Porqueries de les que Odia en JMG ;)
	selectInterface _select;
public:
	server(const std::string lhost="",const U16 lport=8642,const Byte backlog=10);
	~server();
	void setBindAddress(const std::string lhost,const U16 lport);
	void startOp();
	void stopOp();
	void requestLoop();
	void run();
	int proccessRequest(int csock, char * buf, int n);
	int mysend(int csock,char * msg);
};

#endif
