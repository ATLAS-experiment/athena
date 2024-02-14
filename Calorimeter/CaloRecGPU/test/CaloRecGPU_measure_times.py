# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Measures per-thread times for CPU and GPU implementations,
#without synchronization on the GPU side
#(thus only the total GPU time is accurate).

import CaloRecGPUTestingConfig
    
if __name__=="__main__":

    flags, perfmon, numevents = CaloRecGPUTestingConfig.PrepareTest()
    flags.CaloRecGPU.Default.MeasureTimes = True
    flags.CaloRecGPU.Default.ClustersOutputName="CaloCalTopoClustersNew"
    flags.lock()
    flagsActive = flags.cloneAndReplace("CaloRecGPU.ActiveConfig", "CaloRecGPU.Default")

    topoAcc = CaloRecGPUTestingConfig.MinimalSetup(flagsActive,perfmon)

    topoAcc.merge(CaloRecGPUTestingConfig.FullTestConfiguration(flagsActive,TestGrow=True, TestSplit=True, SkipSyncs=True))

    topoAcc.run(numevents)

    
