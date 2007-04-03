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

Eina per a fer proves
"""

import select
import popen2
import fcntl
from time import sleep


def usage():
    """
    Prints some usage useful, nice help
    """
    import sys
    print """%s server:port [-b ../client] [-buf 4096]
-b <binary>: Sets the binary to test
-buf <size>: Sets the incomming buffer size (default 4096)
""" %(sys.argv[0])


def parse_args():
    """
    Parses some command line arguments
    """
    config={ "client_bin" : "../client", "server_port" : 8642, "server_addr" : "" }
    import sys
    n=len(sys.argv)
    i=6
    while i<n:
        arg=sys.argv[i]
        if arg=="-h" or arg=="--help":
            usage()
            sys.exit(0)
        elif arg=="-buf" and i+1<n:
            i+=1
            config["buf"]=int(sys.argv[i])
        elif arg=="-b" and i+1<n:
            i+=1
            config["client_bin"]=int(sys.argv[i])
        else:
            t = sys.argv[i].split(":")
            if len(t)>1:
                server = t[0]
                port = t[1]
            else:
                server = t[0]
                port = 8642
            config["server_port"]=int(port)
            config["server_addr"]=server
            config["login"]=login
        i+=1
    return config


class test_suite(object):

    def __init__(self,config):
	import sys
        self.config=config
        # Now i can cheat :)
        self.select=select.poll()
        self.keep_running=int(sys.argv[4])

    def do_test(self):
	import sys
	usr2=sys.argv[1]+"\n"
    	ip2=sys.argv[2]+"\n"
    	port2=sys.argv[3]+"\n"
        baby = popen2.Popen4(self.config["client_bin"])
        self.select.register(baby.fromchild,select.POLLIN)
        args = fcntl.fcntl(baby.fromchild,fcntl.F_GETFL)
        #define O_NONBLOCK        04000
        args |= 04000
        print args
        fcntl.fcntl(baby.fromchild,fcntl.F_SETFL,args)

        # Aixó es pura merda en estat pur
        sleep(1)
        
        imsg = baby.fromchild.read()
        print "client: %s" % (imsg,)
        baby.tochild.write(usr2)
        baby.tochild.flush()
        sleep(1)

        imsg = baby.fromchild.read()
        print "client: %s" % (imsg,)
        baby.tochild.write(ip2)
        baby.tochild.flush()
        sleep(1)


        imsg = baby.fromchild.read()
        print "client: %s" % (imsg,)
        baby.tochild.write(port2)
        baby.tochild.flush()
        sleep(20)

        while self.keep_running>0:
            sleep(0.5)
	    aux=self.keep_running
            imsg = baby.fromchild.read()
            print "client: %s" % (imsg,)
            baby.tochild.write("todos: "+str(aux)+" -- Spam!!\n")
            baby.tochild.flush()

            sleep(0.5)

            imsg = baby.fromchild.read()
            print "client: %s" % (imsg,)
            baby.tochild.write("todos: "+str(aux)+" -- Eggs!!\n")
            baby.tochild.flush()

	    self.keep_running-=1
	
	sleep(20)
	imsg = baby.fromchild.read()
        print "client: %s" % (imsg,)
        baby.tochild.write("salir\n")
        baby.tochild.flush()

##        while self.keep_running:
##            print "hay"
##            for desc in self.select.poll(1000):
##                fd, st = desc
##                print baby.fromchild.read()
##                #baby.tochild.write("hi\n")
##                baby.tochild.flush()



def main():
    """
    Main entry point
    """
    try:
        try:
            config=parse_args()
        except:
            usage()
            import sys
            sys.exit(0)
        t=test_suite(config)
        t.do_test()
    except: # Esto es el control de errores, que más quieres!??
        import traceback,sys
        print "Error: Check the traceback!!"
        trace=file("traceback.log","w")
        traceback.print_exc(file=trace)
        trace.close()
        traceback.print_exc(file=sys.stderr)

if __name__ == "__main__":
    main()
