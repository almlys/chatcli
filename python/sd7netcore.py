import sd7net

class sd7netcore(Unet5):
    def __init__(self):
        sd7net.Unet5.__init__(self)
    def setConfig(self,config):
        self.config=config
    def applySettings(self):
        
