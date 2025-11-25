# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def fromRunArgs(runArgs):

    """ This is the main skeleton for merging POOL files generically.
    Currently it handles (D)AOD/(D)ESD files, but can be extended in the future.
    That'll mostly entail configuring extra components needed for TP conversion (if any).
    """

    # Setup logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('MergePool_Skeleton')
    log.info('****************** STARTING MergePool MERGING *****************')

    # Print arguments
    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    # Setup configuration flags
    log.info('**** Setting up configuration flags')

    from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Exec.EventPrintoutInterval = 100
    commonRunArgsToFlags(runArgs, flags)

    # First let's find the input/output files
    inputFile, outputFile = None, None

    for attr in dir(runArgs):
        if attr.startswith('input') and attr.endswith('File'):
            inputFile = getattr(runArgs, attr)
        elif attr.startswith('output') and attr.endswith('File'):
            outputFile = getattr(runArgs, attr)

    if not inputFile or not outputFile:
        raise RuntimeError('Could NOT determine the input/output files!')

    # Now set the input files before we attempt to read the processing tags
    flags.Input.Files = inputFile

    # Now figure out what stream type we're trying to merge
    streamToMerge = flags.Input.ProcessingTags[0].removeprefix('Stream') if flags.Input.ProcessingTags else None

    if not streamToMerge:
        raise RuntimeError('Could NOT determine the stream type!')

    # Now set the output file name and add additional flags
    # For known formats, e.g., ESD, AOD, we already have the associated flags
    # However, for other formats, e.g., DAOD_XYZ, we need to create the flags
    try:
        setattr(flags.Output, f'{streamToMerge}FileName', outputFile)
    except RuntimeError: # If a flag doesn't exist CA throws a runtime error
        flags.addFlag(f'Output.{streamToMerge}FileName', outputFile)
        flags.addFlag(f'Output.doWrite{streamToMerge}', True)
        if 'DAOD' in streamToMerge:
            flags.Output.doWriteDAOD = True

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # Pre-include
    from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
    log.info('**** Processing preInclude')
    processPreInclude(runArgs, flags)

    # Pre-exec
    log.info('**** Processing preExec')
    processPreExec(runArgs, flags)

    # To respect --athenaopts
    log.info('**** Processing athenaopts')
    flags.fillFromArgs()

    # Lock configuration flags
    log.info('**** Locking configuration flags')
    flags.lock()

    # Set up necessary job components
    log.info('**** Setting up job components')

    # Main services
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    # Input reading
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # Output writing

    # Configure the output stream
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg, outputStreamName
    cfg.merge(OutputStreamCfg(flags, streamToMerge, takeItemsFromInput = True, extendProvenanceRecord = False))
    Stream = cfg.getEventAlgo(outputStreamName(streamToMerge))
    Stream.ForceRead = True
    # Add in-file MetaData
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from AthenaConfiguration.Enums import MetadataCategory

    cfg.merge(
        SetupMetaDataForStreamCfg(
            flags,
            streamToMerge,
            createMetadata=[
                MetadataCategory.IOVMetaData,
            ],
        )
    )

    log.info(f'**** Configured {streamToMerge} writing')

    # Configure extra bits that are needed for TP conversion
    for item in flags.Input.TypedCollections:
        ctype, cname = item.split('#')
        if ctype.startswith('Trk') or ctype.startswith('InDet'):
            from TrkEventCnvTools.TrkEventCnvToolsConfig import TrkEventCnvSuperToolCfg
            cfg.merge(TrkEventCnvSuperToolCfg(flags))
        if ctype.startswith('Calo') or ctype.startswith('LAr'):
            from LArGeoAlgsNV.LArGMConfig import LArGMCfg
            cfg.merge(LArGMCfg(flags))
        if ctype.startswith('Calo') or ctype.startswith('Tile'):
            from TileGeoModel.TileGMConfig import TileGMCfg
            cfg.merge(TileGMCfg(flags))
        if ctype.startswith('Muon'):
            from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
            cfg.merge(MuonGeoModelCfg(flags))

    # Needed for merging in MT
    if 'ESD' in streamToMerge:
        Stream.ExtraInputs.add(
            ( 'MuonGM::MuonDetectorManager',
                  'ConditionStore+MuonDetectorManager' ) )
        Stream.ExtraInputs.add(
            ( 'InDetDD::SiDetectorElementCollection',
                  'ConditionStore+PixelDetectorElementCollection' ) )
        Stream.ExtraInputs.add(
            ( 'InDetDD::SiDetectorElementCollection',
                  'ConditionStore+SCT_DetectorElementCollection' ) )
        Stream.ExtraInputs.add(
            ( 'InDetDD::TRT_DetElementContainer',
                  'ConditionStore+TRT_DetElementContainer' ) )

    # Post-include
    log.info('**** Processing postInclude')
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    log.info('**** Processing postExec')
    processPostExec(runArgs, flags, cfg)

    # Now run the job and exit accordingly
    sc = cfg.run()
    import sys
    sys.exit(not sc.isSuccess())
