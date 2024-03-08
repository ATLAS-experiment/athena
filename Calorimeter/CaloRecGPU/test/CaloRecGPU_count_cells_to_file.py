# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs cell information (and cell assignment, thus also some cluster information)
#to a textual format, for both CPU and GPU growing and splitting,
#and also for the cross-check versions (CPU growing with GPU splitting
#and GPU growing with CPU splitting).

import CaloRecGPUTestingConfig
    
if __name__=="__main__":

    flags, testopts = CaloRecGPUTestingConfig.PrepareTest()
    
    flags.lock()
    
    testopts.TestGrow = True
    testopts.TestSplit = True
    testopts.DoCrossTests = True
    testopts.OutputCounts = True
    
    CaloRecGPUTestingConfig.RunFullTestConfiguration(flags, testopts)

