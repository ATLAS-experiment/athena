# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs plots for comparing
#CPU growing with GPU growing
#and all possible combinations for the splitter:
#CPU growing with CPU splitting,
#GPU growing with GPU splitting,
#CPU growing with GPU splitting
#and GPU growing with CPU splitting.

import CaloRecGPUTestingConfig

    
if __name__=="__main__":

    flags, testopts = CaloRecGPUTestingConfig.PrepareTest()
        
    flags.lock()
    
    testopts.TestGrow = True
    testopts.TestSplit = True
    testopts.DoCrossTests = True
    
    PlotterConfig = CaloRecGPUTestingConfig.PlotterConfigurator(["CPU_growing", "GPU_growing", "CPUCPU_splitting", "GPUGPU_splitting", "CPUGPU_splitting", "GPUCPU_splitting"],
                                                          ["growing", "CPU_to_GPUGPU_splitting", "CPU_to_CPUGPU_splitting", "CPU_to_GPUCPU_splitting"])   
    
    CaloRecGPUTestingConfig.RunFullTestConfiguration(flags, testopts, PlotterConfigurator = PlotterConfig)
    