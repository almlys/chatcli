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

Implementació d'un petit servidor de chat

Execució:
./server.py [-p 8642] [-b 10] [-D] [-buf 4096] [-nw] [-bind 0] [-ctt] [-e]
-p <port>: Select a diferent listenning tcp port (default 8642)
-b <backlog>: Set how many pending connections the queue will hold (default 10)
-D Daemon mode
-buf <size>: Sets the incomming buffer size (default 4096)
-nw: Forbides spaces in the nicknames (default are enabled)
-bind <address>: Sets the address to bind (default 0 = all available addresses on all interfaces)
-ctt: Cleans telnet protocol characters <IAC>,<TOF>,<OPT> (default dissabled)
-e: Allow empty cmds from client (default dissabled)
"""

# TODO: Will be nice to Allow to bind to an specific interface, for those machines with multiple interfaces.

import socket
from netcommon import selectInterface
from cprotocol import *
from sessionMGR import sessionMGR, clientSession


class server(object):
    """
    The server
    """

    keep_running = True
    shutting_down = False

    def __init__(self,lhost="",lport=8642,backlog=10,bufsize=4096,nw=True,emptycmds=False,trash=False,egg=0xf):
        """
        Constructor
        @param lhost Bind address (default all)
        @param lport Bind port (default 8642)
        @param backlog Set how many pending connections the queue will hold (default 10)
        @param bufsize Sets the incomming buffer size (default 4096)
        @param nw Allow whitespaces in the nicknames (default True)
        @param emptycmds Allow empty commands
        @param trash Trash telnet protocol chars
        """
        self.socket=None
        self.initialized=False
        self.setBindAddress(lhost,lport)
        self.backlog=backlog
        self.bufferSize = bufsize
        self.allowwhitespaces = nw
        self.shutdowncount = egg
        self.trash_telnet_garbage = trash
        self.allow_empty_cmds = emptycmds
        self.clients = sessionMGR(self)

    def setBindAddress(self,lhost,lport):
        """
        Sets the server bind address (adress/port) pair
        @param lhost Bind address
        @param lport Bind Port
        """
        self.bindAddr=lhost
        self.bindPort=lport

    def startOp(self):
        """
        Creates the socket
        """
        if self.initialized:
            return
        self.installSignalHandlers()
        self.socket=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # set Reuse Address
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.bind((self.bindAddr,self.bindPort))
        self.socket.listen(self.backlog)
        print "DBG: Listening to incoming connections on %s port tcp %s" %(self.socket.getsockname())
        self.initialized=True
        self.select = selectInterface()
        self.select.register(self.socket)
        
    def stopOp(self):
        """
        Destroys the socket
        """
        # this forces a close() to be done by all clients
        # Python automatically calls close, when the socket reference count gets to 0
        # with this del, we force the destruction of the sessionMGR, reducing the reference
        # count of all clients sockets to 0, invoking the implicit socket.close() call inside
        # the destructor. very nice for python, but in the C++ version, we must do this in a manual
        # way (remember that C++ lacks of a garbage collector)
        del self.clients
        self.socket.close() # this is not really necessary, the del statement will call close
        del self.socket
        self.initialized=False

    def broadcast(self,msg,client=None):
        """
        Sends a broadcast message to all connected clients, who are in the registered status
        The other clients, will not be notified
        @param msg Message to sent
        @param client Client to ignore in the broadcast (this is useful for ignoring messages originating from that client)
        """
        for key in self.clients.clients:
            try:
                cli = self.clients.clients[key]
                if cli!=client and cli.isRegistered():
                    cli.send(protocol.bcast + " " + msg + protocol.sep)
            except socket.error:
                #Ignorar exceptiones producidas al enviar un mensaje
                print "Excepcion sending data..."
                import traceback,sys
                traceback.print_exc(file=sys.stderr)            


    def requestLoop(self):
        """
        Request main LOOP
        """
        read, write, excp = self.select.wait()
            
        for client in read:
            if client==self.socket:
                # accept (A new connection was accepted)
                self.clients.add(*client.accept())
            else:
                # Data was recieved from a client
                data = client.conn.recv(self.bufferSize)
                #print "Partial data ->%s<-" %(data,)
                if len(data)==0:
                    self.clients.remove(client)
                    continue
                for req in client.partialData.feed(data):
                    self.processRequest(client,req)

    def processRequest(self,client,msg):
        """
        Process a client request
        @param client Client object who has sent the request
        @param data Raw data recieved from the client
        """
        print "From %s:%s" %(client.addr)
        #print "->%s<-" %(msg,)

        try:
            cmd, data = msg.split(' ',1)
        except ValueError:
            cmd = msg
            data = ""

        port = 0
        try:
            print "processing cmd: %s, data: %s" %(cmd,data)

            def getNickAndPort(self,data,portc=True):
                port=0
                nick=""
                if len(data)!=0:
                    if self.allowwhitespaces:
                        if portc:
                            t = data.rsplit(' ',1)
                            nick = t[0]
                            if len(t)>1:
                                try:
                                    port = int(t[1])
                                except ValueError: pass
                        else:
                            nick = data
                    else:
                        t = data.split()
                        nick = t[0]
                        print len(t),port
                        if len(t)>1:
                            try:
                                port = int(t[1])
                            except ValueError: pass
                return (nick,port)

            #print "nick: %s, port: %i" %(nick,port)

            if cmd==protocol.helo and not client.isHallowed():
                client.rcvHello()
                client.sendOk("OK %s:%s" % (client.addr))
            elif (client.isHallowed() or client.isRegistered()) \
                 and cmd==protocol.register:
                nick, port = getNickAndPort(self,data,True)
                if port!=0 and len(nick)!=0:
                    self.clients.register(client,nick,port)
                    client.sendOk()
                else:
                    raise ProtocolViolation,"Syntax error"
            elif client.isRegistered() and cmd==protocol.query:
                nick, port = getNickAndPort(self,data,False)
                if len(nick)!=0:
                    client.sendanswer("%s" %(self.clients.findAddress(nick)))
                else:
                    raise ProtocolViolation,"Syntax error"
            elif client.isRegistered() and cmd==protocol.bcast:
                author=client.name
                self.broadcast(author + " says: " + data,client)
                client.sendOk("OK")
            elif cmd==protocol.exit:
                self.clients.remove(client)
            else:
                #client.senderror("Unknown cmd:%s" %(cmd,))
                raise ProtocolViolation,"Unknown cmd"
        
##        except HelloAlreadySent:
##            client.senderror("HelloAlreadySent")
##        except NickAlreadyExists:
##            client.senderror("NickAlreadyExists")
##        except BadFormatedMsg:
##            client.senderror("BadFormatedMsg")
##        except NoHelloWasSent:
##            client.senderror("NoHelloWasSent")
##        except NickNotFound:
##            client.senderror("NickNotFound")
        except ProtocolViolation,e:
            print e
            client.senderror()
            self.clients.remove(client)
        except (socket.error, ValueError):
            #Ignorar exceptiones producidas al procesar una petición
            print "Excepcion procesando petición..."
            import traceback,sys
            traceback.print_exc(file=sys.stderr)
            client.senderror()
            self.clients.remove(client)

    def run(self):
        """
        Starts the server Main Loop
        """
        self.startOp()
        while(self.keep_running):
            try:
                self.requestLoop()
            except socket.error:
                print "Excepcion in request loop..."
                import traceback,sys
                traceback.print_exc(file=sys.stderr)
        self.stopOp()

    def signalHandler(self,num,frame):
        """
        Handles some signals
        @param num Signal number
        @param frame Current stack frame
        """
        import signal
        if num==signal.SIGUSR1:
            signal.signal(num,self.signalHandler)
            if self.shutting_down:
                msg = "NOTICE: Self-Destruction cancelled at %i seconds left by admin" %(self.shutdowncount)
                print msg
                self.broadcast(msg)
                self.shutting_down=False
            else:
                msg = "NOTICE: Self-Destruction activated!!"
                print msg
                self.broadcast(msg)
                signal.alarm(1)
                self.shutting_down=True
                self.shutdowncount=15
        elif num==signal.SIGALRM:
            signal.signal(num,self.signalHandler)
            if self.shutting_down:
                if(self.shutdowncount<=0):
                    self.keep_running=False
                    msg = "KABOOUM!!!!"
                    self.broadcast(msg)
                    print msg
                else:
                    msg = "NOTICE: %i seconds left for the self-destruction" %(self.shutdowncount)
                    print msg
                    self.broadcast(msg)
                    self.shutdowncount-=1
                    signal.alarm(1)
        elif num==signal.SIGTERM or num==signal.SIGINT:
            if self.keep_running:
                self.keep_running=False
                self.broadcast("I'm being killed!! aie!!")
                signal.signal(num,self.signalHandler)
            else:
                import sys
                print "The server was brutally assasinated by the administrator!"
                sys.exit()

    def installSignalHandlers(self):
        """
        Installs all those nice shiny signal handlers
        Mainly to avoid that hateful "KeyboradInterrupt" exception
        """
        import signal
        signal.signal(signal.SIGTERM, self.signalHandler)
        signal.signal(signal.SIGINT, self.signalHandler)
        signal.signal(signal.SIGUSR1, self.signalHandler)
        signal.signal(signal.SIGALRM, self.signalHandler)


def usage():
    """
    Prints some usage useful, nice help
    """
    import sys
    print """%s [-p 8642] [-b 10] [-D] [-buf 4096] [-nw] [-bind 0] [-ctt] [-e]
