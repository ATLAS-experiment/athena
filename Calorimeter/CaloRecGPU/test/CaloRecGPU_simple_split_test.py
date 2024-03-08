# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs plots for comparing CPU and GPU splitting.

import CaloRecGPUTestingConfig
    
if __name__=="__main__":

    flags, testopts = CaloRecGPUTestingConfig.PrepareTest()
        
    flags.lock()
    
    testopts.TestGrow = True
    
    PlotterConfig = CaloRecGPUTestingConfig.PlotterConfigurator(["CPU_splitting", "GPU_splitting"], ["splitting"])
    
    CaloRecGPUTestingConfig.RunFullTestConfiguration(flags, testopts, PlotterConfigurator = PlotterConfig)
    
