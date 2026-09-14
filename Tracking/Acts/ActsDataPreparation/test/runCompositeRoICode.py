#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TestCompositeRoIToolCfg(flags,
                            name: str = 'TestCompositeRoITool',
                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('EtaCenters', [-1.2, 2.3, 3.8])
    kwargs.setdefault('PhiCenters', [0, 1, 2])
    kwargs.setdefault('HalfEtaWidths', [0.3, 0.1, 0.05])
    kwargs.setdefault('HalfPhiWidths', [0.4, 0.02, 0.2])

    kwargs.setdefault('ZCenters', [0, 0, 0])
    kwargs.setdefault('HalfZWidths', [250, 250, 250])

    kwargs.setdefault('OutputLevel', 2)

    acc.setPrivateTools(CompFactory.ActsTrk.TestRoICreatorTool(name, **kwargs))
    return acc
    
if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.DQ.useTrigger = False
    flags.Output.HISTFileName = "ActsMonitoringOutput.root"
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultConditionsTags
    flags.Input.Files = defaultTestFiles.RDO_RUN4
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
    flags.Exec.MaxEvents = 1

    # Set the Main Pass
    flags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        "Tracking.ITkActsPass")

    flags.fillFromArgs()
    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # RoI creator
    from ActsConfig.ActsRegionsOfInterestConfig import ActsMainRegionsOfInterestCreatorAlgCfg
    acc.merge(ActsMainRegionsOfInterestCreatorAlgCfg(flags,
                                                     RoIs='TestCompositeRoI',
                                                     RoICreatorTool=acc.popToolsAndMerge(TestCompositeRoIToolCfg(flags))))

    # Data Preparation - Clustering
    from ActsConfig.ActsClusterizationConfig import ActsPixelClusterizationAlgCfg
    acc.merge(ActsPixelClusterizationAlgCfg(flags,
                                            RoIs='TestCompositeRoI'))
    from ActsConfig.ActsClusterizationConfig import ActsStripClusterizationAlgCfg
    acc.merge(ActsStripClusterizationAlgCfg(flags,
                                            RoIs='TestCompositeRoI'))
    
    from ActsConfig.ActsAnalysisConfig import ActsPixelClusterAnalysisAlgCfg, ActsStripClusterAnalysisAlgCfg
    acc.merge(ActsPixelClusterAnalysisAlgCfg(flags))
    acc.merge(ActsStripClusterAnalysisAlgCfg(flags))

    acc.printConfig(withDetails = True, summariseProps = True)
    acc.run()
