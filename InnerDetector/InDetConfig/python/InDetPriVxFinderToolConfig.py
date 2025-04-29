# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetPriVxFinderTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrkConfig.VertexFindingFlags import VertexSetup


def IterativeFindingBaseCfg(
        flags, name="InDetIterativePriVxFinderTool", **kwargs):
    acc = ComponentAccumulator()

    if "SeedFinder" not in kwargs:
        if flags.Tracking.PriVertex.useBeamConstraint:
            from TrkConfig.TrkVertexSeedFinderToolsConfig import ZScanSeedFinderCfg
            kwargs.setdefault("SeedFinder", acc.popToolsAndMerge(
                ZScanSeedFinderCfg(flags)))
        else:
            from TrkConfig.TrkVertexSeedFinderToolsConfig import (
                IVF_CrossDistancesSeedFinderCfg)
            kwargs.setdefault("SeedFinder", acc.popToolsAndMerge(
                IVF_CrossDistancesSeedFinderCfg(flags)))

    if "TrackSelector" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            VtxInDetTrackSelectionCfg)
        kwargs.setdefault("TrackSelector", acc.popToolsAndMerge(
            VtxInDetTrackSelectionCfg(flags)))

    if "LinearizedTrackFactory" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import (
            FullLinearizedTrackFactoryCfg)
        kwargs.setdefault("LinearizedTrackFactory", acc.popToolsAndMerge(
            FullLinearizedTrackFactoryCfg(flags)))

    if "ImpactPoint3dEstimator" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import (
            ImpactPoint3dEstimatorCfg)
        kwargs.setdefault("ImpactPoint3dEstimator", acc.popToolsAndMerge(
            ImpactPoint3dEstimatorCfg(flags)))

    if "VertexFitterTool" not in kwargs:
        from TrkConfig.TrkVertexFittersConfig import AdaptiveVertexFitterCfg
        kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(
            AdaptiveVertexFitterCfg(
                flags,
                SeedFinder=kwargs["SeedFinder"],
                LinearizedTrackFactory=kwargs["LinearizedTrackFactory"],
                ImpactPoint3dEstimator=kwargs["ImpactPoint3dEstimator"])))

    kwargs.setdefault("useBeamConstraint",
                      flags.Tracking.PriVertex.useBeamConstraint)
    kwargs.setdefault("significanceCutSeeding", 12)
    kwargs.setdefault("maximumChi2cutForSeeding", 49)
    kwargs.setdefault("maxVertices", flags.Tracking.PriVertex.maxVertices)
    kwargs.setdefault("doMaxTracksCut",
                      flags.Tracking.PriVertex.doMaxTracksCut)
    kwargs.setdefault("MaxTracks", flags.Tracking.PriVertex.maxTracks)

    acc.setPrivateTools(
        CompFactory.InDet.InDetIterativePriVxFinderTool(name, **kwargs))
    return acc


def FastIterativeFindingCfg(
        flags, name="FastInDetIterativePriVxFinderTool", **kwargs):
    acc = ComponentAccumulator()

    if "VertexFitterTool" not in kwargs:
        from TrkConfig.TrkVertexBilloirToolsConfig import FastVertexFitterCfg
        kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(
            FastVertexFitterCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(
        IterativeFindingBaseCfg(flags, name, **kwargs)))
    return acc


def GaussIterativeFindingCfg(
        flags, name="GaussInDetIterativePriVxFinderTool", **kwargs):
    acc = ComponentAccumulator()

    if "SeedFinder" not in kwargs:
        from TrkConfig.TrkVertexSeedFinderToolsConfig import (
            TrackDensitySeedFinderCfg)
        kwargs.setdefault("SeedFinder", acc.popToolsAndMerge(
            TrackDensitySeedFinderCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(
        IterativeFindingBaseCfg(flags, name, **kwargs)))
    return acc


def TrigGaussIterativeFindingCfg(
        flags, name="InDetTrigPriVxFinderTool", **kwargs):
    acc = ComponentAccumulator()

    if "VertexFitterTool" not in kwargs:
        from TrkConfig.TrkVertexBilloirToolsConfig import FastVertexFitterCfg
        kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(
            FastVertexFitterCfg(flags)))

    if "TrackSelector" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            TrigVtxInDetTrackSelectionCfg)
        kwargs.setdefault("TrackSelector", acc.popToolsAndMerge(
            TrigVtxInDetTrackSelectionCfg(flags)))

    kwargs.setdefault("useBeamConstraint", True)
    kwargs.setdefault("maximumChi2cutForSeeding", 29)
    kwargs.setdefault("createSplitVertices", False)
    kwargs.setdefault("doMaxTracksCut", True)
    kwargs.setdefault("MaxTracks", 3000)

    acc.setPrivateTools(acc.popToolsAndMerge(
        GaussIterativeFindingCfg(
            flags, name+flags.Tracking.ActiveConfig.input_name, **kwargs)))
    return acc


def VertexFinderToolCfg(flags, **kwargs):

    if flags.Tracking.PriVertex.setup == VertexSetup.IVF:
        return IterativeFindingBaseCfg(flags, **kwargs)
    elif flags.Tracking.PriVertex.setup == VertexSetup.FastIVF:
        return FastIterativeFindingCfg(flags, **kwargs)
    elif flags.Tracking.PriVertex.setup == VertexSetup.ActsGaussAMVF:
        from ActsConfig.ActsPriVxFinderConfig import ActsGaussAdaptiveMultiFindingCfg
        return ActsGaussAdaptiveMultiFindingCfg(flags, **kwargs)
    elif flags.Tracking.PriVertex.setup == VertexSetup.ExperimentalActsIVF:
        from ActsConfig.ActsPriVxFinderConfig import (
            ActsIterativeFindingCfg)
        return ActsIterativeFindingCfg(flags, **kwargs)

    elif flags.Tracking.PriVertex.setup == VertexSetup.ActsGridDensity:
        from ActsConfig.ActsPriVxFinderConfig import ActsGridAdaptiveMultiFindingCfg
        return ActsGridAdaptiveMultiFindingCfg(flags, **kwargs)


def TrigVertexFinderToolCfg(flags, **kwargs):
    if flags.Tracking.ActiveConfig.adaptiveVertex:
        from ActsConfig.ActsPriVxFinderConfig import (
            TrigActsGaussAdaptiveMultiFindingCfg)
        return TrigActsGaussAdaptiveMultiFindingCfg(flags, **kwargs)
    else:
        return TrigGaussIterativeFindingCfg(flags, **kwargs)

