# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    cfgFlags = initConfigFlags()
    cfgFlags.Concurrency.NumThreads=8
    cfgFlags.Input.isMC=True
    cfgFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PFlowTests/mc21_13p6TeV/mc21_13p6TeV.601589.PhPy8EG_A14_ttbar_hdamp258p75_nonallhadron.recon.ESD.e8485_s3986_r14060/ESD.31373517._000035.pool.root.1"]
    cfgFlags.Output.AODFileName="output_AOD.root"
    cfgFlags.Output.doWriteAOD=True
    cfgFlags.PF.doThinning=False
    cfgFlags.PF.EOverPMode=True
    cfgFlags.fillFromArgs()
    cfgFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(cfgFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(cfgFlags))
    from eflowRec.PFRun3Config import PFFullCfg
    cfg.merge(PFFullCfg(cfgFlags))
    
    from eflowRec.PFRun3Remaps import ListRemaps

    list_remaps=ListRemaps(cfg, 'AOD')
    for mapping in list_remaps:
        cfg.merge(mapping)    

    #Add metadata which is required for AOD->ntuple step of e/p derivation.
    from PFlowUtils.configureRecoForPFlow import configureMetaDataForPFlowCfg
    cfg.merge(configureMetaDataForPFlowCfg(cfgFlags))

    cfg.run()
