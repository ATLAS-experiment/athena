# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    cfgFlags = initConfigFlags()
    cfgFlags.Concurrency.NumThreads=8
    cfgFlags.Input.isMC=True
    cfgFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PFlowTests/mc21_13p6TeV/mc21_13p6TeV.601589.PhPy8EG_A14_ttbar_hdamp258p75_nonallhadron.recon.ESD.e8485_s3986_r14060/ESD_100Events.pool.root"]
    cfgFlags.Output.AODFileName="output_AOD.root"
    cfgFlags.Output.doWriteAOD=True
    cfgFlags.PF.addCPData=True
    cfgFlags.Tau.doDiTauRec = False #does not run from ESD - tries to use aux variables which do not exist
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

    cfg.run()
