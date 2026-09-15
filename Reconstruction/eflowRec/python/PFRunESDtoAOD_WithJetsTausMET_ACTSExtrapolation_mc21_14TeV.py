# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    cfgFlags = initConfigFlags()
    cfgFlags.Concurrency.NumThreads=1
    cfgFlags.Exec.MaxEvents=100
    cfgFlags.Input.isMC=True
    cfgFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PFlowTests/mc21_14TeV/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8481_s4383_r15934/AOD.41490164._005514.pool.root.1"]
    cfgFlags.Output.AODFileName="output_AOD.root"
    cfgFlags.Output.doWriteAOD=True
    cfgFlags.DiTau.doDiTauRec = False #does not run from ESD - tries to use aux variables which do not exist
    cfgFlags.Detector.GeometryCalo = True
    cfgFlags.Acts.TrackingGeometry.UseBlueprint = True
    cfgFlags.Detector.GeometryITk = True
    cfgFlags.Detector.GeometryBpipe = True
    cfgFlags.PF.useActsExtrapolation=True #Toggle usage of ACTS extrapolation for track propagation to calorimeter
    #Auto configure works by reading the conditions tag  metadata from the input file.
    #Hence for files produced using pre-CREST conditions tags this choice is not
    #compatible with the latest releases which only support CREST conditions tags.
    #So we force it to use the current Run 4 tag
    #See https://its.cern.ch/jira/browse/ATLASRECTS-8434
    from AthenaConfiguration.TestDefaults import defaultConditionsTags
    cfgFlags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
    cfgFlags.fillFromArgs()
    cfgFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(cfgFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(cfgFlags))

    from eflowRec.PFRun3Config import PFFullCfg
    cfg.merge(PFFullCfg(cfgFlags,runTauReco=True))

    from eflowRec.PFRun3Config import PFTauFELinkCfg
    cfg.merge(PFTauFELinkCfg(cfgFlags))

    from eflowRec.PFRun3Remaps import ListRemaps

    list_remaps=ListRemaps(cfg, 'AOD')
    for mapping in list_remaps:
        cfg.merge(mapping)    

    from PFlowUtils.configureRecoForPFlow import configureRecoForPFlowCfg
    cfg.merge(configureRecoForPFlowCfg(cfgFlags))

    #Add containers needed to run jet finding from resultant AOD for pflow CP studies
    from PFlowUtils.configureRecoForPFlow import addContainersForPFlowCPStudiesCfg
    cfg.merge(addContainersForPFlowCPStudiesCfg(cfgFlags))

    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    from pathlib import Path
    trackingGeometrySvc = ActsTrackingGeometrySvcCfg(cfgFlags,
                                    RunConsistencyChecks=True,
                                    #  ConsistencyCheckOutput="trk_geo_check.csv", # enable debug output writing
                                    BlueprintGraphviz=str(Path.cwd() / "blueprint.dot"),
                                    ObjDebugOutput=False)

    cfg.merge(trackingGeometrySvc)

    cfg.run()
