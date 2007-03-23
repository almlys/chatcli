# 7d7net Prototype
import socket, time

class Unet5:
    def __init__(self,lhost="",lport=0):
        self.socket=None
        self.initialized=False
        self.setBindAddress(lhost,lport)
    def setBindAddress(self,lhost="",lport=0):
        self.bindAddr=lhost
        self.bindPort=lport
    def updateTimer(self,secs):
        if self.initialized:
            self.socket.settimeout(secs)
    def startOp(self):
        if self.initialized:
            return
        self.socket=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # set Non-Blocking, Bcast and other flags
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.setblocking(False)
        self.socket.bind((self.bindAddr,self.bindPort))
        self.socket.settimeout(0.5)
        # 13 May 2006 - Unet5 Prototype
        print "DBG: Listening to incoming datagrams on %s port udp %s" %(self.socket.getsockname())
        self.initialized=True
    def stopOp(self):
        self.socket.close()
        del self.socket
        self.initialized=False
    def recv(self):
        try:
            print self.socket.recvfrom(4096*2)
        except socket.error:
            pass
    def send(self,ip,port,message):
        print "sending %s to %s:%i" %(message,ip,port)
        self.socket.sendto(message,0,(ip,port))
