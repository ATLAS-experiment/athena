#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TestSingleRoIToolCfg(flags,
                         name: str = 'TestSingleRoITool',
                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('EtaCenters', [2.3])
    kwargs.setdefault('PhiCenters', [0])
    kwargs.setdefault('HalfEtaWidths', [0.05])
    kwargs.setdefault('HalfPhiWidths', [0.1])

    acc.setPrivateTools(CompFactory.ActsTrk.TestRoICreatorTool(name, **kwargs))
    return acc
    
if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.Detector.GeometryITkPixel = True
    flags.Detector.GeometryITkStrip = True
    flags.Detector.EnableITkPixel = True
    flags.Detector.EnableITkStrip = True
    flags.DQ.useTrigger = False
    flags.Output.HISTFileName = "ActsMonitoringOutput.root"
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"]
    flags.Exec.MaxEvents = 1

    # Set the Main Pass
    flags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        "Tracking.ITkMainPass")
    
    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # RoI creator
    from ActsConfig.ActsRegionsOfInterestConfig import ActsMainRegionsOfInterestCreatorAlgCfg
    acc.merge(ActsMainRegionsOfInterestCreatorAlgCfg(flags,
                                                     RoIs='TestSingleRoI',
                                                     RoICreatorTool=acc.popToolsAndMerge(TestSingleRoIToolCfg(flags))))

    # Data Preparation - Clustering
    from ActsConfig.ActsClusterizationConfig import ActsPixelClusterizationAlgCfg
    acc.merge(ActsPixelClusterizationAlgCfg(flags,
                                            RoIs='TestSingleRoI'))
    from ActsConfig.ActsClusterizationConfig import ActsStripClusterizationAlgCfg
    acc.merge(ActsStripClusterizationAlgCfg(flags,
                                            RoIs='TestSingleRoI'))

    from ActsConfig.ActsAnalysisConfig import ActsPixelClusterAnalysisAlgCfg, ActsStripClusterAnalysisAlgCfg
    acc.merge(ActsPixelClusterAnalysisAlgCfg(flags))
    acc.merge(ActsStripClusterAnalysisAlgCfg(flags))

    acc.printConfig(withDetails = True, summariseProps = True)
    acc.run()
