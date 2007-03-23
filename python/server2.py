import sd7net

srv=sd7net.Unet5("",7000)
srv.startOp()
i=0
while 1:
    srv.recv()
    i=i+1
    print "Idle... %i" %(i)

