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
Common stuff for the server/client aplications
"""

__all__ = ["selectInterface",]

import select

"""
Like the poll object, but I think that using poll will be considered
some sort type of cheating, so I'm writting my own poll interface
with select. In fact writting all this stuff in Python is alredy like cheating :)
Well, it allows to register some descriptors, to be monitored for read events.
you can register any object that provides a fileno() method, that will be
watched by select
"""
class selectInterface(object):

    rwait = []

    def register(self,fdesc):
        """
        Register a new descriptor
        @param fdesc Descriptor
        """
        self.rwait.append(fdesc)

        """
        Unregisters a old descriptor
        @param fdesc Descriptor
        """
    def unregister(self,fdesc):
        self.rwait.remove(fdesc)

    def wait(self):
        """
        Waits for data in one of the registered descriptors
        @returns List of descriptors with new available data
        """
        try:
            return select.select(self.rwait, [], [])
        except select.error,e:
            if e[0]==4:
                # A signal was caugh by select, safely ignore it
                # Signals must be ignored, or the Exception will
                # sent our server to oblivion.
                return [],[],[]
            else:
                # we are doomed!, propagate the error
                raise e