-p <port>: Select a diferent listenning tcp port (default 8642)
-b <backlog>: Set how many pending connections the queue will hold (default 10)
-D Daemon mode
-buf <size>: Sets the incomming buffer size (default 4096)
-nw: Forbides spaces in the nicknames (default are enabled)
-bind <address>: Sets the address to bind (default 0 = all available addresses on all interfaces)
-ctt: Cleans telnet protocol characters <IAC>,<TOF>,<OPT> (default dissabled)
-e: Allow empty cmds from client (default dissabled)
""" %(sys.argv[0])


def parse_args():
    """
    Parses some command line arguments
    """
    config={"daemon" : False, "port" : 8642, "backlog" : 10, "buf" : 4096,
            "nw" : True, "bind" : "", "egg" : 0xf, "trash_telnet_garbage" : False,
            "allow_empty_cmds" : False}
    import sys
    n=len(sys.argv)
    i=1
    while i<n:
        arg=sys.argv[i]
        if arg=="-h" or arg=="--help":
            usage()
            sys.exit(0)
        elif arg=="-D":
            config["daemon"]=True
        elif arg=="-p" and i+1<n:
            i+=1
            config["port"]=int(sys.argv[i])
        elif arg=="-b" and i+1<n:
            i+=1
            config["backlog"]=int(sys.argv[i])
        elif arg=="-buf" and i+1<n:
            i+=1
            config["buf"]=int(sys.argv[i])
        elif arg=="-nw":
            config["nw"]=not config["nw"]
        elif arg=="-bind" and i+1<n:
            i+=1
            config["bind"]=sys.argv[i]
        elif arg=="+egg" and i+1<n:
            i+=1
            config["egg"]=sys.argv[i]
        elif arg=="-ctt":
            config["trash_telnet_garbage"]=not config["trash_telnet_garbage"]
        elif arg=="-e":
            config["allow_empty_cmds"]=not config["allow_empty_cmds"]
        else:
            print "\033[1;31mWarning:\033[0m Ignoring unkown command line param %s" % (arg,)
            #usage()
            #sys.exit(0)
        i+=1
    return config


def main():
    """
    Main entry point
    """
    try:
        try:
            config=parse_args()
        except ValueError:
            usage()
            import sys
            sys.exit(0)
        if config["daemon"]:
            from daemon import daemon
            daemon()
        srv=server(config["bind"],config["port"],config["backlog"],config["buf"],config["nw"],
                   config["trash_telnet_garbage"],config["allow_empty_cmds"],config["egg"])
        print """
\033[0;36m/***************************************************************\\
|                IgAlJo python server starting...               |
\\***************************************************************/\033[0m
        """
        print "Presh Ctrl+C to stop execution, or send a SIGUSR1 signal to have some fun"
        srv.run()
    except SystemExit:
        pass
    except: # Esto es el control de errores, que más quieres!??
        import traceback,sys
        print "Cannot start the server due to an error: Check the traceback!!"
        trace=file("traceback.log","w")
        traceback.print_exc(file=trace)
        trace.close()
        traceback.print_exc(file=sys.stderr)


if __name__ == "__main__":
    main()
