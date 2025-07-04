# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    cfgFlags = initConfigFlags()
    cfgFlags.Concurrency.NumThreads=8
    cfgFlags.Input.isMC=True
    cfgFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PFlowTests/mc21_13p6TeV/mc21_13p6TeV.601589.PhPy8EG_A14_ttbar_hdamp258p75_nonallhadron.recon.ESD.e8485_s3986_r14060/ESD_100Events.pool.root"]
    cfgFlags.Output.AODFileName="output_AOD.root"
    cfgFlags.Output.doWriteAOD=True
    #This flag enables CaloCalTopoCluster to be created, in addition to CaloTopoCluster
    cfgFlags.Calo.TopoCluster.doTopoClusterLocalCalib=True
    cfgFlags.Calo.TopoCluster.addCalibrationHitDecoration=True
    cfgFlags.Tau.doDiTauRec = False #does not run from ESD - tries to use aux variables which do not exist
    cfgFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(cfgFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(cfgFlags))

    from eflowRec.PFRun3Config import PFFullCfg
    cfg.merge(PFFullCfg(cfgFlags,runTauReco=True))
    
    from eflowRec.PFRun3Remaps import ListRemaps

    list_remaps=ListRemaps()
    for mapping in list_remaps:
        cfg.merge(mapping)    

    from PFlowUtils.configureRecoForPFlow import configureRecoForPFlowCfg
    cfg.merge(configureRecoForPFlowCfg(cfgFlags))

    from PFlowUtils.PFlowCalibHitDecoratorCfg import PFlowCalibHitDecoratorCfg
    cfg.merge(PFlowCalibHitDecoratorCfg(cfgFlags))

    cfg.getEventAlgo("PFlowCalibPFODecoratorAlgorithm").PFOWriteDecorHandleKey_NLeadingTruthParticles="GlobalNeutralParticleFlowObjects.calpfo_NLeadingTruthParticleBarcodeEnergyPairs"

    cfg.run()
