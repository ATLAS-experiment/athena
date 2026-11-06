# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
  ComponentAccumulator tool configuration for ISF_ActsTools
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging
from ISF_Algorithms.CollectionMergerConfig import CollectionMergerCfg

def _fatrasHitCollections(flags):
    """(write-handler property, bare collection name, CollectionMerger input
    property, merge region) of every SiHit collection ActsFatras can write,
    for the detectors enabled in this job. Same names as the Geant4 SDs."""
    colls = []
    if flags.Detector.EnableITkPixel:
        colls.append(("PixelCollectionName", "ITkPixelHits", "ITkPixelHits", "ITk"))
    elif flags.Detector.EnablePixel:
        colls.append(("PixelCollectionName", "PixelHits", "PixelHits", "ID"))
    if flags.Detector.EnableITkStrip:
        colls.append(("SCTCollectionName", "ITkStripHits", "ITkStripHits", "ITk"))
    elif flags.Detector.EnableSCT:
        colls.append(("SCTCollectionName", "SCT_Hits", "SCTHits", "ID"))
    if flags.Detector.EnableHGTD:
        colls.append(("HGTDCollectionName", "HGTD_Hits", "HGTDHits", "HGTD"))
    if flags.Detector.EnablePLR:
        colls.append(("PLRCollectionName", "PLR_Hits", "PLRHits", "ITk"))
    return colls

FatrasHitsSuffix = "_Fatras"

def ActsFatrasWriteHandlerCfg(flags, name="ActsFatrasWriteHandler", **kwargs):
    """Return ActsFatrasWriteHandler configured with ComponentAccumulator"""
    acc = ComponentAccumulator()
    mlog = logging.getLogger(name)
    mlog.info('Start configuration ActsFatrasWriteHandler')

    # Collections not listed here keep an empty name, i.e. are not written
    for prop, bare_collection_name, merger_input_property, region in _fatrasHitCollections(flags):
        acc_coll, hits_collection_name = CollectionMergerCfg(flags,
                                                             bare_collection_name,
                                                             FatrasHitsSuffix,
                                                             merger_input_property,
                                                             region)
        acc.merge(acc_coll)
        kwargs.setdefault(prop, hits_collection_name)
        mlog.info('%s = %s', prop, hits_collection_name)

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
    writtenContainers = []
    if flags.Sim.ISFRun:
        for _, bare_collection_name, _, region in _fatrasHitCollections(flags):
            if flags.Sim.ISF.HITSMergingRequired.get(region, True):
                writtenContainers += [("SiHitCollection", bare_collection_name + FatrasHitsSuffix)]
    kwargs.setdefault("ExtraOutputs", writtenContainers)
    acc.setPrivateTools(CompFactory.ISF.ActsFatrasSimTool(name, **kwargs))
    return acc