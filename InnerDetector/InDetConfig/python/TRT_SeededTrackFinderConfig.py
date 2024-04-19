# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_SeededTrackFinder package
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

def TRT_SeededTrackFinderCfg(flags, name='InDetTRT_SeededTrackFinder',
                             InputCollections=None,
                             **kwargs):

    # TRT seeded back tracking algorithm
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc = BeamSpotCondAlgCfg(flags)

    if flags.Tracking.ActiveConfig.usePixel:
        from InDetConfig.SiCombinatorialTrackFinderToolConfig import (
            SiDetElementBoundaryLinksCondAlg_xk_Pixel_Cfg)
        acc.merge(SiDetElementBoundaryLinksCondAlg_xk_Pixel_Cfg(flags))

    if flags.Tracking.ActiveConfig.useSCT:
        from InDetConfig.SiCombinatorialTrackFinderToolConfig import (
            SiDetElementBoundaryLinksCondAlg_xk_SCT_Cfg)
        acc.merge(SiDetElementBoundaryLinksCondAlg_xk_SCT_Cfg(flags))

    if "RefitterTool" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import InDetTrackFitterBTCfg
        kwargs.setdefault("RefitterTool", acc.popToolsAndMerge(
            InDetTrackFitterBTCfg(flags)))

    if "TrackExtensionTool" not in kwargs:
        from InDetConfig.TRT_TrackExtensionToolConfig import (
            TRT_TrackExtensionToolCfg)
        kwargs.setdefault("TrackExtensionTool", acc.popToolsAndMerge(
            TRT_TrackExtensionToolCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            InDetTrackSummaryToolNoHoleSearchCfg)
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(
            InDetTrackSummaryToolNoHoleSearchCfg(flags)))

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    if "TrackTool" not in kwargs:
        from InDetConfig.TRT_SeededTrackFinderToolConfig import (
            TRT_SeededTrackFinder_ATLCfg)
        InDetTRT_SeededTrackTool = acc.popToolsAndMerge(
            TRT_SeededTrackFinder_ATLCfg(flags,
                                         InputCollections=InputCollections))
        acc.addPublicTool(InDetTRT_SeededTrackTool)
        kwargs.setdefault("TrackTool", InDetTRT_SeededTrackTool)

    kwargs.setdefault("PRDtoTrackMap",
                      'InDetSegmentPRDtoTrackMap' if InputCollections is not None else "")
    kwargs.setdefault("MinTRTonSegment", flags.Tracking.BackTracking.minTRT)
    kwargs.setdefault("MinTRTonly", flags.Tracking.BackTracking.minTRT)
    kwargs.setdefault("TrtExtension", True)
    kwargs.setdefault("SiExtensionCuts", flags.Tracking.BackTracking.SiExtensionCuts)
    kwargs.setdefault("minPt", flags.Tracking.BackTracking.minPt)
    kwargs.setdefault("maxRPhiImp", flags.Tracking.BackTracking.maxSecondaryImpact)
    kwargs.setdefault("maxZImp", flags.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("maxEta", flags.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("RejectShortExtension",
                      flags.Tracking.BackTracking.rejectShortExtensions)
    kwargs.setdefault("OutputSegments", False)

    if flags.Tracking.BackTracking.doRoISeeded:
        from RegionSelector.RegSelToolConfig import regSelTool_SCT_Cfg
        RegSelTool_SCT = acc.popToolsAndMerge(regSelTool_SCT_Cfg(flags))
        acc.addPublicTool(RegSelTool_SCT)

        kwargs.setdefault("RegSelTool", RegSelTool_SCT)
        kwargs.setdefault("CaloSeededRoI", True)
        kwargs.setdefault("EMROIPhiRZContainer", (
            "InDetCaloClusterROIPhiRZ%.0fGeVBackTracking" %
            (flags.Tracking.BackTracking.minRoIClusterEt/Units.GeV)))

    acc.addEventAlgo(CompFactory.InDet.TRT_SeededTrackFinder(name, **kwargs))
    return acc
