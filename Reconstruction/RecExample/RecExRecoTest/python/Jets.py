#!/usr/bin/env athena.py --CA
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    # Setup flags with custom input-choice
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.addFlag('RecExRecoTest.doMC', False, help='custom option for RexExRecoText to run data or MC test')
    flags.fillFromArgs()
        
    # Use latest Data or MC
    # This runs from a recent AOD, to check compatibility of any recent jet changes.
    # Tests of jet reconstruction in RAWtoALL is performed by RecJobTransformTests
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultConditionsTags
    if flags.RecExRecoTest.doMC is True:
        flags.Input.Files = defaultTestFiles.AOD_RUN3_MC
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
    else:
        flags.Input.Files = defaultTestFiles.AOD_RUN3_DATA
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA
      
    # We have to set the production step, which PFFlow muon linking uses for autoconfiguration.
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep=ProductionStep.Derivation
        
    flags.lock()

    from JetRecConfig.JetRecoSteering import JetRecoSteeringTest
    JetRecoSteeringTest(flags)
