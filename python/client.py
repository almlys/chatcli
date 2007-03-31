#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# $Revision$
#
#    Copyright (C) 2007 Ignasi Barri Vilardell
#    Copyright (C) 2007 Alberto Montañola Lacort
#    Copyright (C) 2007 Josep Rius Torrento
#    See the file AUTHORS for more info
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
#    USA.
#
#    Please see the file COPYING for the full license.
#

"""
Pràctica I
Xarxes II

Implementació d'un petit client de chat

Execució:
./client.py nick@server:port [-p 0] [-bind 0] [-buf 4096]
-p <port>: Select the listenning udp port (default 0 = randomly assigned by OS)
-buf <size>: Sets the incomming buffer size (default 4096)
-bind <address>: Sets the address to bind (default 0 = all available addresses on all interfaces)
"""

import socket
from netcommon import selectInterface
from cprotocol import *

class client(object):
    """
    Client app
    """

    keep_running = True
    state = ClientStatus.new
    command_stack = []

    def __init__(self,login="anonymous",server=("localhost",8642),lhost="",lport=7766,buf=4096):
        """
        Constructor of the client app
        @param server Server address/port tuple
        @param lhost Udp bind address
        @param lport Udp bind port
        @param buf Input buffer size
        """
        self.udpsocket=None
        self.tcpsocket=None
        self.setBindAddress(lhost,lport)
        self.serveraddr=server
        self.bufferSize = buf
        self.nick = login

    def setBindAddress(self,lhost,lport):
        """
        Sets the udp server bind address (adress/port) pair
        @param lhost Udp bind address
        @param lport Udp bind port
        """
        self.bindAddr=lhost
        self.bindPort=lport

    def startOp(self):
        """
        Creates the UDP socket
        """
        self.installSignalHandlers()
        self.udpsocket=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # set Reuse Address
        #self.udpsocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.udpsocket.bind((self.bindAddr,self.bindPort))
        print "DBG: Listening to incoming datagrams on %s port udp %s" %(self.udpsocket.getsockname())
        self.initialized=True
        self.select = selectInterface()
        import sys
        self.select.register(sys.stdin)
        self.select.register(self.udpsocket)

    def connect(self,address=("localhost",8642)):
        """
        Creates and Connects the tcp socket to the specified address
        @param address Ip/port server address pair
        """
        self.tcpsocket=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.tcpsocket.connect(address)
        self.select.register(self.tcpsocket)
        self.sendCmd(protocol.helo)

    def sendCmd(self,cmd):
        """
        Sends a command to be run in the server
        """
        self.tcpsocket.sendall(cmd + protocol.sep)

    def setNick(self,nick):
        """
        Sets the nick
        @param nick Username to send to the server
        """
        self.sendCmd(protocol.register + " " + nick + " " + self.udpsocket.getsockname()[1])
        
    def stopOp(self):
        """
        Cleanup code
        """
        self.udpsocket.close()
        del self.udpsocket
        self.tcpsocket.close()
        del self.tcpsocket

    def requestLoop(self):
        """
        Main client loop
        """
        import sys
        read, write, excp = self.select.wait()
            
        for desc in read:
            if desc==sys.stdin:
                # data was recieved from stdin
                #print "stdin data"
                data = desc.readline()
                if(len(data)==0):
                    # The user has sent a EOF (Ctrl+D), and now stdin is closed
                    self.usershutdown()
                    continue
                self.processUserInput(data.strip())
                #print data
            elif desc==self.tcpsocket:
                # Data was recieved from the server
                data = desc.recv(self.bufferSize)
                if len(data)==0:
                    print "Connection closed by server"
                    desc.close()
                    self.keep_running=False
                print data
            else:
                # Data was recieved from a client
                data,addr = desc.recvfrom(self.bufferSize)
                print addr,data

    def run(self):
        """
        Starts the Main Client Loop
        """
        self.startOp()
        self.connect(self.serveraddr)
        while(self.keep_running):
            self.requestLoop()
        self.stopOp()

    def usershutdown(self):
        """
        User shutdown
        """
        self.sendCmd(protocol.exit)

    def processUserInput(self,msg):
        try:
            cmd, data = msg.split(':',1)
        except ValueError:
            cmd = msg
            data = ""
        if len(data)==0:
            if cmd=="salir":
                self.usershutdown()
            if cmd=="ayuda":
                print """
**** Client Help system ****
===============================================================================
   command                               action
 ayuda           Shows this help
 todos: msg      Sends a message to everybody connected in the network
 <nick>: msg     Sends a private message to nick
 salir           Terminates the program
                       This client has Super Cow Powers.
"""
            elif cmd=="moo":
                print """
         (__)
         (oo)
   /------\\/
  / |    ||
 *  /\\---/\\
    ~~   ~~
...."Have you mooed today?"...
"""
            else:
                print "Unknown command %s" %(cmd,)

        elif self.state == ClientStatus.register:
            if cmd=="todos":
                self.sendBcastMsg(data)
            else:
                print data
        else:
            print "Error, Cannot send a message, because the client is still not connected"
        print "cliente > ",
        import sys
        sys.stdout.flush()

    def sendBcastMsg(self,msg):
        """
        Send a broadcast message
        """
        self.command_stack.append(protocol.bcast)
        self.sendCmd(protocol.bcast + " " + data)
    
    def signalHandler(self,num,frame):
        """
        A signal handler
        @param num Signal number
        @param frame Current stack frame
        """
        import signal
        if self.keep_running:
            self.keep_running=False
            signal.signal(num,self.signalHandler)
        else:
            import sys
            print "Killed!"
            sys.exit()

    def installSignalHandlers(self):
        """
        Installs some signal handlers
        """
        import signal
        signal.signal(signal.SIGTERM, self.signalHandler)
        signal.signal(signal.SIGINT, self.signalHandler)

