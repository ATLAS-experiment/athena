# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs plots for comparing CPU and GPU splitting.

import CaloRecGPUTestingConfig
from PlotterConfigurator import PlotterConfigurator
    
if __name__=="__main__":

    PlotterConfig = PlotterConfigurator(["CPU_splitting", "GPU_splitting"], ["splitting"])

    flags, perfmon, numevents = CaloRecGPUTestingConfig.PrepareTest()
    flags.CaloRecGPU.Default.DoMonitoring = True

    flags.CaloRecGPU.Default.ClustersOutputName="CaloCalTopoClustersNew"
    flags.lock()
    flagsActive = flags.cloneAndReplace("CaloRecGPU.ActiveConfig", "CaloRecGPU.Default")

    topoAcc = CaloRecGPUTestingConfig.MinimalSetup(flagsActive,perfmon)

    topoAcc.merge(CaloRecGPUTestingConfig.FullTestConfiguration(flagsActive, TestSplit = True, PlotterConfigurator = PlotterConfig))

    topoAcc.run(numevents)
