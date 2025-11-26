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
    
    if flags.Detector.EnablePixel:
      bare_collection_name = "PixelHits"
      merger_input_property = "PixelHits"
      region = "ID"
    if flags.Detector.EnableITkPixel:
      bare_collection_name = "ITkPixelHits"
      merger_input_property = "ITkPixelHits"
      region = "ITk"
    mergeable_collection_suffix = "_Fatras"
    acc_pixel, pixel_hits_collection_name = CollectionMergerCfg(flags,
                                                      bare_collection_name,
                                                      mergeable_collection_suffix,
                                                      merger_input_property,
                                                      region)
    acc.merge(acc_pixel)

    if flags.Detector.EnableSCT:
      bare_collection_name = "SCT_Hits"
      merger_input_property = "SCTHits"
      region = "ID"
    if flags.Detector.EnableITkPixel:
      bare_collection_name = "ITkStripHits"
      merger_input_property = "ITkStripHits"
      region = "ITk"
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
    if flags.Detector.EnablePixel:
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('ID', True)):
            writtenContainers += [("SiHitCollection", "PixelHits_Fatras")]
    if flags.Detector.EnableSCT:
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('ID', True)):
           writtenContainers += [("SiHitCollection", "SCT_Hits_Fatras")]
    if flags.Detector.EnableITkPixel:
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('ITk', True)):
            writtenContainers += [("SiHitCollection", "ITkPixelHits_Fatras")]
    if flags.Detector.EnableITkStrip:
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('ITk', True)):
           writtenContainers += [("SiHitCollection", "ITkStripHits_Fatras")]

    kwargs.setdefault("ExtraOutputs", writtenContainers)
    acc.setPrivateTools(CompFactory.ISF.ActsFatrasSimTool(name, **kwargs))
    return acc