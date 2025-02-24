# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory

if __name__ == '__main__':
    
    from AthenaCommon.Logging import logging
    from AthenaCommon.Constants import DEBUG

    logger = logging.getLogger('run_nodata')
    logger.setLevel(DEBUG)

    
    import argparse
    from argparse import RawTextHelpFormatter

    
    parser = argparse.ArgumentParser(
        "Running UCL hypo block",
        formatter_class=RawTextHelpFormatter)


    parser.add_argument(
        "-n",
        "--nevent",
        type=int,
        action="store",
        dest="nevent",
        help="Maximum number of events will be executed.",
        default=0,
        required=False)

    parser.add_argument(
        "-s",
        "--skipEvents",
        type=int,
        action="store",
        dest="skipEvents",
        help="Number of  events to skip.",
        default=0,
        required=False)

    
    parser.add_argument(
        "-ifex",
        "--doCaloInput",
        action="store_true",
        dest="doCaloInput",
        help="Decoding L1Calo inputs",
        default=False,
        required=False)


    args = parser.parse_args()

    logger.debug('args:')

    logger.debug(args)
    
 
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
       
    if(args.nevent > 0):
        flags.Exec.MaxEvents = args.nevent
     
    #flags.Common.isOnline = not flags.Input.isMC
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Trigger.doLVL1 = True

  
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1

    flags.GeoModel.AtlasVersion="ATLAS-R3S-2021-03-01-00"

    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataFlow = True
    flags.Trigger.EDMVersion = 3
    flags.Trigger.doLVL1 = True
    flags.Trigger.enableL1CaloPhase1 = True


    flags.lock()
    flags.dump()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)


    hypoTestBench_alg = CompFactory.GlobalSim.HypoTestBenchAlg(
        "testBenchAlg")

    hypoTestBench_alg.eEmFileName = \
        "/eos/atlas/atlascerngroupdisk/data-art/large-input/trig-val/GlobalSimTest/Data_eEmTobs.dat"
    
    acc.addEventAlgo(hypoTestBench_alg)
    from AthenaCommon.Constants import DEBUG
    
    hypoTestBench_alg.OutputLevel = DEBUG

    from GlobalSimAlgCfg_hypo_container import GlobalSimulationAlgCfg
    acc.merge(GlobalSimulationAlgCfg(flags))

    if acc.run().isFailure():
        import sys
        sys.exit(1)

            
