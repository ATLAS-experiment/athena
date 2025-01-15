# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#Runs physics validation on global pflow containers
#Todo that we need to run jet finding first to create the global containers

if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    cfgFlags = initConfigFlags()
    cfgFlags.Exec.MaxEvents=100
    cfgFlags.Input.isMC=True
    cfgFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PFlowTests/valid1/valid1.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.deriv.DAOD_PHYSVAL.e8514_a945_s4374_r16157_p6557/DAOD_PHYSVAL.42163833._000004.pool.root.1"]
    #This will stop jet finding crashing due to missing containers needed for au data
    #We don't need these to validate the FlowElement containers, so no need to worry about this.
    cfgFlags.Jet.strictMode = False
    cfgFlags.PhysVal.OutputFileName="physval.root"
    cfgFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(cfgFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(cfgFlags))

    #Run jet finding because it creates the global containers we want to validate
    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow
    cfg.merge( JetRecCfg(cfgFlags,AntiKt4EMPFlow) )     

    #Configure the physics validation to use global containers
    from PFODQA.PFPhysValConfig import PhysValPFOCfg
    from PhysValMonitoring.PhysValMonitoringConfig import PhysValMonitoringCfg
    cfg.merge(PhysValMonitoringCfg(cfgFlags,tools=cfg.popToolsAndMerge(PhysValPFOCfg(cfgFlags,useGlobalContainers=True))))

    #remap jet names to avoid errors about modifying locked data
    from SGComps.AddressRemappingConfig import InputRenameCfg
    cfg.merge(InputRenameCfg("xAOD::JetContainer","AntiKt4EMPFlowJets","AntiKt4EMPFlowJets.OLD"))
    cfg.merge(InputRenameCfg("xAOD::JetAuxContainer","AntiKt4EMPFlowJetsAux.","AntiKt4EMPFlowJetsAux.OLD."))


    cfg.run()

