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


import socket
import netcommon
from netcommon import selectInterface

class protocol(object):
    sep="\r\n"
    # "\n" *nix systems
    # "\r" Old Mac systems
    # "\r\n" Hasecorp Hasefroch systems
    ok="100"
    error="200"
    helo="HOLA"
    register="300"
    query="400"
    answer="500"
    pm="600"
    bcast="700"
    exit="800"

class ClientStatus(object):
    new=0
    ident=1
    register=2

class ProtocolViolation(Exception): pass
class HelloAlreadySent(ProtocolViolation): pass
class NickAlreadyExists(ProtocolViolation): pass
class BadFormatedMsg(ProtocolViolation): pass
class NoHelloWasSent(ProtocolViolation): pass
class NoRegisterSent(ProtocolViolation): pass
class NickNotFound(ProtocolViolation): pass

class clientSession(object):

    def __init__(self,conn,addr):
        print "Connection from %s:%s" % (addr)
        self.addr = addr
        self.conn = conn
        self.status = ClientStatus.new
        self.name = None

    def fileno(self):
        return self.conn.fileno()

    def send(self,msg):
        self.conn.sendall(msg)

    def senderror(self,msg=""):
        #self.conn.sendall("200 %s\n" %msg)
        self.conn.sendall(protocol.error + protocol.sep)

    def sendOk(self,msg=""):
        #self.conn.sendall("100 %s\n" %msg)
        self.conn.sendall(protocol.ok + protocol.sep)

    def sendanswer(self,msg=""):
        #self.conn.sendall("100 %s\n" %msg)
        self.conn.sendall(protocol.answer + " " + msg + protocol.sep)

    def rcvHello(self):
        if self.status!=ClientStatus.new:
            raise HelloAlreadySent
        else:
            self.status=ClientStatus.ident

    def setName(self,name):
        self.name = name
        self.status=ClientStatus.register

    def getName(self):
        return self.name

    def helloCheck(self):
        if self.status==ClientStatus.new:
            raise NoHelloWasSent

    def registerCheck(self):
        if self.status!=ClientStatus.register:
            raise NoRegisterSent,"No register was sent"

    def isRegistered(self):
        return self.status==ClientStatus.register

class sessionMGR(object):

    clients = {}
    nicks = {}

    def __init__(self,parent):
        self.parent = parent

    def add(self,conn,addr):
        cli = clientSession(conn,addr)
        if self.clients.has_key(addr):
            self.parent.select.unregister(self.clients[addr])
            name=self.clients[addr].getName()
            if name!=None and self.nicks.has_key(name):
                del self.nicks[name]
            del self.clients[addr]
        self.clients[addr] = cli
        self.parent.select.register(cli)
        return cli

    def remove(self,client):
        print "Connection %s:%s was closed" %(client.addr)
        client.conn.close()
        self.parent.select.unregister(client)
        if client.getName()!=None:
            self.parent.broadcast("%s has left the chat" %(client.getName(),),client)
            del self.nicks[client.getName()]
        del self.clients[client.addr]

    def register(self,client,name):
        client.helloCheck()
        if self.nicks.has_key(name):
            raise NickAlreadyExists
        if client.getName()!=None and self.nicks.has_key(client.getName()):
            del self.nicks[client.getName()]
            self.parent.broadcast("%s is now know as %s" %(client.getName(),name),client)
        else:
            self.parent.broadcast("%s has joined the chat" %(name,),client)
        self.nicks[name] = client
        client.setName(name)

    def findAddress(self,name):
        if self.nicks.has_key(name):
            return self.nicks[name].addr[0]
        else:
            #raise NickNotFound
            return "null"


class server(object):

    keep_running = True
    bufferSize = 4096
    shutdowncount = 15
    shutting_down = False
    allowwhitespaces = True

    def __init__(self,lhost="",lport=8642,backlog=10):
        self.socket=None
        self.initialized=False
        self.setBindAddress(lhost,lport)
        self.backlog=backlog
        self.clients = sessionMGR(self)

    def setBindAddress(self,lhost,lport):
        """ Sets the server bind address (adress/port) pair """
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
        self.socket.close()
        del self.socket
        self.initialized=False

    def broadcast(self,msg,client=None):
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
        read, write, excp = self.select.wait()
            
        for client in read:
            if client==self.socket:
                # accept (A new connection was accepted)
                self.clients.add(*client.accept())
            else:
                # Data was recieved from a client
                data = client.conn.recv(self.bufferSize)
                if len(data)==0:
                    self.clients.remove(client)
                    continue
                self.processRequest(client,data)

    def processRequest(self,client,data):
        print "From %s:%s" %(client.addr)
        #print data,

        msg = data.strip() #Esto se carga el \n y el famoso \r que tanto le preocupa al profe
        try:
            cmd, data = msg.split(' ',1)
        except ValueError:
            cmd = msg
            data = ""
        try:
            print "processing cmd: %s, data: %s" %(cmd,data)

            if cmd==protocol.helo:
                client.rcvHello()
                client.sendOk("OK %s:%s" % (client.addr))
            elif cmd==protocol.register:
                if self.allowwhitespaces:
                    nick = data
                else:
                    nick, = data.split()
                if len(nick)==0:
                    raise BadFormatedMsg
                self.clients.register(client,nick)
                client.sendOk()
            elif cmd==protocol.query:
                if self.allowwhitespaces:
                    nick = data
                else:
                    nick, = data.split()
                if len(nick)==0:
                    raise BadFormatedMsg
                client.helloCheck()
                client.sendanswer("%s" %(self.clients.findAddress(nick)))
            elif cmd==protocol.bcast:
                client.registerCheck()
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
        except socket.error:
            #Ignorar exceptiones producidas al procesar una petición
            print "Excepcion procesando petición..."
            import traceback,sys
            traceback.print_exc(file=sys.stderr)            

    def run(self):
        self.startOp()
        while(self.keep_running):
            self.requestLoop()
        self.stopOp()

    def signalHandler(self,num,frame):
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
        import signal
        signal.signal(signal.SIGTERM, self.signalHandler)
        signal.signal(signal.SIGINT, self.signalHandler)
        signal.signal(signal.SIGUSR1, self.signalHandler)
        signal.signal(signal.SIGALRM, self.signalHandler)


def main():
    try:
        srv=server()
        srv.run()
    except: # Esto es el control de errores, que más quieres!??
        import traceback,sys
        trace=file("traceback.log","w")
        traceback.print_exc(file=trace)
        trace.close()
        traceback.print_exc(file=sys.stderr)


if __name__ == "__main__":
    main()
