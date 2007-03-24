#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# $Id$
#

import socket, select

class selectInterface(object):

    rwait = []

    def register(self,fdesc):
        self.rwait.append(fdesc)

    def unregister(self,fdesc):
        self.rwait.remove(fdesc)

    def wait(self):
        return select.select(self.rwait, [], [])

class ProtocolViolation(Exception): pass
class HelloAlreadySent(ProtocolViolation): pass
class NickAlreadyExists(ProtocolViolation): pass
class BadFormatedMsg(ProtocolViolation): pass
class NoHelloWasSent(ProtocolViolation): pass
class NickNotFound(ProtocolViolation): pass

class clientSession(object):

    def __init__(self,conn,addr):
        print "Connection from %s:%s" % (addr)
        self.addr = addr
        self.conn = conn
        self.hellostatus = False
        self.name = None

    def fileno(self):
        return self.conn.fileno()

    def send(self,msg):
        self.conn.sendall(msg)

    def senderror(self,msg=""):
        self.conn.sendall("200 %s\n" %msg)

    def sendOk(self,msg=""):
        self.conn.sendall("100 %s\n" %msg)

    def rcvHello(self):
        if self.hellostatus:
            raise HelloAlreadySent
        else:
            self.hellostatus=True

    def setName(self,name):
        self.name = name

    def getName(self):
        return self.name

    def helloCheck(self):
        if not self.hellostatus:
            raise NoHelloWasSent

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
            del self.nicks[client.getName()]
        del self.clients[client.addr]

    def register(self,client,name):
        client.helloCheck()
        if self.nicks.has_key(name):
            raise NickAlreadyExists
        if client.getName()!=None and self.nicks.has_key(client.getName()):
            del self.nicks[client.getName()]
            self.parent.broadcast("%s is now know as %s\n" %(client.getName(),name),client)
        else:
            self.parent.broadcast("%s has joined the chat\n" %(name,),client)
        self.nicks[name] = client
        client.setName(name)

    def findAddress(self,name):
        if self.nicks.has_key(name):
            return self.nicks[name].addr[0]
        else:
            raise NickNotFound


class server(object):

    keep_running = True
    bufferSize = 4096

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
            cli = self.clients.clients[key]
            if cli!=client:
                cli.send(msg)

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

            if cmd=="HELO":
                client.rcvHello()
                client.sendOk("OK %s:%s" % (client.addr))
            elif cmd=="REGISTER":
                nick = data.split()
                if len(nick)==0:
                    raise BadFormatedMsg
                self.clients.register(client,nick[0])
            elif cmd=="QUERY":
                nick = data.split()
                if len(nick)==0:
                    raise BadFormatedMsg
                client.helloCheck()
                client.sendOk("%s" %(self.clients.findAddress(nick[0])))
            elif cmd=="BCAST":
                client.helloCheck()
                if client.name==None:
                    author="Anonymous"
                else:
                    author=client.name
                self.broadcast(author + " says: " + data + "\n",client)
                client.sendOk("OK")
            elif cmd=="EXIT":
                self.clients.remove(client)
            else:
                client.senderror("Unknown cmd:%s" %(cmd,))
        
        except HelloAlreadySent:
            client.senderror("HelloAlreadySent")
        except NickAlreadyExists:
            client.senderror("NickAlreadyExists")
        except BadFormatedMsg:
            client.senderror("BadFormatedMsg")
        except NoHelloWasSent:
            client.senderror("NoHelloWasSent")
        except NickNotFound:
            client.senderror("NickNotFound")

    def run(self):
        self.startOp()
        while(self.keep_running):
            self.requestLoop()
        self.stopOp()



def main():
    srv=server()
    srv.run()

if __name__ == "__main__":
    main()
