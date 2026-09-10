# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":


    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultConditionsTags
    
    cfgFlags = initConfigFlags()
    cfgFlags.Concurrency.NumThreads=1
    cfgFlags.Exec.MaxEvents=1
    cfgFlags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
    cfgFlags.Input.isMC=True
    cfgFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PFlowTests/mc21_14TeV/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8481_s4383_r15934/AOD.41490164._005514.pool.root.1"]
    cfgFlags.Output.AODFileName="output_AOD.root"
    cfgFlags.Output.doWriteAOD=True
    cfgFlags.DiTau.doDiTauRec = False #does not run from ESD - tries to use aux variables which do not exist
    cfgFlags.Detector.GeometryCalo = True
    cfgFlags.Acts.TrackingGeometry.UseBlueprint = True
    cfgFlags.Detector.GeometryITk = True
    cfgFlags.Detector.GeometryBpipe = True
    cfgFlags.fillFromArgs()
    cfgFlags.lock()


    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(cfgFlags)


    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(cfgFlags))

    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    from pathlib import Path
    cfg.merge(ActsTrackingGeometrySvcCfg(cfgFlags,
                                    RunConsistencyChecks=True,
                                    #  ConsistencyCheckOutput="trk_geo_check.csv", # enable debug output writing
                                    BlueprintGraphviz=str(Path.cwd() / "blueprint.dot"),
                                    ObjDebugOutput=False))

  
    from ActsConfig.CaloExtensionBuilderConfig import ActsCaloExtensionBuilderCfg
    cfg.merge(ActsCaloExtensionBuilderCfg(cfgFlags))

    cfg.run()