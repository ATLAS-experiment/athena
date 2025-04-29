# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Configuration of common interface with various track fitters
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TrkConfig.TrkConfigFlags import TrackFitterType

#########################
##### InDet configs #####
#########################

def InDetTrackFitterCfg(flags, name='InDetTrackFitter', **kwargs) :
    from TrkConfig.TrkDistributedKalmanFilterConfig import (
        DistributedKalmanFilterCfg)
    from TrkConfig.TrkGlobalChi2FitterConfig import InDetGlobalChi2FitterCfg
    from TrkConfig.TrkGaussianSumFilterConfig import GaussianSumFitterCfg
    return {
        TrackFitterType.DistributedKalmanFilter : DistributedKalmanFilterCfg,
        TrackFitterType.GlobalChi2Fitter        : InDetGlobalChi2FitterCfg,
        TrackFitterType.GaussianSumFilter       : GaussianSumFitterCfg
    }[flags.Tracking.trackFitterType](flags, name, **kwargs)

# Config to be called outside of loops over tracking passes in main reco
# Will configure subtools based on MainPass
def InDetStandaloneTrackFitterCfg(flags, name='InDetTrackFitter', **kwargs) :
    primaryFlags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        f"Tracking.{flags.Tracking.PrimaryPassConfig.value}Pass")
    return InDetTrackFitterCfg(primaryFlags, name, **kwargs)

def InDetTrackFitterHoleSearchCfg(flags, name='InDetTrackFitterHoleSearch', **kwargs) :
    acc = ComponentAccumulator()

    if "BoundaryCheckTool" not in kwargs:
        from InDetConfig.InDetBoundaryCheckToolConfig import (
            InDetBoundaryCheckToolCfg)
        kwargs.setdefault("BoundaryCheckTool", acc.popToolsAndMerge(
            InDetBoundaryCheckToolCfg(flags)))

    kwargs.setdefault("DoHoleSearch", True)

    acc.setPrivateTools(acc.popToolsAndMerge(
        InDetTrackFitterCfg(flags, name, **kwargs)))
    return acc

def InDetTrackFitterAmbiCfg(flags, name='InDetTrackFitterAmbi', **kwargs) :
    acc = ComponentAccumulator()

    InDetTrackFitter = acc.popToolsAndMerge(
        InDetTrackFitterHoleSearchCfg(flags, name, **kwargs))
    ClusterSplitProbabilityName = ("InDetAmbiguityProcessorSplitProb" +
                                   flags.Tracking.ActiveConfig.extension)

    if flags.Tracking.trackFitterType==TrackFitterType.DistributedKalmanFilter:
        InDetTrackFitter.RecalibratorHandle.BroadPixelClusterOnTrackTool.ClusterSplitProbabilityName = ClusterSplitProbabilityName

    elif flags.Tracking.trackFitterType==TrackFitterType.GlobalChi2Fitter:
        InDetTrackFitter.ClusterSplitProbabilityName = ClusterSplitProbabilityName
        InDetTrackFitter.RotCreatorTool.ToolPixelCluster.ClusterSplitProbabilityName = ClusterSplitProbabilityName
        InDetTrackFitter.BroadRotCreatorTool.ToolPixelCluster.ClusterSplitProbabilityName = ClusterSplitProbabilityName

    elif flags.Tracking.trackFitterType==TrackFitterType.GaussianSumFilter:
        InDetTrackFitter.ToolForROTCreation.ToolPixelCluster.ClusterSplitProbabilityName = ClusterSplitProbabilityName

    acc.setPrivateTools(InDetTrackFitter)
    return acc

def InDetTrackFitterTRTCfg(flags, name='InDetTrackFitterTRT', **kwargs) :
    if flags.Tracking.trackFitterType==TrackFitterType.GlobalChi2Fitter:
        from TrkConfig.TrkGlobalChi2FitterConfig import (
            InDetGlobalChi2FitterTRTCfg)
        TrackFitterTRTCfg = InDetGlobalChi2FitterTRTCfg
    else:
        TrackFitterTRTCfg = InDetTrackFitterCfg
    return TrackFitterTRTCfg(flags, name, **kwargs)

def InDetTrackFitterLowPtCfg(flags, name='InDetTrackFitter', **kwargs) :
    if flags.Tracking.trackFitterType==TrackFitterType.GlobalChi2Fitter:
        from TrkConfig.TrkGlobalChi2FitterConfig import (
            InDetGlobalChi2FitterLowPtCfg)
        TrackFitterLowPtCfg = InDetGlobalChi2FitterLowPtCfg
    else:
        TrackFitterLowPtCfg = InDetTrackFitterCfg
    return TrackFitterLowPtCfg(flags, name, **kwargs)

def InDetTrackFitterLowPtHoleSearchCfg(flags, name='InDetTrackFitterHoleSearch', **kwargs) :
    acc = ComponentAccumulator()

    if "BoundaryCheckTool" not in kwargs:
        from InDetConfig.InDetBoundaryCheckToolConfig import (
            InDetBoundaryCheckToolCfg)
        kwargs.setdefault("BoundaryCheckTool", acc.popToolsAndMerge(
            InDetBoundaryCheckToolCfg(flags)))

    kwargs.setdefault("DoHoleSearch", True)

    acc.setPrivateTools(acc.popToolsAndMerge(
        InDetTrackFitterLowPtCfg(flags, name, **kwargs)))
    return acc

