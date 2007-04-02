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

#include "protocol.h"
#include "netcommon.h"
#include "partialDataReader.h"

/// Client class
class client {

private:
	bool _keep_running;
	ClientStatus _state;
	std::queue<std::string> _command_stack;
	std::queue<std::string> _pm_stack;
	std::string _nick;
	std::string _server_addr;
	U16 _server_port;
	std::string _bindAddr;
	U16 _bindPort;
	int _sockudp;
	int _sockfd;
	U16 _udpport;
	PartialDataReader _partialData;
public:
	//Porqueries de les que Odia en JMG ;)
	selectInterface _select;
public:

	/// Constructor
	/// @param login Username
	/// @param server_addr Server Address
	/// @param server_port Server port
	/// @param lhost UDP bind address (NOT IMPLEMENTED) FIXME (uses 0 = all available addresses)
	/// @param lport UDP bind port
	client(const std::string login="anonymous",const std::string server_addr="localhost",U16 port=8642,
		const std::string lhost="",U16 lport=7766);

	/// Creates the udp socket
	void startOp();
	
	/// Cleanup code
	void stopOp();

	/// Main client loop
	void requestLoop();

	/// Creates and connects the tcp socket
	void connect2();

	/// Runs the Main aplication loop
	void run();

	/// Sets the nick
	/// @param nick The nick to set
	void setNick(const std::string nick);

	/// Sends all data throught the TCP socket
	/// @param msg Message to send
	void sendall(const char * msg);

	/// Sends a command to be run in the server
	/// @param cmd Command to run
	void sendCmd(const std::string cmd);

	/// Sends the hello request to the server
	void sendHello();

	/// User shutdown
	void usershutdown();

	/// Show client usage information
	void help(void);

	/// Process recived data of the user
	/// @param req User data to be processed
	void processUserInput(const std::string req);

	/// Process recived data of the server
	/// @param req The request recieved from server
	void proccessServerResponse(const std::string req);

	/// Sends a broadcast message
	/// @param msg The message to send
	void sendBcastMsg(const std::string msg);

	/// Sends a private message by p2p
	/// @param nick Destination Username
	/// @param msg The message
	void sendp2pMsg(const std::string nick, const std::string msg);

	/// Sends a p2p message
	/// @param ip Destination ip address
	/// @param p Destination port
	/// @param msg The message
	void sendp2pMsg2peer(const std::string ip,U16 p,const std::string msg);

	/// Show client prompt
	void writePrompt();

	/// Handles some signals
	/// @param s Signal number
	void signalHandler(int s);

	/// Installs all signal handlers
	void installSignalHandlers();

};


/// Configuration struct
struct mconfig {
	int port;
	std::string login;
	std::string server_addr;
	U16 server_port;
};

