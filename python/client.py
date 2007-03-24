import sd7net

srv=sd7net.Unet5()
srv.startOp()
i=0
while 1:
    srv.recv()
    i=i+1
    srv.send("172.26.1.17",7000,"Message %i" %(i))
    print "Idle... %i" %(i)
