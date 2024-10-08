# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Just runs the GPU algorithms.

import CaloRecGPUTestingConfig
    
if __name__=="__main__":

    flags, testopts = CaloRecGPUTestingConfig.PrepareTest()
            
    flags.lock()
    
    testopts.TestType = CaloRecGPUTestingConfig.TestTypes.RunAllGPU
    
    CaloRecGPUTestingConfig.RunFullTestConfiguration(flags, testopts)



