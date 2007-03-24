#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# $Id$
#

import select

class selectInterface(object):

    rwait = []

    def register(self,fdesc):
        self.rwait.append(fdesc)

    def unregister(self,fdesc):
        self.rwait.remove(fdesc)

    def wait(self):
        try:
            return select.select(self.rwait, [], [])
        except select.error,e:
            if e[0]==4:
                # A signal was caugh by select, safely ignore it
                return [],[],[]
            else:
                # propagate the error
                raise e
                
