# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configure the ReadTriggerDecision algorithm
# For guidelines on writing configuration scripts see the following pages:
# https://atlassoftwaredocs.web.cern.ch/guides/ca_configuration/

# Imports of the configuration machinery
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def ReadTriggerDecisionCfg(flags):
    '''Method to configure the ReadTriggerDecision algorithm'''
    acc = ComponentAccumulator()

    # Trigger decision tool configuration
    # https://twiki.cern.ch/twiki/bin/viewauth/Atlas/TrigDecisionTool
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))

    # Configure the algorithm.... note that the tool from above is passed
    # Then add the algorithm to the accumulator
    acc.addEventAlgo(CompFactory.ReadTriggerDecision(name = "ReadTriggerDecision",
                                                     TriggerName = "HLT_e5_idperf_tight_L1EM3",
                                                     TriggerDecisionTool = tdt))
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

    # Configure the file reading machinery
    cfg = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags)) 

    # Number of events to process
    flags.Exec.MaxEvents = 1000
    flags.fillFromArgs()
    flags.lock()

    # Run the job
    cfg.merge(ReadTriggerDecisionCfg(flags))
    sys.exit(cfg.run().isFailure())

