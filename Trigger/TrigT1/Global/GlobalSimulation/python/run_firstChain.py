# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__ == '__main__':
    
    from AthenaCommon.Logging import logging
    from AthenaCommon.Constants import DEBUG

    logger = logging.getLogger('run_firstChain')
    logger.setLevel(DEBUG)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    parser = flags.getArgumentParser()
    
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
    flags.Trigger.triggerConfig='FILE'

    flags.fillFromArgs(parser=parser)

    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, ['LAr','Tile','MBTS'], toggle_geometry=True)

    flags.lock()
    flags.dump()

    ##################################################
    # Set up central services: Main + Input reading + L1Menu + Output writing
    ##################################################
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    # Generate run3 L1 menu
    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg, generateL1Menu
    acc.merge(L1ConfigSvcCfg(flags))
    generateL1Menu(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    from TrigCaloRec.TrigCaloRecConfig import hltCaloCellSeedlessMakerCfg
    acc.merge(hltCaloCellSeedlessMakerCfg(flags, roisKey=''))

    from AthenaConfiguration.Enums import Format
    if flags.Input.Format == Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        acc.merge(PoolReadCfg(flags))
    else:
        from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
        acc.merge(ByteStreamReadCfg(flags))

    # Add TopoClusterConfig to build calo conditions correctly
    from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg
    caloconditions = CaloTopoClusterCfg(flags)
    acc.merge(caloconditions)

    gepAlgs_output_level = DEBUG
    
    # Add algorithm to prepare LAr cells for Global
    from  GlobalSimulation.LArCellPreparationAlgCfg import LArCellPreparationAlgCfg
    gblLArCellContainerKey = "GlobalLArCells"
    acc.merge(LArCellPreparationAlgCfg(flags,
                               NumberOfEnergyBits = 6,
                               ValueLeastSignificantBit = 40,
                               ValueGainFactor = 4,
                               gblLArCellsKey = gblLArCellContainerKey,
                               OutputLevel=DEBUG))

    # Add algorithm to simulate MUX input/output for LAr cells
    from  GlobalSimulation.LArCellMuxAlgCfg import LArCellMuxAlgCfg
    acc.merge(LArCellMuxAlgCfg(flags,
                               gblLArCellsKey = gblLArCellContainerKey,
                               writeMuxInputBitstreamToFile = True,
                               writeMuxOutputBitstreamToFile = True,
                               OutputLevel=DEBUG))

    # Algorithm to build cell towers
    from  GlobalSimulation.GlobalCellTowerAlgToolCfg import GlobalCellTowerAlgToolCfg
    acc.merge(GlobalCellTowerAlgToolCfg(flags,
                               gblLArCellsKey = gblLArCellContainerKey,
                               gblCellTowersKey = "GlobalCellTowers",
                               OutputLevel=DEBUG))

    if acc.run().isFailure():
        import sys
        sys.exit(1)        
