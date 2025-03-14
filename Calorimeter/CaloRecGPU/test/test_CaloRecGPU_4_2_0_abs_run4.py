#!/usr/bin/env python
# art-description: GPU Topological (Topo-Automaton) Clustering test: 4 2 0 thresholds (in absolute value) with Run 4 MC.
# art-type: grid
# art-include: main/Athena
# art-architecture: '#&nvidia'
# art-output: expert-monitoring.root

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import CaloRecGPUTestingConfig
from CaloRecGPUTestingChecker import check
import sys

def do_test(files):
    flags, testopts = CaloRecGPUTestingConfig.PrepareTest(default_argument_for_files = files, parse_command_arguments = False)
    
    flags.CaloRecGPU.ActiveConfig.SeedThreshold = 4.0
    flags.CaloRecGPU.ActiveConfig.GrowThreshold = 2.0
    flags.CaloRecGPU.ActiveConfig.TermThreshold = 0.0
        
    flags.CaloRecGPU.ActiveConfig.UseAbsSeedThreshold = True
    flags.CaloRecGPU.ActiveConfig.UseAbsGrowThreshold = True
    flags.CaloRecGPU.ActiveConfig.UseAbsTermThreshold = True
    
    flags.CaloRecGPU.ActiveConfig.SplittingUseNegativeClusters = True
    
    flags.CaloRecGPU.ActiveConfig.UseOriginalCriteria = False
    flags.CaloRecGPU.ActiveConfig.doTwoGaussianNoise = False
    
    flags.lock()
    
    testopts.TestType = CaloRecGPUTestingConfig.TestTypes.GrowSplit
    testopts.NumEvents = 500
    
    PlotterConfig = CaloRecGPUTestingConfig.PlotterConfigurator(["CPU_growing", "GPU_growing", "CPU_splitting", "GPU_splitting"], ["growing", "splitting"])
        
    CaloRecGPUTestingConfig.RunFullTestConfiguration(flags, testopts, plotter_configurator = PlotterConfig)
    
if __name__=="__main__":
    do_test(['ttbar_pu200_Run4'])
    sys.exit(check())

