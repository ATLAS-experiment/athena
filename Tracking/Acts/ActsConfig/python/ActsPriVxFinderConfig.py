# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetPriVxFinderTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def ActsGaussAdaptiveMultiFindingCfg(flags,
                                     name="ActsAdaptiveMultiPriVtxFinderTool",
                                     **kwargs):
    acc = ComponentAccumulator()

    if "TrackSelector" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            VtxInDetTrackSelectionCfg)
        kwargs.setdefault("TrackSelector", acc.popToolsAndMerge(
            VtxInDetTrackSelectionCfg(flags)))

    if "TrackingGeometryTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault("TrackingGeometryTool", acc.popToolsAndMerge(
            ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle

    if "ExtrapolationTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault("ExtrapolationTool", acc.popToolsAndMerge(
            ActsExtrapolationToolCfg(flags))) # PrivateToolHandle

    kwargs.setdefault("useBeamConstraint",
                      flags.Tracking.PriVertex.useBeamConstraint)
    kwargs.setdefault("tracksMaxZinterval",
                      flags.Tracking.PriVertex.maxZinterval)
    kwargs.setdefault("doFullSplitting",
                      not flags.Tracking.PriVertex.useBeamConstraint)

    if flags.GeoModel.Run >= LHCPeriod.Run4:
        kwargs.setdefault("minWeight", 0.02)
        kwargs.setdefault("maxIterations", 200)

    acc.setPrivateTools(
        CompFactory.ActsTrk.AdaptiveMultiPriVtxFinderTool(name, **kwargs))
    return acc

def TrigActsGaussAdaptiveMultiFindingCfg(
        flags,
        name="ActsAdaptiveMultiPriVtxFinderTool",
        **kwargs):
    acc = ComponentAccumulator()

    if "TrackSelector" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            TrigVtxInDetTrackSelectionCfg)
        kwargs.setdefault("TrackSelector", acc.popToolsAndMerge(
            TrigVtxInDetTrackSelectionCfg(flags)))

    kwargs.setdefault("useBeamConstraint", True)
    kwargs.setdefault("useSeedConstraint", False)
    kwargs.setdefault("tracksMaxZinterval", flags.Tracking.ActiveConfig.TracksMaxZinterval)
    kwargs.setdefault("doFullSplitting", False)
    kwargs.setdefault("addSingleTrackVertices", flags.Tracking.ActiveConfig.addSingleTrackVertices)

    acc.setPrivateTools(acc.popToolsAndMerge(
        ActsGaussAdaptiveMultiFindingCfg(flags, name+flags.Tracking.ActiveConfig.input_name, **kwargs)))
    return acc

def ActsIterativeFindingCfg(flags,
                            name="ActsIterativePriVtxFinderTool",
                            **kwargs):
    acc = ComponentAccumulator()

    if "TrackSelector" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            VtxInDetTrackSelectionCfg)
        kwargs.setdefault("TrackSelector", acc.popToolsAndMerge(
            VtxInDetTrackSelectionCfg(flags)))

    if "TrackingGeometryTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault("TrackingGeometryTool", acc.popToolsAndMerge(
            ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle

    if "ExtrapolationTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault("ExtrapolationTool", acc.popToolsAndMerge(
            ActsExtrapolationToolCfg(flags))) # PrivateToolHandle

    kwargs.setdefault("useBeamConstraint",
                      flags.Tracking.PriVertex.useBeamConstraint)
    kwargs.setdefault("significanceCutSeeding", 12)
    kwargs.setdefault("maximumChi2cutForSeeding", 49)
    kwargs.setdefault("maxVertices", flags.Tracking.PriVertex.maxVertices)
    kwargs.setdefault("doMaxTracksCut", flags.Tracking.PriVertex.doMaxTracksCut)
    kwargs.setdefault("maxTracks", flags.Tracking.PriVertex.maxTracks)

    acc.setPrivateTools(
        CompFactory.ActsTrk.IterativePriVtxFinderTool(name, **kwargs))
    return acc
