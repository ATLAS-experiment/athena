#!/usr/bin/env python
#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Test script for NavigationDAODTesterAlgv2 - verifies R2 to R3 navigation conversion on DAOD level.

if __name__ == "__main__":
    import sys
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Exec.VerboseMessageComponents = ["*Trigger*"]
    
    # Example input file - MODIFY PATH AS NEEDED for your DAOD file
    # flags.Input.Files=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data18_13TeV.00357772.physics_Main.recon.AOD.r13286/AOD.27654050._000557.pool.root.1"]
    # flags.Input.Files = ["/home/przygoda/build/DAOD_PHYS.DAOD.pool.root"]
    # flags.Input.Files = ["/home/damian/hdd1/DAOD/DAOD_PHYS.41651897._000166.pool.root.1"]
    # flags.Input.Files = ["DAOD_PHYS.DAOD.NEW.pool.root"]
    flags.Input.Files = ["/home/przygoda/build/DAOD_PHYS.DAOD.NEW.pool.root"]
    # flags.Input.Files = ["/home/przygoda/build/DAOD_PHYS.DAOD_PHYS.DAOD.NEW2.pool.root"]
    # flags.Input.Files = ["/home/przygoda/build/DAOD_PHYS.DAOD_PHYS.DAOD.NEW3.pool.root"]
    flags.Exec.MaxEvents = 1000
    flags.Exec.SkipEvents = 0
    flags.fillFromArgs()
    flags.lock()
    
    # Initialize main services and merge necessary configurations.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaCommon.Constants import DEBUG

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg
    cfg.merge(CutFlowSvcCfg(flags))

    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
    cfg.merge(MetaDataSvcCfg(flags))
    
    # Obtain the default Trigger Decision Tool (configured from the file metadata).
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg, getRun3NavigationContainerFromInput
    tdt = cfg.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    tdt.HLTSummary = "HLTNav_Summary_DAODSlimmed"
    tdt.NavigationFormat = "TrigComposite"
    
    # Create matching and composite tools
    r3MatchingTool = CompFactory.Trig.R3MatchingTool("R3MatchingTool")
    r3MatchingTool.TrigDecisionTool = tdt
    r3MatchingTool.IncludeSubfeatures = True  # Enable retrieval of subfeatures (lower-pT objects from Run2->Run3 conversion)
    
    matchFromCompositeTool = CompFactory.Trig.MatchFromCompositeTool("MatchFromCompositeTool")
    matchFromCompositeTool.InputPrefix = "TrigMatch_"
    matchFromCompositeTool.DRThreshold = 0.1  # CRITICAL: Enable DR matching instead of pointer equality
    
    # Select triggers to test - can be customized
    # For a comprehensive list, you can use TriggerListsHelper:
    # from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    # triggerListsHelper = TriggerListsHelper(flags)
    # list_triggers = triggerListsHelper.Run2TriggerNamesNoTau + triggerListsHelper.Run2TriggerNamesTau
    # list_triggers = ['HLT_e7_lhmedium_nod0_mu24']
    # list_triggers = ['HLT_mu24_ivarloose_L1MU15']
    # list_triggers = ['HLT_mu24_ivarloose']
    list_triggers = ['HLT_2mu10_nomucomb'] # example 1
    # list_triggers = ['HLT_e17_lhloose_nod0_2e9_lhloose_nod0'] # example 2
    # list_triggers = ['HLT_mu26_ivarmedium']
    # list_triggers = ['HLT_mu24']
    print("Testing triggers:", list_triggers)
    
    # Create an alternative Run3 TDT explicitly.
    r2ToR3OutputName = getRun3NavigationContainerFromInput(flags)
    run3tdt = CompFactory.Trig.TrigDecisionTool("Run3TrigDecisionTool",
                                                HLTSummary = r2ToR3OutputName,
                                                NavigationFormat = 'TrigComposite',
                                                AcceptMultipleInstance = True,
                                                TrigConfigSvc = tdt.TrigConfigSvc)
    cfg.addPublicTool(run3tdt)
    
    # --- Create NavigationDAODTesterAlgv2 ---
    # This algorithm compares R2 (from pre-stored composites) and R3 (from converted navigation)
    # matching results for offline physics objects.
    
    checker = CompFactory.Trig.NavigationDAODTesterAlgv2(
        TrigDecisionTool = tdt,
        R3MatchingTool = r3MatchingTool,
        MatchFromCompositeTool = matchFromCompositeTool,
        ContainerName = "Muons",
        PrintSubfeatures = True,  # Print subfeatures (lower-pT objects from Run2->Run3 conversion) for investigation
        OutputLevel = DEBUG)
    checker.Chains = list_triggers
    
    cfg.addEventAlgo(checker)
    
    # Configure the MessageSvc for verbose output.
    msg = cfg.getService('MessageSvc')
    msg.debugLimit = 10000
    msg.infoLimit = 10000
    msg.warningLimit = 10000
    msg.Format = '% F%35W%C% F%9W%e%7W%R%T %0W%M'
    
    cfg.printConfig(withDetails=True, summariseProps=False)
    sc = cfg.run()
    sys.exit(0 if sc.isSuccess() else 1)
