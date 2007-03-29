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
Defines all the pràctica I chat protocol v2.0
"""

__all__ = ["protocol","ClientStatus","ProtocolViolation","HelloAlreadySent"
           ,"NickAlreadyExists","BadFormatedMsg","NoHelloWasSent",
           "NoRegisterSent","NickNotFound",]

class protocol(object):
    """
    Chat Protocol v2.0
    """
    #sep="\r\n"
    # "\n" *nix systems
    # "\r" Old Mac systems
    # "\r\n" Hasecorp Hasefroch systems
    # I personally not see any difference, but let's play a safe game (some
    # people claim that if you put \n it will be translated to \r\n or whatever
    # it's defined by your OS. Since I don't have access to other platforms, I
    # cannot confirm this. But as far as I now, if you write a \n in C using both
    # Linux and Winsucks Winsock api, both are sending a \n, and not a \r\n
    # but yes, it's true that if in winsucks you write a \n in a file that is not
    # open in binary mode, it will convert your \n to \r\n. (very nice), but I have
    # not seen this behaviur on the Winsock API, it there a way to open a socket in
    # a non-binary mode?, if that is posible, then the OS will convert the \n
    # As a final decision, we are using CR LF, see http://www.rfc-editor.org/EOLstory.txt
    sep="\x0D\x0A"
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
    """
    Defines in wich states can be the client
     new 0 -> New connection, not hallowed yet
     ident 1 -> Client has been hallowed (HOLA recieved)
     register 2 -> Client has been hallowed and registered (HOLA and REGISTER
                   succesfully recieved and not duplicated)
    """
    new=0
    ident=1
    register=2


"""
List of Protocol Violation exceptions
"""

class ProtocolViolation(Exception): pass
class HelloAlreadySent(ProtocolViolation): pass
class NickAlreadyExists(ProtocolViolation): pass
class BadFormatedMsg(ProtocolViolation): pass
class NoHelloWasSent(ProtocolViolation): pass
class NoRegisterSent(ProtocolViolation): pass
class NickNotFound(ProtocolViolation): pass
