#!/usr/bin/env python
# art-description: GPU Topological (Topo-Automaton) Clustering test: 6 3 3 thresholds.
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
    
    flags.CaloRecGPU.ActiveConfig.SeedThreshold = 6.0
    flags.CaloRecGPU.ActiveConfig.GrowThreshold = 3.0
    flags.CaloRecGPU.ActiveConfig.TermThreshold = 3.0
        
    flags.CaloRecGPU.ActiveConfig.UseAbsSeedThreshold = False
    flags.CaloRecGPU.ActiveConfig.UseAbsGrowThreshold = False
    flags.CaloRecGPU.ActiveConfig.UseAbsTermThreshold = False
    
    flags.CaloRecGPU.ActiveConfig.SplittingUseNegativeClusters = False
    
    flags.CaloRecGPU.ActiveConfig.UseOriginalCriteria = False
    flags.CaloRecGPU.ActiveConfig.doTwoGaussianNoise = False
    
    flags.lock()
    
    testopts.TestType = CaloRecGPUTestingConfig.TestTypes.GrowSplit
    testopts.NumEvents = 500
    
    PlotterConfig = CaloRecGPUTestingConfig.PlotterConfigurator(["CPU_growing", "GPU_growing", "CPU_splitting", "GPU_splitting"], ["growing", "splitting"])
        
    CaloRecGPUTestingConfig.RunFullTestConfiguration(flags, testopts, plotter_configurator = PlotterConfig)
    
if __name__=="__main__":
    do_test(['ttbar'])
    sys.exit(check())

