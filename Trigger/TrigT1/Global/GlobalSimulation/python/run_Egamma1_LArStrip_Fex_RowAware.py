#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

if __name__ == '__main__':
    
    from add_subsystems import add_subsystems

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


    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags, defaultConditionsTags
    flags.Input.Files = defaultTestFiles.RAW_RUN3
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

    # Enable only calo for this test
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags

    setupDetectorFlags(flags, ['LAr','Tile','MBTS'], toggle_geometry=True)

    args = flags.fillFromArgs(parser=parser)
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
    else:
        subsystems = ('eFex',)
        acc.merge(add_subsystems(flags, subsystems, args, OutputLevel=flags.Exec.OutputLevel))

        from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
        acc.merge(ByteStreamReadCfg(flags))

        from TrigCaloRec.TrigCaloRecConfig import hltCaloCellSeedlessMakerCfg
        acc.merge(hltCaloCellSeedlessMakerCfg(flags, roisKey=''))

    # add in the Algorithm to be run
    from Egamma1_LArStrip_Fex_RowAwareCfg import Egamma1_LArStrip_Fex_RowAwareCfg
    acc.merge(Egamma1_LArStrip_Fex_RowAwareCfg(flags,
                                      OutputLevel=flags.Exec.OutputLevel,
                                      makeCaloCellContainerChecks=False,
                                      dump=args.dump,
                                      dumpTerse=args.dumpTerse))

    if acc.run().isFailure():
        import sys
        sys.exit(1)

            
