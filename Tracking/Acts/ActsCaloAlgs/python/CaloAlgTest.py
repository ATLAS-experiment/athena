# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":


    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultConditionsTags
    
    flags = initConfigFlags()
    flags.Concurrency.NumThreads=1
    flags.Exec.MaxEvents=1
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
    flags.Input.isMC=True
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PFlowTests/mc21_14TeV/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8481_s4383_r15934/AOD.41490164._005514.pool.root.1"]
    flags.Output.AODFileName="output_AOD.root"
    flags.Output.doWriteAOD=True
    flags.Output.doWriteESD=False
    flags.DiTau.doDiTauRec = False #does not run from ESD - tries to use aux variables which do not exist
    flags.Detector.GeometryCalo = True
    flags.Acts.TrackingGeometry.UseBlueprint = True
    flags.Detector.GeometryITk = True
    flags.Detector.GeometryBpipe = True
    flags.fillFromArgs()
    flags.lock()


    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)


    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))


    from eflowRec.PFRun3Config import PFFullCfg
    cfg.merge(PFFullCfg(flags,runTauReco=True))


    from eflowRec.PFRun3Config import PFTauFELinkCfg
    cfg.merge(PFTauFELinkCfg(flags))


    from eflowRec.PFRun3Remaps import ListRemaps

    from ActsConfig.ActsPersistificationConfig import PersistifyTrackParticles
    cfg.merge(PersistifyTrackParticles(flags, trackParticleCollections=['InDetActsTrackParticles']))

    list_remaps=ListRemaps(cfg, 'AOD')
    for mapping in list_remaps:
        cfg.merge(mapping)    


    from PFlowUtils.configureRecoForPFlow import configureRecoForPFlowCfg
    cfg.merge(configureRecoForPFlowCfg(flags))


    #Add containers needed to run jet finding from resultant AOD for pflow CP studies
    from PFlowUtils.configureRecoForPFlow import addContainersForPFlowCPStudiesCfg
    cfg.merge(addContainersForPFlowCPStudiesCfg(flags))


    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    from pathlib import Path

    cfg.merge(ActsTrackingGeometrySvcCfg(flags,
                                    RunConsistencyChecks=True,
                                    #  ConsistencyCheckOutput="trk_geo_check.csv", # enable debug output writing
                                    BlueprintGraphviz=str(Path.cwd() / "blueprint.dot"),
                                    ObjDebugOutput=False))

    from ActsConfig.CaloExtensionBuilderConfig import ActsCaloExtensionBuilderCfg
    cfg.merge(ActsCaloExtensionBuilderCfg(flags))

    cfg.run()