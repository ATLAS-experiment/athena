#!/usr/bin/env python
# art-description: GPU Topological (Topo-Automaton) Clustering test: 4 2 0 thresholds.
# art-type: grid
# art-include: main/Athena
# art-architecture: '#&nvidia'
# art-output: expert-monitoring.root

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import CaloRecGPUTestingConfig
from PlotterConfigurator import PlotterConfigurator
from CaloRecGPUTestingChecker import check
import sys

def do_test(files):
    
    PlotterConfig = PlotterConfigurator(["CPU_growing", "GPU_growing", "CPU_splitting", "GPU_splitting"], ["growing", "splitting"])
    

    flags, perfmon, numevents = CaloRecGPUTestingConfig.PrepareTest()
    flags.CaloRecGPU.DoMonitoring = True
    flags.CaloRecGPU.TopoClusterSeedCutsInAbsE = False
    flags.CaloRecGPU.TopoClusterCellCutsInAbsE = False
    flags.CaloRecGPU.TopoClusterNeighborCutsInAbsE = False
    flags.CaloRecGPU.SplitterUseNegativeClusters = False

    flags.CaloRecGPU.TwoGaussianNoise = False

    flags.CaloRecGPU.TopoClusterSNRSeedThreshold = 4.0
    flags.CaloRecGPU.TopoClusterSNRGrowThreshold = 2.0
    flags.CaloRecGPU.TopoClusterSNRCellThreshold = 0.0

    flags.CaloRecGPU.ClusterSize = "Topo_420"
    
    flags.CaloRecGPU.ClustersOutputName="CaloCalTopoClustersNew"
    flags.lock()

    topoAcc = CaloRecGPUTestingConfig.MinimalSetup(flags,perfmon)

    topoAcc.merge(CaloRecGPUTestingConfig.FullTestConfiguration(flags, TestGrow = True, TestSplit = True, PlotterConfigurator = PlotterConfig))


    topoAcc.run(100)
    

if __name__=="__main__":
    do_test(["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000001.pool.root.1",
             "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000002.pool.root.1",
             "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000003.pool.root.1" ])
#    do_test(["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigEgammaValidation/valid3.147917.Pythia8_AU2CT10_jetjet_JZ7W.recon.RDO.e3099_s2578_r6596_tid05293007_00/RDO.05293007._000001.pool.root.1"],
#             "jets")
    sys.exit(check())

