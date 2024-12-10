# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__ == '__main__':
    
    from AthenaCommon.Logging import logging
    from AthenaCommon.Constants import DEBUG

    from add_subsystems import add_subsystems

    logger = logging.getLogger('run_Egamma1BDT_only')
    logger.setLevel(DEBUG)

    
    import argparse
    from argparse import RawTextHelpFormatter

    
    parser = argparse.ArgumentParser(
        "Running GlobalSim Egamma1BDT_only",
        formatter_class=RawTextHelpFormatter)


    
    parser.add_argument(
        "-i",
        "--inputs",
        nargs='*',
        action="store",
        dest="inputFiles",
        help="files to process",
        required=True)
    

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
        
    if args.inputFiles:
        flags.Input.Files = args.inputFiles
    else:
        flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/RAW/data23_13p6TeV.00452463.physics_Main.daq.RAW/540events.data23_13p6TeV.00452463.physics_Main.daq.RAW._lb0514._SFO-16._0004.data']
        
     
    flags.Output.AODFileName = 'AOD.pool.root'
    flags.Common.isOnline = not flags.Input.isMC
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


    # Enable only calo for this test
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags

    setupDetectorFlags(flags, ['LAr','Tile','MBTS'], toggle_geometry=True)

    flags.lock()
    flags.dump()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

 
    # Generate run3 L1 menu
    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg, generateL1Menu
    acc.merge(L1ConfigSvcCfg(flags))
    generateL1Menu(flags)

    from AthenaConfiguration.Enums import Format
    if flags.Input.Format == Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        acc.merge(PoolReadCfg(flags))

        from TrigCaloRec.TrigCaloRecConfig import hltCaloCellSeedlessMakerCfg
        acc.merge(hltCaloCellSeedlessMakerCfg(flags, roisKey='')) 

    else:
        subsystems = ('eFex',)
        acc.merge(add_subsystems(flags, subsystems, args, OutputLevel=DEBUG))

        from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
        acc.merge(ByteStreamReadCfg(flags))

        from TrigCaloRec.TrigCaloRecConfig import hltCaloCellSeedlessMakerCfg
        acc.merge(hltCaloCellSeedlessMakerCfg(flags, roisKey=''))

    # add in the Algortihm to build a  LArStrip Neighborhood container
    from  GlobalSimulation.Egamma1_LArStrip_FexCfg import (
        Egamma1_LArStrip_FexCfg,
        )
    acc.merge(Egamma1_LArStrip_FexCfg(flags,
                                      OutputLevel=DEBUG,
                                      makeCaloCellContainerChecks=False,
                                      dump=True,
                                      dumpTerse=True))

    # add in the EgammaBDT Algorithm to be run
    from GlobalSimulation.GlobalSimAlgCfg_Egamma1BDT  import GlobalSimulationAlgCfg
    acc.merge(GlobalSimulationAlgCfg(flags,
                                     OutputLevel=DEBUG,
                                     dump=True))


    if acc.run().isFailure():
        import sys
        sys.exit(1)

            
