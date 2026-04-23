# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

if __name__ == '__main__':
    
    from AthenaCommon.Logging import logging
    from AthenaCommon.Constants import DEBUG

    from add_subsystems import add_subsystems

    logger = logging.getLogger('run_Egamma1BDT_only')
    logger.setLevel(DEBUG)

    
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

    args, _ = parser.parse_known_args()    
 
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags, defaultConditionsTags
    # Default to the current data test file for Run-3
    if not args.filesInput:
        flags.Input.Files = defaultTestFiles.RAW_RUN3_DATA24
        flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA24
     
    flags.Output.AODFileName = 'AOD.pool.root'
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Trigger.doLVL1 = True

    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataFlow = True
    flags.Trigger.EDMVersion = 3
    flags.Trigger.enableL1CaloPhase1 = True

    flags.fillFromArgs(parser=parser)
    
    # Enable only calo for this test
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags

    setupDetectorFlags(flags, ['LAr','Tile','MBTS'], toggle_geometry=True)

    flags.lock()
    flags.dump()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

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

    # Add TopoClusterConfig to build calo conditions correctly
    from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg
    caloconditions = CaloTopoClusterCfg(flags)
    acc.merge(caloconditions)

    # Add algorithm to prepare LAr cells for Global
    from  GlobalSimulation.LArCellPreparationAlgCfg import LArCellPreparationAlgCfg
    gblLArCellContainerKey = "GlobalLArCells"
    acc.merge(LArCellPreparationAlgCfg(flags,
                               NumberOfEnergyBits = 6,
                               ValueLeastSignificantBit = 40,
                               ValueGainFactor = 4,
                               gblLArCellsKey = gblLArCellContainerKey,
                               OutputLevel=DEBUG))

    
    # add in the Algortihm to build an online GlobalLArStrip Neighborhood container
    from  GlobalSimulation.Egamma1_OnlineMapNbhoodCfg import (
        Egamma1_OnlineMapNbhoodCfg,
        )
    acc.merge(Egamma1_OnlineMapNbhoodCfg(flags,
                                          OutputLevel=DEBUG,
                                          makeCaloCellContainerChecks=False,
                                          dump=True,
                                          dumpTerse=False))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc.merge(OutputStreamCfg(flags, 'AOD', ["std::vector<float>#eFEXeta"]))
    acc.merge(OutputStreamCfg(flags, 'AOD', ["std::vector<float>#eFEXphi"]))
    acc.merge(OutputStreamCfg(flags, 'AOD', ["std::vector<float>#FailedeFEXeta"]))
    acc.merge(OutputStreamCfg(flags, 'AOD', ["std::vector<float>#FailedeFEXphi"]))
    
    acc.getService("MessageSvc").debugLimit = 100000000
    
    # add in the EgammaBDT Algorithm to be run
    from GlobalSimulation.GlobalSimAlgCfg_Egamma1BDT  import GlobalSimulationAlgCfg
    acc.merge(GlobalSimulationAlgCfg(flags,
                                     OutputLevel=DEBUG,
                                     dump=False))

    acc.merge(OutputStreamCfg(flags, 'AOD', ["std::vector<float>#eGamma1BDT"]))
    
    if acc.run().isFailure():
        import sys
        sys.exit(1)

            
