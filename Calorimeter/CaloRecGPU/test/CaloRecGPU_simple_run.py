# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Just runs the GPU algorithms.

import CaloRecGPUTestingConfig
    
if __name__=="__main__":

    flags, testopts = CaloRecGPUTestingConfig.PrepareTest()
            
    flags.lock()
    
    CaloRecGPUTestingConfig.RunFullTestConfiguration(flags, testopts)



