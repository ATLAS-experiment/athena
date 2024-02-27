# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs plots for comparing
#CPU growing with GPU growing
#and all possible combinations for the splitter:
#CPU growing with CPU splitting,
#GPU growing with GPU splitting,
#CPU growing with GPU splitting
#and GPU growing with CPU splitting.

import CaloRecGPUTestingConfig
from PlotterConfigurator import PlotterConfigurator

    
if __name__=="__main__":

    
    PlotterConfig = PlotterConfigurator(["CPU_growing", "GPU_growing", "CPUCPU_splitting", "GPUGPU_splitting", "CPUGPU_splitting", "GPUCPU_splitting"],
                                                          ["growing", "CPU_to_GPUGPU_splitting", "CPU_to_CPUGPU_splitting", "CPU_to_GPUCPU_splitting"])   

    flags, perfmon, numevents = CaloRecGPUTestingConfig.PrepareTest()
    flags.CaloRecGPU.Default.DoMonitoring = True
    flags.CaloRecGPU.Default.ClustersOutputName="CaloCalTopoClustersNew"
    flags.lock()
    flagsActive = flags.cloneAndReplace("CaloRecGPU.ActiveConfig", "CaloRecGPU.Default")
    
    topoAcc = CaloRecGPUTestingConfig.MinimalSetup(flagsActive,perfmon)

    topoAcc.merge(CaloRecGPUTestingConfig.FullTestConfiguration(flagsActive, TestGrow = True, TestSplit = True, DoCrossTests = True, PlotterConfigurator = PlotterConfig))

    
    topoAcc.run(numevents)

