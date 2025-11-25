# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#Runs physics validation on global pflow containers
#Todo that we need to run jet finding first to create the global containers

if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Exec.MaxEvents=100
    flags.Input.isMC=True
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PFlowTests/valid1/valid1.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.deriv.DAOD_PHYSVAL.e8514_s4479_s4377_r16821_p7025/DAOD_PHYSVAL.100evts.pool.root.1"]
    #This will stop jet finding crashing due to missing containers needed for au data
    #We don't need these to validate the FlowElement containers, so no need to worry about this.
    flags.Jet.strictMode = False
    flags.PhysVal.OutputFileName="physval.root"
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    #Run jet finding because it creates the global containers we want to validate
    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow
    cfg.merge( JetRecCfg(flags,AntiKt4EMPFlow) )

    #Configure the physics validation to use global containers
    from PFODQA.PFPhysValConfig import PhysValPFOCfg
    from PhysValMonitoring.PhysValMonitoringConfig import PhysValMonitoringCfg
    cfg.merge(PhysValMonitoringCfg(flags,tools=cfg.popToolsAndMerge(PhysValPFOCfg(flags,useGlobalContainers=True))))

    #remap jet names to avoid errors about modifying locked data
    from SGComps.AddressRemappingConfig import InputRenameCfg
    cfg.merge(InputRenameCfg("xAOD::JetContainer","AntiKt4EMPFlowJets","AntiKt4EMPFlowJets.OLD"))
    cfg.merge(InputRenameCfg("xAOD::JetAuxContainer","AntiKt4EMPFlowJetsAux.","AntiKt4EMPFlowJetsAux.OLD."))

    cfg.run()

