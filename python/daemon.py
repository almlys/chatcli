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
Implements the missing and really wanted nice daemon() stuff
"""

class fake(object):
    """
    Fake logging object
    """
    def write(self,s):
        pass
    def flush(self):
        pass


def daemon():
    """
    does the same as daemon() (or tries to do it)
    """
    #this is similar to the daemon() system call
    import os,sys
    try:
        pid = os.fork()
        if pid>0:
            sys.exit(0)
    except OSError:
        print >>sys.stderr, "Cannot enter into Daemon mode"
        sys.exit(1)

    os.chdir("/")
    os.setsid()
    os.umask(0)
    sys.stdout = fake()
    sys.stderr = fake()
#    sys.stdout.close()
#    sys.stderr.close()
#    sys.stdin.close()

    try:
        pid = os.fork()
        if pid>0:
            sys.exit(0)
    except OSError:
        print >>sys.stderr, "Cannot enter into Daemon mode (step2)"
        sys.exit(1)

