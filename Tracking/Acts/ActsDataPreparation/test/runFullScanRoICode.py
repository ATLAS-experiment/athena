# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
    
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
    acc.merge(EventViewCreatorAlgCfg(flags))

    # Data Preparation - Clustering
    from ActsConfig.ActsClusterizationConfig import ActsITkPixelClusterizationAlgCfg
    acc.merge(ActsITkPixelClusterizationAlgCfg(flags))

    from ActsConfig.ActsClusterizationConfig import ActsITkStripClusterizationAlgCfg
    acc.merge(ActsITkStripClusterizationAlgCfg(flags))

    from ActsConfig.ActsAnalysisConfig import ActsPixelClusterAnalysisAlgCfg, ActsStripClusterAnalysisAlgCfg
    acc.merge(ActsPixelClusterAnalysisAlgCfg(flags))
    acc.merge(ActsStripClusterAnalysisAlgCfg(flags))

    acc.printConfig(withDetails = True, summariseProps = True)
    acc.run()
