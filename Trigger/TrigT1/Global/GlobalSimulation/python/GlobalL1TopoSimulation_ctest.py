#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import VERBOSE
logger.setLevel(VERBOSE)

import sys

if __name__ == '__main__':
    from AthenaCommon.Logging import logging

    from AthenaCommon.Constants import DEBUG


    log = logging.getLogger('globalSim')
    log.setLevel(DEBUG)
    algLogLevel = DEBUG

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

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RAW_RUN3
    
        
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1

    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataFlow = True
    flags.Trigger.EDMVersion = 3
    flags.Trigger.enableL1CaloPhase1 = True


    flags.Output.AODFileName = 'AOD.pool.root'
    flags.Trigger.triggerMenuSetup = 'PhysicsP1_pp_run3_v1'

    flags.GeoModel.AtlasVersion="ATLAS-R3S-2021-03-01-00"
    flags.Common.isOnline = not flags.Input.isMC
    

    args = flags.fillFromArgs(parser=parser)
    flags.dump()

    flags.lock()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags, type_names=['CTP_RDO/CTP_RDO']))

    # Generate run3 L1 menu
    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg, generateL1Menu
    acc.merge(L1ConfigSvcCfg(flags))
    generateL1Menu(flags)

    subsystems = ('jFex', 'eFex')
    from add_subsystems import add_subsystems

    acc.merge(add_subsystems(flags,
                             subsystems,
                             args,
                             OutputLevel=flags.Exec.OutputLevel))

    from GlobalSimulation.GlobalL1TopoSimulation import (
        GlobalL1TopoSimulationCfg,
        )
    
    acc.merge(GlobalL1TopoSimulationCfg(flags, algLogLevel))
      
    
    roib2topo = CompFactory.LVL1.RoiB2TopoInputDataCnv(
        name='RoiB2TopoInputDataCnv')
    
    roib2topo.OutputLevel = algLogLevel
    acc.addEventAlgo(roib2topo, sequenceName="AthAlgSeq")

    from L1TopoByteStream.L1TopoByteStreamConfig import L1TopoByteStreamCfg
    acc.merge(L1TopoByteStreamCfg(flags), sequenceName='AthAlgSeq')
    
    if acc.run().isFailure():
        sys.exit(1)

    
