#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

import argparse
from AthenaCommon.Logging import logging
from AthenaCommon.Constants import DEBUG

if __name__ == "__main__":
    logger = logging.getLogger("runPU1Suppression")
    logger.setLevel(DEBUG)

    parser = argparse.ArgumentParser("Run PU1 suppression algorithm")
    parser.add_argument("-n", "--nevent", type=int, default=-1, help="Number of events to run over")
    parser.add_argument("--input", required=True, help="Path to test input file containing 256-bit TOBs, handles both hex and binary")
    parser.add_argument("--rho", required=True, help="Path to rho file")
    args = parser.parse_args()



    flags = initConfigFlags()
    flags.Exec.MaxEvents = args.nevent
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1

    flags.Trigger.doLVL1 = True
    flags.Trigger.enableL1CaloPhase1 = True
    flags.Trigger.EDMVersion = 3
    flags.GeoModel.AtlasVersion = "ATLAS-R3S-2021-03-01-00"

    flags.lock()
    flags.dump()

    acc = MainServicesCfg(flags)
    
    testbench = CompFactory.GlobalSim.PU1SuppTestBenchAlg("PU1SuppTestBenchAlg")
    testbench.TestsFileName = args.input
    testbench.RhoFileName = args.rho
    testbench.OutputLevel = DEBUG
    acc.addEventAlgo(testbench)


    # Simulation logic
    from GlobalSimAlgCfg_PU1_suppression import GlobalSimulationAlgCfg
    acc.merge(GlobalSimulationAlgCfg(flags))

    if acc.run().isFailure():
        import sys
        sys.exit(1)
