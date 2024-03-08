# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs plots for comparing
#CPU growing with GPU growing
#and CPU growing + splitting with GPU growing + splitting

import CaloRecGPUTestingConfig
    
if __name__=="__main__":

    flags, testopts = CaloRecGPUTestingConfig.PrepareTest()
    
    flags.lock()
    
    testopts.TestGrow = True
    testopts.TestSplit = True
    
    PlotterConfig = CaloRecGPUTestingConfig.PlotterConfigurator(["CPU_growing", "GPU_growing", "CPU_splitting", "GPU_splitting"], ["growing", "splitting"])  
    
    CaloRecGPUTestingConfig.RunFullTestConfiguration(flags, testopts, PlotterConfigurator = PlotterConfig)


