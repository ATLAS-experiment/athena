# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


"""
  ComponentAccumulator tool configuration for ISF_ActsTools
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging

def ActsFatrasSimToolCfg(flags, name="ISF_ActsFatrasSimTool", **kwargs):
    """Return ISF_FatrasSimHitCreatorID configured with ComponentAccumulator"""
    acc = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    kwargs.setdefault('TrackingGeometryTool', acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)))

    kwargs.setdefault("MaxSteps", 2000)
    
    # added https://its.cern.ch/jira/browse/ATLASSIM-7245
    from ISF_Services.ISF_ServicesConfig import TruthServiceCfg
    kwargs.setdefault("TruthRecordService", acc.getPrimaryAndMerge(TruthServiceCfg(flags)).name)
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RNGService", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)

    acc.setPrivateTools(CompFactory.ISF.ActsFatrasSimTool(name, **kwargs))
    return acc

def ActsFatrasWriteHandlerCfg(flags, name="ActsFatrasWriteHandler", **kwargs):
    """Return ActsFatrasWriteHandler configured with ComponentAccumulator"""
    from ISF_Algorithms.CollectionMergerConfig import CollectionMergerCfg

    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    result = ComponentAccumulator()

    bare_collection_name = "PixelHits"
    mergeable_collection_suffix = "_ActsFatras"
    merger_input_property = "PixelHits"
    region = "ID"

    acc, pixel_hits_collection_name = CollectionMergerCfg(flags,
                                                       bare_collection_name,
                                                       mergeable_collection_suffix,
                                                       merger_input_property,
                                                       region)
    result.merge(acc)

    bare_collection_name = "SCT_Hits"
    mergeable_collection_suffix = "_ActsFatras"
    merger_input_property = "SCT_Hits"
    region = "ID"

    acc, sct_hits_collection_name = CollectionMergerCfg(flags,
                                                       bare_collection_name,
                                                       mergeable_collection_suffix,
                                                       merger_input_property,
                                                       region)
    result.merge(acc)
    
    kwargs.setdefault("PixelCollectionName", pixel_hits_collection_name)
    kwargs.setdefault("SCTCollectionName", sct_hits_collection_name)

    result.setPrivateTools(CompFactory.ActsFatrasWriteHandler(name=name, **kwargs))
    return result