def usage():
    """
    Prints some usage useful, nice help
    """
    import sys
    print """%s [nick@server:port] [-p 0] [-bind 0] [-buf 4096]
-p <port>: Select the listenning udp port (default 0 = randomly assigned by OS)
-buf <size>: Sets the incomming buffer size (default 4096)
-bind <address>: Sets the address to bind (default 0 = all available addresses on all interfaces)
""" %(sys.argv[0])


def parse_args():
    """
    Parses some command line arguments
    """
    config={"port" : 0, "buf" : 4096, "bind" : "", "login" : "",
            "server_port" : 8642, "server_addr" : "" }
    import sys
    n=len(sys.argv)
    i=1
    while i<n:
        arg=sys.argv[i]
        if arg=="-h" or arg=="--help":
            usage()
            sys.exit(0)
        elif arg=="-p" and i+1<n:
            i+=1
            config["port"]=int(sys.argv[i])
        elif arg=="-buf" and i+1<n:
            i+=1
            config["buf"]=int(sys.argv[i])
        elif arg=="-bind" and i+1<n:
            i+=1
            config["bind"]=sys.argv[i]
        else:
            t = sys.argv[i].split("@")
            if len(t)>1:
                login = t[0]
                server = t[1]
            else:
                login = ""
                server = t[0]
            t = server.split(":")
            if len(t)>1:
                server = t[0]
                port = t[1]
            else:
                server = t[0]
                port = 8642
            config["server_port"]=int(port)
            config["server_addr"]=server
            config["login"]=login

        i+=1
    return config


def main():
    """
    Main entry point
    """
    try:
        try:
            config=parse_args()
        except:
            usage()
            import sys
            sys.exit(0)
        # Init state
        try:
            while config["login"]=="":
                config["login"]=raw_input("cliente> esperando un identificador de usuario: ")
            while config["server_addr"]=="":
                config["server_addr"]=raw_input("cliente> esperando la dirección de un servidor: ")
        except EOFError:
            import sys
            sys.exit(0)            
        cli=client(config["login"],(config["server_addr"],config["server_port"]),
                   config["bind"],config["port"],config["buf"])
        print "cliente > ",
        import sys
        sys.stdout.flush()
        cli.run()
    except SystemExit:
        pass
    except: # Esto es el control de errores, que más quieres!??
        import traceback,sys
        print "Error: Check the traceback!!"
        trace=file("traceback.log","w")
        traceback.print_exc(file=trace)
        trace.close()
        traceback.print_exc(file=sys.stderr)

if __name__ == "__main__":
    main()
