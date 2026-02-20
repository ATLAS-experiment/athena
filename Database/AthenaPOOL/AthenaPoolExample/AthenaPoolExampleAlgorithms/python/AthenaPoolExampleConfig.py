# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# Hide function names in log messages to avoid GCC/Clang formatting differences
import ROOT
ROOT.errorcheck.ReportMessage.hideFunctionNames(True)

# additional read catalogs are optional
def AthenaPoolExampleReadCfg(flags, readCatalogs = [] ):
    """Basic services configuration for AthenaPoolExamples that read imput"""
    acc = ComponentAccumulator()
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge( PoolReadCfg( flags ) )
    acc.getService("PoolSvc").ReadCatalog += readCatalogs
    
    # ---------------- Configure basic metadata
    from xAODMetaDataCnv.InfileMetaDataConfig import createEventStreamInfo, propagateMetaData, MetaDataHelperLists
    from AthenaConfiguration.Enums import MetadataCategory
    mdLists = MetaDataHelperLists()

    lists, caConfig = propagateMetaData( flags, "", MetadataCategory.FileMetaData )
    mdLists += lists
    acc.merge(caConfig)

    mdLists.mdItems += ["IOVMetaDataContainer#*"]

    esiList, _ = createEventStreamInfo(flags)
    mdLists += esiList

    # Configure the MetaDataSvc and pass the relevant tools
    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
    acc.merge( MetaDataSvcCfg( flags, tools = mdLists.mdTools, toolNames = mdLists.mdToolNames ) )

    from AthenaConfiguration.MainServicesConfig import MessageSvcCfg
    acc.merge( MessageSvcCfg( flags ) )
    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels( flags, acc )

    return acc


def AthenaPoolExampleWriteCfg(flags, outputStreamName, writeCatalog = None, disableEventTag = True ):
    """Basic services configuration for AthenaPoolExamples that write output"""
    acc = ComponentAccumulator()
    if not flags.Input.Files:
        # inputless examples use McEventSelector
        from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
        acc.merge( McEventSelectorCfg( flags,
                                       RunNumber         = 1,
                                       EventsPerRun      = 0x100999999,
                                       FirstEvent        = 0x100000001,
                                       EventsPerLB       = 5,
                                       FirstLB           = 1,
                                       InitialTimeStamp  = 0,
                                       TimeStampInterval = 5 ) )

    # Pool writing
    from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
    acc.merge( PoolWriteCfg( flags ) )
    if writeCatalog is not None:
        # Explicitly specify a non-default catalog name
        acc.getService("PoolSvc").WriteCatalog = writeCatalog
    
    # ---------------- Configure basic metadata
    from xAODMetaDataCnv.InfileMetaDataConfig import createEventStreamInfo, propagateMetaData, MetaDataHelperLists
    from AthenaConfiguration.Enums import MetadataCategory
    mdLists = MetaDataHelperLists()

    lists, caConfig = propagateMetaData( flags, outputStreamName, MetadataCategory.FileMetaData )
    mdLists += lists
    acc.merge(caConfig)

    mdLists.mdItems += ["IOVMetaDataContainer#*"]

    esiList, _ = createEventStreamInfo(flags, streamName=outputStreamName)
    mdLists += esiList

    # add metadata items and tools to the output stream
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc.merge( OutputStreamCfg( flags, outputStreamName, disableEventTag = disableEventTag,
                                MetadataItemList = mdLists.mdItems,
                                HelperTools = mdLists.helperTools ) )
    
    # Configure the MetaDataSvc and pass the relevant tools
    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
    acc.merge( MetaDataSvcCfg( flags, tools = mdLists.mdTools, toolNames = mdLists.mdToolNames ) )
    
    from AthenaConfiguration.MainServicesConfig import MessageSvcCfg
    acc.merge( MessageSvcCfg( flags ) )
    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels( flags, acc )

    return acc







