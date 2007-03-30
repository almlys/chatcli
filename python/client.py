#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# $Id$
#

import socket
import netcommon
from netcommon import selectInterface

class client(object):
    keep_running = True
    bufferSize = 4096

    def __init__(self,lhost="",lport=7766):
        self.udpsocket=None
        self.tcpsocket=None
        self.setBindAddress(lhost,lport)

    def setBindAddress(self,lhost,lport):
        """ Sets the server bind address (adress/port) pair """
        self.bindAddr=lhost
        self.bindPort=lport

    def startOp(self):
        """
        Creates the UDP socket
        """
        self.installSignalHandlers()
        self.udpsocket=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # set Reuse Address
        self.udpsocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.udpsocket.bind((self.bindAddr,self.bindPort))
        print "DBG: Listening to incoming datagrams on %s port tcp %s" %(self.udpsocket.getsockname())
        self.initialized=True
        self.select = selectInterface()
        import sys
        self.select.register(sys.stdin)
        self.select.register(self.udpsocket)

    def connect(self,address=("localhost",8642)):
        """
        Creates and Connects the tcp socket to the specified address
        """
        self.tcpsocket=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.tcpsocket.connect(address)
        self.select.register(self.tcpsocket)
        self.tcpsocket.sendall("HOLA\n")

    def setNick(self,nick):
        self.tcpsocket.sendall("300 %s\n" %nick)
        
    def stopOp(self):
        self.udpsocket.close()
        del self.udpsocket
        self.tcpsocket.close()
        del self.tcpsocket

    def requestLoop(self):
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
                data = desc.recv(self.bufferSize)
                print data

    def run(self):
        self.startOp()
        self.connect()
        while(self.keep_running):
            self.requestLoop()
        self.stopOp()

    def usershutdown(self):
        self.tcpsocket.sendall("800\n")

    def processUserInput(self,msg):
        try:
            cmd, data = msg.split(':',1)
        except ValueError:
            cmd = msg
            data = ""
        if cmd=="exit":
            self.usershutdown()
        elif cmd=="todos" and len(data)!=0:
            self.tcpsocket.sendall("700 " + data + "\n")
        if len(data)==0:
            if cmd=="help":
                print """
**** Client Help system ****
===============================================================================
   command                               action
 help            Shows this help
 all: msg        Sends a message to everybody connected in the network
 <nick>: msg     Sends a private message to nick
 exit            Terminates the program
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
        else:
            print data
    
    def signalHandler(self,num,frame):
        import signal
        if self.keep_running:
            self.keep_running=False
            signal.signal(num,self.signalHandler)
        else:
            import sys
            print "Killed!"
            sys.exit()

    def installSignalHandlers(self):
        import signal
        signal.signal(signal.SIGTERM, self.signalHandler)
        signal.signal(signal.SIGINT, self.signalHandler)

def main():
    cli=client()
    cli.run()

if __name__ == "__main__":
    main()
