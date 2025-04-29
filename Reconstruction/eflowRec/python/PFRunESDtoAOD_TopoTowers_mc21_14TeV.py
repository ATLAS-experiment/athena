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
    #We need to use EM scale topotowers for pflow
    cfgFlags.Calo.FwdTower.prepareLCW=False
    cfgFlags.PF.useTopoTowers = True
    #Global links don't currently work with topotowers (other CP objects do not support topotowers)
    cfgFlags.PF.useElPhotLinks = False
    cfgFlags.PF.useMuLinks = False
    cfgFlags.fillFromArgs()
    cfgFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(cfgFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(cfgFlags))

    #Build EM scale combined topocluster + tower container
    from CaloRec.CaloFwdTopoTowerConfig import CaloFwdTopoTowerCfg
    cfg.merge(CaloFwdTopoTowerCfg(cfgFlags,CaloTopoClusterContainerKey="CaloTopoClusters",
                                           towerContainerKey="CaloFwdTopoTowers",
                                           TowerBuilderName="CaloTopoTowerFromClusterMaker",
                                           TowerMergerName="CaloTopoSignalMaker",
                                           signalContainerKey="CaloTopoSignal"))

    #Also rebuild LCW scale combined topocluster + tower container - ensures both containers are of same size, 
    #which is an assumption in the pflow code when dealing with LCW weights
    cfg.merge(CaloFwdTopoTowerCfg(cfgFlags,TowerBuilderName="CaloCalTopoTowerFromClusterMaker",
                                           TowerMergerName="CaloCalTopoSignalMaker",
                                           signalContainerKey="CaloCalTopoSignal"))

    from eflowRec.PFRun3Config import PFFullCfg
    cfg.merge(PFFullCfg(cfgFlags,runTauReco=True))

    from eflowRec.PFRun3Remaps import ListRemaps

    list_remaps=ListRemaps(cfg, 'AOD')
    for mapping in list_remaps:
        cfg.merge(mapping)    

    cfg.run()
