# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Just runs the GPU algorithms.

import CaloRecGPUTestingConfig
    
if __name__=="__main__":

    flags, perfmon, numevents = CaloRecGPUTestingConfig.PrepareTest()

    flags.CaloRecGPU.Default.ClustersOutputName="CaloCalTopoClustersNew"
    flags.lock()
    flagsActive = flags.cloneAndReplace("CaloRecGPU.ActiveConfig", "CaloRecGPU.Default")

    topoAcc = CaloRecGPUTestingConfig.MinimalSetup(flagsActive,perfmon)
    
    topoAcc.merge(CaloRecGPUTestingConfig.FullTestConfiguration(flagsActive))

    topoAcc.run(numevents)

