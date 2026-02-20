# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Measures per-thread times for CPU and GPU implementations,
#without synchronization on the GPU side
#(thus only the total GPU time is accurate).

import CaloRecGPUTestingConfig
    
if __name__=="__main__":

    flags, testopts = CaloRecGPUTestingConfig.PrepareTest()
        
    flags.CaloRecGPU.ActiveConfig.MeasureTimes = True
    
    flags.lock()
    
    testopts.TestType = CaloRecGPUTestingConfig.TestTypes.GrowSplitMoments
    
    CaloRecGPUTestingConfig.RunFullTestConfiguration(flags, testopts)

    
