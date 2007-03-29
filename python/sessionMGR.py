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
A session Manager (what were you expecting?)
An related client session objects
"""

__all__ = ["clientSession","sessionMGR",]

from cprotocol import *

class clientSession(object):
    """
    A client Session
    Holds client related data to its session
    """

    def __init__(self,conn,addr):
        """
        Session constructor
        @param conn Socket object associated to the new session
        @param addr Ip/port tuple with the source client address
        """
        print "Connection from %s:%s" % (addr)
        self.addr = addr
        self.conn = conn
        self.status = ClientStatus.new
        self.name = None

    def fileno(self):
        """
        Returns the file descriptor, so it can be watched by select
        """
        return self.conn.fileno()

    def send(self,msg):
        """
        Sends a message
        @param msg The message that you want to send, what else?
        """
        self.conn.sendall(msg)

    def senderror(self,msg=""):
        """
        Sends an error message
        @param msg Depreceated and Unused in protocol 2.0
        """
        #self.conn.sendall("200 %s\n" %msg)
        self.conn.sendall(protocol.error + protocol.sep)

    def sendOk(self,msg=""):
        """
        Sends an ok message
        @param msg Depreceated and Unused in protocol 2.0
        """
        #self.conn.sendall("100 %s\n" %msg)
        self.conn.sendall(protocol.ok + protocol.sep)

    def sendanswer(self,msg=""):
        """
        Sends an answer message (reply to a query msg)
        @param msg IP Address of the system if found, elsewhere the string null
        """
        #self.conn.sendall("100 %s\n" %msg)
        self.conn.sendall(protocol.answer + " " + msg + protocol.sep)

    def rcvHello(self):
        """
        Notifies this client, that hello was send
        @throws HelloAlreadySent if hello was already sent
        """
        if self.status!=ClientStatus.new:
            raise HelloAlreadySent,"Hello was already sent"
        else:
            self.status=ClientStatus.ident

    def setName(self,name):
        """
        Sets the name
        @param name The name of the luser connecting to our server
        """
        self.name = name
        self.status=ClientStatus.register

    def getName(self):
        """
        Gets the name
        @return the nickname
        """
        return self.name

    def helloCheck(self):
        """
        Checks if hello was sent
        @throws NoHelloWasSent, if it was not sent
        """
        if self.status==ClientStatus.new:
            raise NoHelloWasSent,"No hello was sent"

    def registerCheck(self):
        """
        Checks if the client is registered
        @throws NoRegisterSent, if it was not registered
        """
        if self.status!=ClientStatus.register:
            raise NoRegisterSent,"No register was sent"

    def isRegistered(self):
        """
        Returns true if the client is correctly registered
        @returns True if the client is registered
        """
        return self.status==ClientStatus.register


class sessionMGR(object):
    """
    Session Manager class, to manage and dismanage the client sessions
    """

    clients = {}
    nicks = {}

    def __init__(self,parent):
        """
        Constructor
        @param parent The server object, with the select interface, and other required methods
        """
        self.parent = parent

    def add(self,conn,addr):
        """
        Adds a new client to our client list
        @param conn The socket object
        @param addr Ip/port tuple of the source client
        """
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
        """
        Removes and kills an already logged client
        @param client The client object to kill
        """
        print "Connection %s:%s was closed" %(client.addr)
        client.conn.close()
        self.parent.select.unregister(client)
        if client.getName()!=None:
            self.parent.broadcast("%s has left the chat" %(client.getName(),),client)
            del self.nicks[client.getName()]
        del self.clients[client.addr]

    def register(self,client,name):
        """
        Registers a nick to our YellowPages database
        @param client The client object
        @param name The name to register
        @throws NickAlreadyExists if the nick already exists
        """
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
        """
        Returns an ip address from the requested nick
        @note protocol v2.0 must return the string "null" instead of an error message
        @param name NickName of the client to search
        @return IP address of the client
        """
        if self.nicks.has_key(name):
            return self.nicks[name].addr[0]
        else:
            #raise NickNotFound
            return "null"

