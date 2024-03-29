# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_SegmentsToTrackTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TRT_Standalone_SegmentToTrackToolCfg(flags, name ='InDetTRT_Standalone_SegmentToTrackTool', **kwargs):
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = AtlasFieldCacheCondAlgCfg(flags)

    if "AssociationTool" not in kwargs:
        if flags.Tracking.ActiveConfig.usePrdAssociationTool:
            from InDetConfig.InDetAssociationToolsConfig import (
                InDetPRDtoTrackMapToolGangedPixelsCfg)
            asso_tool = acc.popToolsAndMerge(
                InDetPRDtoTrackMapToolGangedPixelsCfg(flags))
        else:
            asso_tool = None
        kwargs.setdefault("AssociationTool", asso_tool)

    if "RefitterTool" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import InDetTrackFitterTRTCfg
        kwargs.setdefault("RefitterTool", acc.popToolsAndMerge(
            InDetTrackFitterTRTCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(
            InDetTrackSummaryToolCfg(flags)))

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        InDetExtrapolator = acc.popToolsAndMerge(InDetExtrapolatorCfg(flags))
        acc.addPublicTool(InDetExtrapolator)
        kwargs.setdefault("Extrapolator", InDetExtrapolator)

    if "ScoringTool" not in kwargs:
        from InDetConfig.InDetTrackScoringToolsConfig import (
            InDetTRT_StandaloneScoringToolCfg)
        ScoringTool = acc.popToolsAndMerge(
            InDetTRT_StandaloneScoringToolCfg(flags))
        acc.addPublicTool(ScoringTool)    
        kwargs.setdefault("ScoringTool", ScoringTool)

    kwargs.setdefault("FinalRefit", True)
    kwargs.setdefault("MaxSharedHitsFraction", 0.7)
    kwargs.setdefault("SuppressHoleSearch", True)

    acc.setPrivateTools(CompFactory.InDet.TRT_SegmentToTrackTool(name, **kwargs))
    return acc


def TRT_TrackSegment_SegmentToTrackToolCfg(flags, name ='InDetTRT_TrackSegment_SegmentToTrackTool',  **kwargs):
    acc = ComponentAccumulator()

    if "ScoringTool" not in kwargs:
        from InDetConfig.InDetTrackScoringToolsConfig import (
            InDetTRT_TrackSegmentScoringToolCfg)
        ScoringTool = acc.popToolsAndMerge(
            InDetTRT_TrackSegmentScoringToolCfg(flags))
        acc.addPublicTool(ScoringTool)    
        kwargs.setdefault("ScoringTool", ScoringTool)

    acc.setPrivateTools(acc.popToolsAndMerge(
        TRT_Standalone_SegmentToTrackToolCfg(flags, name, **kwargs)))
    return acc
