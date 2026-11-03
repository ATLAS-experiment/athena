# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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

def ActsFatrasRefineMaterialToolCfg(flags, name="ActsFatrasRefineMaterialTool", **kwargs):
    """Refinement visitor assigning silicon material to sensitive surfaces"""
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ISF.ActsFatrasRefineMaterialTool(name, **kwargs))
    return acc

def ActsFatrasTrackingGeometrySvcCfg(flags, name="ActsFatrasTrackingGeometrySvc", **kwargs):
    """Dedicated TrackingGeometrySvc instance for ActsFatras with refined geometry 
    (Si assigned on sensitive surfaces)."""
    acc = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    svcAcc = ActsTrackingGeometrySvcCfg(flags, name=name, **kwargs)
    svc = svcAcc.getPrimary()
    svc.RefinementTools += [acc.popToolsAndMerge(ActsFatrasRefineMaterialToolCfg(flags))]
    acc.addService(acc.getPrimaryAndMerge(svcAcc), primary=True)
    return acc

def ActsFatrasSimToolCfg(flags, name="ISF_ActsFatrasSimTool", RefineMaterial=None, **kwargs):
    """Return ISF_FatrasSimHitCreatorID configured with ComponentAccumulator

    RefineMaterial: True/False to (not) assign Si to the sensitive surfaces of
    the tracking geometry used by ActsFatras.
    """
    acc = ComponentAccumulator()
    mlog = logging.getLogger(name)
    mlog.info('Start configuration ISF_ActsFatraSimTool')

    kwargs.setdefault("MaxSteps", 20000)

    from ISF_FatrasServices.ISF_FatrasConfig import fatrasKinematicFilterCfg
    kwargs.setdefault("ParticleFilter", acc.addPublicTool(acc.popToolsAndMerge(fatrasKinematicFilterCfg(flags))))
    # added https://its.cern.ch/jira/browse/ATLASSIM-7245
    from ISF_Services.ISF_ServicesConfig import TruthServiceCfg
    kwargs.setdefault("TruthRecordService", acc.getPrimaryAndMerge(TruthServiceCfg(flags)))
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RNGService", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    
    # Ensure GeoIDSvc is configured
    from ISF_Services.ISF_ServicesCoreConfig import ATLFAST_GeoIDSvcCfg
    kwargs.setdefault("GeoIDSvc", acc.getPrimaryAndMerge(ATLFAST_GeoIDSvcCfg(flags)))

    # Tracking geometry: with Si assigment or the standard one 
    if "TrackingGeometrySvc" not in kwargs:
        useRefine = RefineMaterial
        if useRefine:
            mlog.info('Assign Si to sensitive surfaces: ON (ActsFatrasTrackingGeometrySvc)')
            kwargs["TrackingGeometrySvc"] = acc.getPrimaryAndMerge(ActsFatrasTrackingGeometrySvcCfg(flags))
        else:
            mlog.info('Assign Si to sensitive surfaces: OFF (standard ActsTrackingGeometrySvc)')
            from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
            kwargs["TrackingGeometrySvc"] = acc.getPrimaryAndMerge(ActsTrackingGeometrySvcCfg(flags))

    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
    kwargs.setdefault("ExtrapolationTool", acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags)))

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