# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    cfgFlags = initConfigFlags()
    cfgFlags.Concurrency.NumThreads=8
    cfgFlags.Input.isMC=True
    cfgFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PFlowTests/mc21_13p6TeV/mc21_13p6TeV.601589.PhPy8EG_A14_ttbar_hdamp258p75_nonallhadron.recon.ESD.e8485_s3986_r14060/ESD_100Events.pool.root"]
    cfgFlags.Output.AODFileName="output_AOD.root"
    cfgFlags.Output.doWriteAOD=True
    cfgFlags.PF.EOverPMode=True
    cfgFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(cfgFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(cfgFlags))
    from eflowRec.PFRun3Config import PFFullCfg
    cfg.merge(PFFullCfg(cfgFlags))
    
    from eflowRec.PFRun3Remaps import ListRemaps

    list_remaps=ListRemaps()
    for mapping in list_remaps:
        cfg.merge(mapping)    

    cfg.run()
