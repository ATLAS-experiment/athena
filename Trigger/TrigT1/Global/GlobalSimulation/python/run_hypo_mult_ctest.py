#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__ == '__main__':
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    parser = flags.getArgumentParser()

    parser.add_argument(
        "-ifex",
        "--doCaloInput",
        action="store_true",
        dest="doCaloInput",
        help="Decoding L1Calo inputs",
        default=False,
        required=False)

    parser.add_argument(
        "--dump",
        action="store_true",
        help="Write out dumps",
        default=False)

    parser.add_argument(
        "--dumpTerse",
        action="store_true",
        help="Write out dumps: tersely",
        default=False)

    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Trigger.doLVL1 = True

    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataFlow = True

    args = flags.fillFromArgs(parser=parser)
    flags.lock()
    flags.dump()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    # add in the Algorithm to be run
    from GlobalSimAlgCfg_hypo_mult_ctest import GlobalSimulationAlgCfg
    acc.merge(GlobalSimulationAlgCfg(flags, dump=True))

    if acc.run().isFailure():
        import sys
        sys.exit(1)

            
