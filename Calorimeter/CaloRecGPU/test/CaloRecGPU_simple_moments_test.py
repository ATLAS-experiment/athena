# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs plots and textual information
#to compare CPU with GPU moments calculation.

import CaloRecGPUTestingConfig
    
if __name__=="__main__":

    flags, testopts = CaloRecGPUTestingConfig.PrepareTest()
            
    flags.lock()
    
    testopts.TestGrow = True
    testopts.TestSplit = True
    
    PlotterConfig = CaloRecGPUTestingConfig.PlotterConfigurator(["CPU_moments", "GPU_moments"], ["moments"], DoMoments = True)
    
    CaloRecGPUTestingConfig.RunFullTestConfiguration(flags, testopts, PlotterConfigurator = PlotterConfig)
    