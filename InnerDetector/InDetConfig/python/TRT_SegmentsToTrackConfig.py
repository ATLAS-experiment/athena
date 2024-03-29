# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_SegmentsToTrack package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TRT_Cosmics_SegmentsToTrackCfg(flags, name ='InDetTRT_Cosmics_SegmentsToTrack', **kwargs):
    acc = ComponentAccumulator()

    if "ExtrapolationTool" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("ExtrapolationTool", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    if "TrackFitter" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import InDetTrackFitterCfg
        kwargs.setdefault("TrackFitter", acc.popToolsAndMerge(
            InDetTrackFitterCfg(flags)))

    if "SummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        kwargs.setdefault("SummaryTool", acc.popToolsAndMerge(
            InDetTrackSummaryToolCfg(flags)))

    if ("AssociationTool" not in kwargs
        and "InputAssociationMapName" in kwargs
        and kwargs["InputAssociationMapName"] != ''):
        from InDetConfig.InDetAssociationToolsConfig import (
            InDetPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            InDetPRDtoTrackMapToolGangedPixelsCfg(flags)))

    kwargs.setdefault("MinNHit", flags.Tracking.TRTStandalone.minTRT)
    kwargs.setdefault("OutlierRemoval", True)
    kwargs.setdefault("MaterialEffects", False)

    acc.addEventAlgo(CompFactory.InDet.TRT_SegmentsToTrack(name, **kwargs))
    return acc
