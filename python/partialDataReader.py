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
Partial Data reader
Para las @#@ lecturas parciales
"""

__all__ = ["PartialDataReader",]

_debug = False

from cprotocol import *

class PartialDataReader(object):
    """
    The partial data reader
    """

    partialData = ""
    sep_state = None

    def __init__(self,emptycmds=False,trash=False):
        """
        Constructor
        @param emptycmds Allow empty commands
        @param trash Trash telnet protocol chars
        """
        self.trash_telnet_garbage = trash
        self.allow_empty_cmds = emptycmds


    def feed(self,data):
        """
        Feeds the reader with more data
        @param data input data
        @returns Tuple of individual requests
        """
        out=[]

        if _debug:
            print "Partial data ->%s<-" %(data,)
        # Las famosas lecturas parciales que dan tantos quebraderos de cabeza
        # Concatenate a previus incompleted message, with the following data
        data = self.partialData + data
        self.partialData = ""
        for req in data.splitlines(True):
            ok_sep=False
            sep_state = None
            for k in protocol.sep:
                if req.endswith(k):
                    ok_sep=True
                    sep_state = k
            if not ok_sep:
                # No se han recibido todos los datos!!!
                self.partialData = req
                if _debug:
                    print "Notice: Not all data was recieved, waiting for protocol separator"
                break
            # Limpiar la basura que envia Telnet?
            if self.trash_telnet_garbage:
                telnet_garbage="\xFF\xFD\x01\x02\x03"
            else:
                telnet_garbage=""
            req = req.strip("\x00 \t" + protocol.sep + telnet_garbage)
            #Esto se carga el \n y el \r
            # El 0x00, es para que nuestro server funcione con algunas implementaciones
            # mal hechas, que envian ristras de zeros despues del mensaje
            # Si activamos esto, el servidor no considerada los comandos vacios como errores
            if len(req)==0 and sep_state!=self.sep_state and self.sep_state!=None: continue
            if _debug:
                print repr(sep_state),repr(self.sep_state)
            self.sep_state = sep_state
            if self.allow_empty_cmds and len(req)==0: continue
            out.append(req)
        return out
