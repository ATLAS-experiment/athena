#!/usr/bin/env python
# art-description: GPU Topological (Topo-Automaton) Clustering test: 4 2 2 thresholds.
# art-type: grid
# art-include: main/Athena
# art-architecture: '#&nvidia'
# art-output: expert-monitoring.root

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import CaloRecGPUTestingConfig
from CaloRecGPUTestingChecker import check
import sys

def do_test(files):
    #files does nothing for now, to be improved in the future to maybe allow multiple tests?
    
    
    flags, testopts = CaloRecGPUTestingConfig.PrepareTest()
    
    flags.CaloRecGPU.ActiveConfig.SeedThreshold = 4.0
    flags.CaloRecGPU.ActiveConfig.GrowThreshold = 2.0
    flags.CaloRecGPU.ActiveConfig.TermThreshold = 2.0
        
    flags.CaloRecGPU.ActiveConfig.UseAbsSeedThreshold = False
    flags.CaloRecGPU.ActiveConfig.UseAbsGrowThreshold = False
    flags.CaloRecGPU.ActiveConfig.UseAbsTermThreshold = False
    
    flags.CaloRecGPU.ActiveConfig.SplittingUseNegativeClusters = False
    
    flags.CaloRecGPU.ActiveConfig.UseOriginalCriteria = False
    flags.CaloRecGPU.ActiveConfig.doTwoGaussianNoise = False
    
    flags.lock()
    
    testopts.TestGrow = True
    testopts.TestSplit = True
    testopts.NumEvents = 100
    
    PlotterConfig = CaloRecGPUTestingConfig.PlotterConfigurator(["CPU_growing", "GPU_growing", "CPU_splitting", "GPU_splitting"], ["growing", "splitting"])
        
    CaloRecGPUTestingConfig.RunFullTestConfiguration(flags, testopts, PlotterConfigurator = PlotterConfig)
    
if __name__=="__main__":
    do_test(["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000001.pool.root.1",
             "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000002.pool.root.1",
             "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000003.pool.root.1" ])
    sys.exit(check())

