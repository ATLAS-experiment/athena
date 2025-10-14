# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

if __name__ == '__main__':
    
    from AthenaCommon.Logging import logging
    from AthenaCommon.Constants import DEBUG

    from add_subsystems import add_subsystems

    logger = logging.getLogger('run_Egamma1eRatio_only')
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
    # if --filesInput was specified as well (!) this will override
    # This does not work, I get an
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

 
    # Generate run3 L1 menu
    #from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg, generateL1Menu
    #acc.merge(L1ConfigSvcCfg(flags))
    #generateL1Menu(flags)

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
    from  GlobalSimulation.Egamma1_LArStrip_Fex_RowAwareCfg import (
        Egamma1_LArStrip_Fex_RowAwareCfg,
        )
    acc.merge(Egamma1_LArStrip_Fex_RowAwareCfg(flags,
                                      OutputLevel=DEBUG,
                                      makeCaloCellContainerChecks=False,
                                      dump=False,
                                               dumpTerse=False))

    # add in the EgammaeRatio Algorithm to be run
    from GlobalSimulation.GlobalSimAlgCfg_Egamma1eRatio  import GlobalSimulationAlgCfg
    acc.merge(GlobalSimulationAlgCfg(flags,
                                     OutputLevel=DEBUG,
                                     dump=True))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc.merge(OutputStreamCfg(flags, 'AOD', ["std::vector<int>#phimax"]))
    acc.merge(OutputStreamCfg(flags, 'AOD', ["std::vector<int>#eRatio"]))
    acc.merge(OutputStreamCfg(flags, 'AOD', ["std::vector<float>#eRatioSimple"]))
    acc.merge(OutputStreamCfg(flags, 'AOD', ["std::vector<float>#eRatioResult"]))
    
    if acc.run().isFailure():
        import sys
        sys.exit(1)

            
