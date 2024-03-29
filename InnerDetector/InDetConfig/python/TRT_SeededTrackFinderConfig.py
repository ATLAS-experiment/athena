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
    kwargs.setdefault("SiExtensionCuts",
                      flags.Tracking.ActiveConfig.SiExtensionCuts)
    kwargs.setdefault("minPt", flags.Tracking.ActiveConfig.minSecondaryPt)
    kwargs.setdefault("maxRPhiImp",
                      flags.Tracking.ActiveConfig.maxSecondaryImpact)
    kwargs.setdefault("maxZImp", flags.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("maxEta", flags.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("RejectShortExtension",
                      flags.Tracking.ActiveConfig.rejectShortExtensions)
    kwargs.setdefault("FinalRefit", False)
    kwargs.setdefault("FinalStatistics", False)
    kwargs.setdefault("OutputSegments", False)
    kwargs.setdefault("InputSegmentsLocation", 'TRTSegments')
    kwargs.setdefault("OutputTracksLocation", 'TRTSeededTracks')

    if flags.Tracking.ActiveConfig.RoISeededBackTracking:
        from RegionSelector.RegSelToolConfig import regSelTool_SCT_Cfg
        RegSelTool_SCT   = acc.popToolsAndMerge(regSelTool_SCT_Cfg(flags))
        acc.addPublicTool(RegSelTool_SCT)

        kwargs.setdefault("RegSelTool", RegSelTool_SCT)
        kwargs.setdefault("CaloSeededRoI", True)
        kwargs.setdefault("EMROIPhiRZContainer", (
            "InDetCaloClusterROIPhiRZ%.0fGeVUnordered" % 
            (flags.Tracking.ActiveConfig.minRoIClusterEt/Units.GeV)))

    acc.addEventAlgo(CompFactory.InDet.TRT_SeededTrackFinder(name, **kwargs))
    return acc
