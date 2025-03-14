#!/usr/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory

import os

if __name__ == '__main__':
    
    from AthenaCommon.Logging import logging
    from AthenaCommon.Constants import DEBUG

    logger = logging.getLogger('run_eEmSortSelectCount')
    logger.setLevel(DEBUG)

 
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
       
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

    parser = flags.getArgumentParser()
    args = flags.fillFromArgs(parser=parser)
    flags.dump()
    
    flags.lock()
    flags.dump()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)


    hypoTestBench_alg = CompFactory.GlobalSim.HypoTestBenchAlg(
        "testBenchAlg")

    base_data = "/eos/atlas/atlascerngroupdisk/data-art/"\
        "large-input/trig-val/GlobalSimTest/eEmSortSelectCount/"

    # an example input file name of test vectors
    ## would be os.path.join(base_data, 'tests_00.dat')

    # run from  file.
    hypoTestBench_alg.testsFileName=os.path.join(base_data, 'tests_00.dat')
    
    exp_mults_data =  os.path.join(base_data, 'expected_multiplicity_00.dat')
    hypoTestBench_alg.expectedMultsFileName = exp_mults_data

    exp_tobs_data =  os.path.join(base_data, 'expected_tobs_00.dat')
    hypoTestBench_alg.expectedTobsFileName = exp_tobs_data

    # values for manual testing.
    # manual testing is active if no input file name is given
    hypoTestBench_alg.test_vecs = ['0x1000000000000000fe']
    hypoTestBench_alg.expTobs = '0x' + '0'* (198*8)
    hypoTestBench_alg.expMults = '0x' + '0' * 13
    
    
    acc.addEventAlgo(hypoTestBench_alg)
    from AthenaCommon.Constants import DEBUG
    
    hypoTestBench_alg.OutputLevel = DEBUG

    from GlobalSimAlgCfg_hypo_container import GlobalSimulationAlgCfg
    acc.merge(GlobalSimulationAlgCfg(flags))

    if acc.run().isFailure():
        import sys
        sys.exit(1)

            
