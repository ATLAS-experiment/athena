# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configure the RunTriggerMatching algorithm
# For guidelines on writing configuration scripts see the following pages:
# https://atlas-software.docs.cern.ch/athena/configuration/

# Imports of the configuration machinery
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def RunTriggerMatchingCfg(flags):
    '''Method to configure the ReadTriggerDecision algorithm'''
    acc = ComponentAccumulator()

    # Trigger decision tool configuration
    # https://twiki.cern.ch/twiki/bin/viewauth/Atlas/TrigDecisionTool
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))

    # Trigger matching configuration
    # https://twiki.cern.ch/twiki/bin/view/Atlas/R22TriggerAnalysis
    r3MatchingTool = CompFactory.Trig.R3MatchingTool("R3MatchingTool")
    r3MatchingTool.TrigDecisionTool = tdt

    # Configure the algorithm.... note that the tool from above is passed
    # add the algorithm to the accumulator
    acc.addEventAlgo(CompFactory.RunTriggerMatching(name = "RunTriggerMatching",
                                                    TriggerDecisionTool = tdt,
                                                    R3MatchingTool = r3MatchingTool,
                                                    TriggerString = "HLT_mu24.*",
                                                    ContainerName = "Muons"))
    return acc

# Lines to allow the script to be run stand-alone via python
if __name__ == "__main__":

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import sys
    # Configuration flags
    flags = initConfigFlags()
    # Obtain default test files (user can provide their own as well)
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    # Set the input file - can also use command line via --files
    flags.Input.Files = defaultTestFiles.AOD_RUN3_MC

    # Number of events to process
    flags.Exec.MaxEvents = 1000
    flags.fillFromArgs()
    flags.lock()

    # Configure the file reading machinery
    cfg = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # Run the job
    cfg.merge(RunTriggerMatchingCfg(flags))
    sys.exit(cfg.run().isFailure())

