# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TestCompositeRoIToolCfg(flags,
                            name: str = 'TestCompositeRoITool',
                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('RoIs', 'TestCompositeRoI')
    kwargs.setdefault('EtaCenters', [-1.2, 2.3, 3.8])
    kwargs.setdefault('PhiCenters', [0, 1, 2])
    kwargs.setdefault('HalfEtaWidths', [0.3, 0.1, 0.05])
    kwargs.setdefault('HalfPhiWidths', [0.4, 0.02, 0.2])

    kwargs.setdefault('ZCenters', [0, 0, 0])
    kwargs.setdefault('HalfZWidths', [250, 250, 250])

    kwargs.setdefault('OutputLevel', 2)

    acc.setPrivateTools(CompFactory.TestRoICreatorTool(name, **kwargs))
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
    import glob
    flags.Input.Files = glob.glob('/afs/cern.ch/user/c/cvarni/work/ACTS/TimingPlots/data/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/*')
    flags.Exec.MaxEvents = 1

    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # RoI creator
    from ActsConfig.ActsViewConfig import EventViewCreatorAlgCfg
    acc.merge(EventViewCreatorAlgCfg(flags,
                                     RoICreatorTool=acc.popToolsAndMerge(TestCompositeRoIToolCfg(flags))))

    # Data Preparation - Clustering
    from ActsConfig.ActsClusterizationConfig import ActsITkPixelClusterizationAlgCfg
    acc.merge(ActsITkPixelClusterizationAlgCfg(flags,
                                               RoIs='TestCompositeRoI'))
    from ActsConfig.ActsClusterizationConfig import ActsITkStripClusterizationAlgCfg
    acc.merge(ActsITkStripClusterizationAlgCfg(flags,
                                               RoIs='TestCompositeRoI'))

    from ActsConfig.ActsAnalysisConfig import ActsPixelClusterAnalysisAlgCfg, ActsStripClusterAnalysisAlgCfg
    acc.merge(ActsPixelClusterAnalysisAlgCfg(flags))
    acc.merge(ActsStripClusterAnalysisAlgCfg(flags))

    acc.printConfig(withDetails = True, summariseProps = True)
    acc.run()
