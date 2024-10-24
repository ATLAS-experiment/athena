#!/usr/bin/env athena.py --CA
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    
    # Setup flags with custom input-choice
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.addFlag('RecExRecoTest.doMC', False, help='custom option for RexExRecoText to run data or MC test')
    flags.fillFromArgs()
        
    # Use latest Data or MC
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultConditionsTags
    if flags.RecExRecoTest.doMC:
        flags.Input.Files = defaultTestFiles.ESD_RUN3_MC
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
    else:
        flags.Input.Files = defaultTestFiles.ESD_RUN3_DATA22
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA    
    flags.lock()

    from eflowRec.PFRun3Config import PFRun3ConfigTest
    PFRun3ConfigTest(flags)