def InDetTrackFitterLowPtAmbiCfg(flags, name='InDetTrackFitterAmbi', **kwargs) :
    acc = ComponentAccumulator()

    if flags.Tracking.trackFitterType == TrackFitterType.GlobalChi2Fitter:
        from TrkConfig.TrkGlobalChi2FitterConfig import (
            InDetGlobalChi2FitterLowPtCfg)
        InDetGlobalChi2FitterLowPt = acc.popToolsAndMerge(
            InDetGlobalChi2FitterLowPtCfg(flags, name, **kwargs))

        ClusterSplitProbabilityName = ("InDetAmbiguityProcessorSplitProb" +
                                       flags.Tracking.ActiveConfig.extension)
        InDetGlobalChi2FitterLowPt.RotCreatorTool.ToolPixelCluster.ClusterSplitProbabilityName = ClusterSplitProbabilityName
        InDetGlobalChi2FitterLowPt.BroadRotCreatorTool.ToolPixelCluster.ClusterSplitProbabilityName = ClusterSplitProbabilityName
        acc.setPrivateTools(InDetGlobalChi2FitterLowPt)

    else:
        acc.setPrivateTools(acc.popToolsAndMerge(
            InDetTrackFitterAmbiCfg(flags, name, **kwargs)))

    return acc

def InDetTrackFitterBTCfg(flags, name='InDetTrackFitterBT', **kwargs) :
    if flags.Tracking.trackFitterType==TrackFitterType.GlobalChi2Fitter:
        from TrkConfig.TrkGlobalChi2FitterConfig import (
            InDetGlobalChi2FitterBTCfg)
        TrackFitterBTCfg = InDetGlobalChi2FitterBTCfg
    else:
        TrackFitterBTCfg = InDetTrackFitterCfg
    return TrackFitterBTCfg(flags, name, **kwargs)


#########################
#####  ITk configs  #####
#########################

def ITkTrackFitterCfg(flags, name='ITkTrackFitter', **kwargs) :
    from TrkConfig.TrkDistributedKalmanFilterConfig import (
        ITkDistributedKalmanFilterCfg)
    from TrkConfig.TrkGlobalChi2FitterConfig import ITkGlobalChi2FitterCfg
    from TrkConfig.TrkGaussianSumFilterConfig import ITkGaussianSumFitterCfg
    return {
        TrackFitterType.DistributedKalmanFilter : ITkDistributedKalmanFilterCfg,
        TrackFitterType.GlobalChi2Fitter        : ITkGlobalChi2FitterCfg,
        TrackFitterType.GaussianSumFilter       : ITkGaussianSumFitterCfg
    }[flags.Tracking.trackFitterType](flags, name, **kwargs)

# Config to be called outside of loops over tracking passes in main reco
# Will configure subtools based on MainPass
def ITkStandaloneTrackFitterCfg(flags, name='ITkTrackFitter', **kwargs) :
    primaryFlags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        f"Tracking.{flags.Tracking.ITkPrimaryPassConfig.value}Pass")
    return ITkTrackFitterCfg(primaryFlags, name, **kwargs)

def ITkTrackFitterAmbiCfg(flags, name='ITkTrackFitterAmbi', **kwargs) :
    acc = ComponentAccumulator()

    if "BoundaryCheckTool" not in kwargs:
        from InDetConfig.InDetBoundaryCheckToolConfig import (
            ITkBoundaryCheckToolCfg)
        kwargs.setdefault("BoundaryCheckTool", acc.popToolsAndMerge(
            ITkBoundaryCheckToolCfg(flags)))

    kwargs.setdefault("DoHoleSearch", True)

    ITkTrackFitter = acc.popToolsAndMerge(
        ITkTrackFitterCfg(flags, name, **kwargs))
    ClusterSplitProbabilityName = ("ITkAmbiguityProcessorSplitProb" +
                                   flags.Tracking.ActiveConfig.extension)

    if flags.Tracking.trackFitterType==TrackFitterType.DistributedKalmanFilter:
        ITkTrackFitter.RecalibratorHandle.BroadPixelClusterOnTrackTool.ClusterSplitProbabilityName = ClusterSplitProbabilityName

    elif flags.Tracking.trackFitterType==TrackFitterType.GlobalChi2Fitter:
        ITkTrackFitter.ClusterSplitProbabilityName = ClusterSplitProbabilityName
        ITkTrackFitter.RotCreatorTool.ToolPixelCluster.ClusterSplitProbabilityName = ClusterSplitProbabilityName
        ITkTrackFitter.BroadRotCreatorTool.ToolPixelCluster.ClusterSplitProbabilityName = ClusterSplitProbabilityName

    elif flags.Tracking.trackFitterType==TrackFitterType.GaussianSumFilter:
        ITkTrackFitter.ToolForROTCreation.ToolPixelCluster.ClusterSplitProbabilityName = ClusterSplitProbabilityName

    acc.setPrivateTools(ITkTrackFitter)
    return acc
