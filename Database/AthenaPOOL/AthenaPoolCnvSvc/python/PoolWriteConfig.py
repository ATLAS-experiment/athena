"""Configuration for POOL file writing

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.AccumulatorCache import AccumulatorCache

def _overrideTreeAutoFlush(logger, flags, stream, value):
    """Helper function to override TreeAutoFlush from flags."""
    if not flags.Output.TreeAutoFlush or not isinstance(flags.Output.TreeAutoFlush, dict):
        return value

    if stream not in flags.Output.TreeAutoFlush:
        return value

    override = flags.Output.TreeAutoFlush[stream]
    if override is not None:
        logger.info('Overriding TreeAutoFlush value for stream "%s" from %d to %d', stream, value, override)
        return override

    return value

def _getStreamsFromFlags(flags):
    """
    Helper to get all the streams from configuration flags
    For each stream that's configured to be written out
    we have two flags w/ the following convention:
        + Output.{STREAM}FileName
        + Output.doWrite{STREAM}
    """
    result = []
    for key, value in flags._flagdict.items():
        if key.startswith("Output.") and key.endswith("FileName") and value.get():
            stream = key.removeprefix("Output.").removesuffix("FileName")
            if stream not in ["HIST"]: # AthenaPool is not responsible for HIST storage settings
                result.append(stream)
    return result


@AccumulatorCache
def PoolWriteCfg(flags):
    """Return ComponentAccumulator configured to Write POOL files"""
    # based on WriteAthenaPool._configureWriteAthenaPool

    from AthenaCommon.Logging import logging
    logger = logging.getLogger( 'PoolWriteCfg' )

    PoolAttributes = []
    # Switch off splitting by setting default SplitLevel to 0
    PoolAttributes += ["DEFAULT_SPLITLEVEL ='0'"]

    # Set as default the member-wise streaming, ROOT default
    PoolAttributes += ["STREAM_MEMBER_WISE = '1'"]

    # Increase default BasketSize to 32K, ROOT default (but overwritten by POOL)
    PoolAttributes += ["DEFAULT_BUFFERSIZE = '32000'"]

    # Set POOLContainerForm(DataHeaderForm) split level to 0
    PoolAttributes += ["ContainerName = 'TTree=POOLContainerForm(DataHeaderForm)'; CONTAINER_SPLITLEVEL = '0'"]
    PoolAttributes += ["TREE_BRANCH_OFFSETTAB_LEN ='100'"]

    oneDHForm = flags.Output.OneDataHeaderForm

    # Kept in sync with RecoUtils.py
    from AthenaPoolCnvSvc import PoolAttributeHelper as pah

    # Defaults for common formats
    # Stream : [compression algorithm, compression level, auto flush, split level, dyn split level]
    defaults = {
        "EVNT"           : [2, 1,  500, 0, 0],
        "EVNT_TR"        : [2, 1,    1, 0, 0],
        "HITS"           : [2, 1,   10, 0, 0],
        "RDO"            : [2, 1,   10, 0, 0],
        "ESD"            : [2, 1,   10, 0, 0],
        "AOD"            : [2, 1,  100, 0, 0],
        "DAOD_PHYSVAL"   : [5, 5,  100, 0, 1],
        "DAOD_PHYS"      : [5, 5,  500, 0, 1],
        "DAOD_PHYSLITE"  : [5, 5,  500, 1, 1],
        "DAOD_TRUTH3"    : [5, 5,  500, 1, 1],
        "D2AOD_PHYSLITE" : [5, 5,  500, 1, 1],
    }

    # Metadata containers needed for augmentations
    OutputMetadataContainers = []

    # Loop over all streams and set the appropriate attributes
    maxAutoFlush = -1
    for stream in _getStreamsFromFlags(flags):

        # Get the file name - Guaranteed to exist at this point
        fileName = getattr(flags.Output, f"{stream}FileName")

        # Get the ROOT settings to be applied
        compAlg, compLvl, autoFlush, splitLvl, dynSplitLvl = 2, 1, 10, 0, 0 # Defaults: LZMA, Level 1, AutoFlush 10, No Splitting
        if stream in defaults:
            compAlg, compLvl, autoFlush, splitLvl, dynSplitLvl = defaults[stream]
        elif "DAOD" in stream:
            compAlg, compLvl, autoFlush, splitLvl, dynSplitLvl = 5, 5, 100, 0, 1 # Change the defaults for DAODs
        elif "D2AOD" in stream:
            compAlg, compLvl, autoFlush, splitLvl, dynSplitLvl = 5, 5, 500, 1, 1 # Change the defaults for D2AODs

        # For temporary streams/files we use either ZLIB or ZSTD for the compression algorithm to save CPU cycles
        # Temporary in this context might mean one of three things:
        #   a) Outputs of intermediate steps of chained workflows (file name begins with tmp.),
        #   b) Outputs of workers in AthenaMP jobs that are to be merged (file name ends with _000), and
        #   c) Any output stream that is marked by the user as being temporary (via the CA flag Output.TemporaryStreams)
        # The ultimate goal is to reconcile all three cases and propagate the information between the job transform
        # and the job configuration (CA) so that we don't need to rely on the file names here...
        isTemporaryStream = fileName.endswith('_000') or fileName.startswith('tmp.') or stream in flags.Output.TemporaryStreams
        tempFileCompressionSetting = (5,1) # ZSTD at level 1
        if isTemporaryStream:
            # Outputs created in certain workflows are read with older ROOT versions.
            # E.g., temporary RDO files that are used in Run-2 simulation.
            # For those, we have to use ZLIB
            from AthenaConfiguration.Enums import LHCPeriod
            if "RDO" in stream and hasattr(flags, "GeoModel") and flags.GeoModel.Run < LHCPeriod.Run3:
                tempFileCompressionSetting = (1,1) # ZLIB at level 1
            logger.info(f"Stream {stream} is marked as temporary, overwriting the compression settings to {tempFileCompressionSetting}")
        compAlg, compLvl = tempFileCompressionSetting if isTemporaryStream else (compAlg, compLvl)

        # See if the user asked for the AutoFlush to be overwritten
        autoFlush = _overrideTreeAutoFlush(logger, flags, stream, autoFlush)

        # Print some debugging information
        logger.debug(f"{fileName=} {stream=} {compAlg=} {compLvl=} {autoFlush=} {splitLvl=} {dynSplitLvl=}")

        # Set the Collection/Container prefixes (make configurable?)
        outputCollection = "POOLContainer"
        poolContainerPrefix = "CollectionTree"

        # Check to see if this stream is an augmentation
        # Only set file-level attributes for the owning stream
        isAugmentation = flags.hasFlag(f"Output.{stream}ParentStream")
        if not isAugmentation:
            # Set the Compression attributes
            PoolAttributes += [ pah.setFileCompAlg( fileName, compAlg ) ]
            PoolAttributes += [ pah.setFileCompLvl( fileName, compLvl ) ]

            # By default use a maximum basket buffer size of 128k and minimum buffer entries of 10 for (D)AODs
            if "AOD" in stream:
                PoolAttributes += [ pah.setMaxBufferSize( fileName, "131072" ) ]
                PoolAttributes += [ pah.setMinBufferEntries( fileName, "10" ) ]
        else:
            # Changes in this else block need to be coordinated w/ OutputStreamConfig!
            # Set the master index
            PoolAttributes += [ f"DatabaseName = '{fileName}'; INDEX_MASTER = 'POOLContainer(DataHeader)'" ]

            # Set the Collection/Container prefixes
            outputCollection += f"_{stream}"
            poolContainerPrefix += f"_{stream}"
            OutputMetadataContainers += [f"MetaData_{stream}"]

        # Set the AutoFlush attributes
        PoolAttributes += [ pah.setTreeAutoFlush( fileName, poolContainerPrefix, autoFlush ) ]
        PoolAttributes += [ pah.setTreeAutoFlush( fileName, outputCollection, autoFlush ) ]
        PoolAttributes += [ pah.setTreeAutoFlush( fileName, "POOLContainerForm", autoFlush ) ]

        # Set the Spit Level attributes
        PoolAttributes += [ pah.setContainerSplitLevel( fileName, poolContainerPrefix, splitLvl ) ]
        PoolAttributes += [ pah.setContainerSplitLevel( fileName, "Aux.", splitLvl ) ]
        PoolAttributes += [ pah.setContainerSplitLevel( fileName, "Dyn.", dynSplitLvl ) ]

        # ROOT inadvertently broke forward compatibility in v6.30+ (see root/issues/15964)
        # This workaround is needed so that older releases can read files created by the new ones
        # For more information see ATEAM-1001
        if "EVNT" in stream or "RDO" in stream:
            PoolAttributes += [ f"DatabaseName = '{fileName}'; FILEFORWARD_COMPATIBILITY = '1'" ]
            # also for compatibility with rel21 disable single DataHeaderForm
            oneDHForm = False

        # Find the maximum AutoFlush across all formats
        maxAutoFlush = max(maxAutoFlush, autoFlush)

    # If we don't have "enough" events, disable parallelCompression if we're using SharedWriter
    # In this context, "enough" means each worker has a chance to make at least one flush to the disk
    useParallelCompression = flags.MP.UseSharedWriter and flags.MP.UseParallelCompression
    if useParallelCompression:
        # Now compute the total number of events this job will process
        requestedEvents = flags.Exec.MaxEvents
        availableEvents = flags.Input.FileNentries - flags.Exec.SkipEvents
        totalEntries = availableEvents if requestedEvents == -1 else min( availableEvents, requestedEvents )
        if ( totalEntries > 0 ) and ( maxAutoFlush > 0 ) and ( maxAutoFlush * flags.Concurrency.NumProcs > totalEntries ):
            logger.info( "Not enough events to process, disabling parallel compression for SharedWriter!" )
            logger.info( f"Processing {totalEntries} events in {flags.Concurrency.NumProcs} workers "
                         f"and a maximum (across all outputs) AutoFlush of {maxAutoFlush}")
            useParallelCompression = False

    from AthenaPoolCnvSvc.PoolCommonConfig import AthenaPoolCnvSvcCfg
    return AthenaPoolCnvSvcCfg(flags,
                               PoolAttributes=PoolAttributes,
                               ParallelCompression=useParallelCompression,
                               StorageTechnology=flags.Output.StorageTechnology.EventData,
                               OutputMetadataContainers=OutputMetadataContainers,
                               OneDataHeaderForm = oneDHForm)
