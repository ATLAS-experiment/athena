# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs plots and textual information
#to compare CPU with GPU moments calculation.

import CaloRecGPUTestingConfig
from PlotterConfigurator import PlotterConfigurator
    
if __name__=="__main__":

    PlotterConfig = PlotterConfigurator(["CPU_moments", "GPU_moments"], ["moments"], DoMoments = True)
    
    flags, perfmon, numevents = CaloRecGPUTestingConfig.PrepareTest()
    flags.CaloRecGPU.Default.UseAbsEnergyMoments = True
    flags.CaloRecGPU.Default.ClustersOutputName="CaloCalTopoClustersNew"
    flags.lock()
    flagsActive = flags.cloneAndReplace("CaloRecGPU.ActiveConfig", "CaloRecGPU.Default")

    topoAcc = CaloRecGPUTestingConfig.MinimalSetup(flagsActive,perfmon)

    topoAcc.merge(CaloRecGPUTestingConfig.FullTestConfiguration(flagsActive, TestMoments = True, PlotterConfigurator = PlotterConfig))

    topoAcc.run(numevents)
