# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
  ComponentAccumulator tool configuration for ISF_ActsTools
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging
from ISF_Algorithms.CollectionMergerConfig import CollectionMergerCfg

def ActsFatrasWriteHandlerCfg(flags, name="ActsFatrasWriteHandler", **kwargs):
    """Return ActsFatrasWriteHandler configured with ComponentAccumulator"""
    acc = ComponentAccumulator()
    mlog = logging.getLogger(name)
    mlog.info('Start configuration ActsFatrasWriteHandler')

    bare_collection_name = "PixelHits"
    mergeable_collection_suffix = "_Fatras"
    merger_input_property = "PixelHits"
    region = "ID"
    acc_pixel, pixel_hits_collection_name = CollectionMergerCfg(flags,
                                                       bare_collection_name,
                                                       mergeable_collection_suffix,
                                                       merger_input_property,
                                                       region)
    acc.merge(acc_pixel)

    bare_collection_name = "SCT_Hits"
    mergeable_collection_suffix = "_Fatras"
    merger_input_property = "SCTHits"
    region = "ID"
    acc_sct, sct_hits_collection_name = CollectionMergerCfg(flags,
                                                       bare_collection_name,
                                                       mergeable_collection_suffix,
                                                       merger_input_property,
                                                       region)
    acc.merge(acc_sct)

    kwargs.setdefault("PixelCollectionName", pixel_hits_collection_name)
    kwargs.setdefault("SCTCollectionName", sct_hits_collection_name)

    acc.setPrivateTools(CompFactory.ActsFatrasWriteHandler(name=name, **kwargs))
    return acc

def ActsFatrasSimToolCfg(flags, name="ISF_ActsFatrasSimTool", **kwargs):
    """Return ISF_FatrasSimHitCreatorID configured with ComponentAccumulator"""
    acc = ComponentAccumulator()
    mlog = logging.getLogger(name)
    mlog.info('Start configuration ISF_ActsFatrasSimTool')
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    kwargs.setdefault('TrackingGeometryTool', acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))

    kwargs.setdefault("MaxSteps", 2000)
    
    # added https://its.cern.ch/jira/browse/ATLASSIM-7245
    from ISF_Services.ISF_ServicesConfig import TruthServiceCfg
    kwargs.setdefault("TruthRecordService", acc.getPrimaryAndMerge(TruthServiceCfg(flags)))
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RNGService", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)))

    kwargs.setdefault("ActsFatrasWriteHandler", acc.popToolsAndMerge(ActsFatrasWriteHandlerCfg(flags)))
    writtenContainers =[]
    writtenContainers += [("SiHitCollection", "PixelHits_Fatras")]
    writtenContainers += [("SiHitCollection", "SCT_Hits_Fatras")]
    kwargs.setdefault("ExtraOutputs", writtenContainers)
    acc.setPrivateTools(CompFactory.ISF.ActsFatrasSimTool(name, **kwargs))
    return acc
