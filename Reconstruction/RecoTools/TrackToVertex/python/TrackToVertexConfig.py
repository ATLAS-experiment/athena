# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of TrackToVertex package
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def TrackToVertexCfg(flags, name="AtlasTrackToVertexTool", **kwargs):
    result = ComponentAccumulator()
    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    kwargs.setdefault("StartTRTStandaloneTracksAtOriginalPerigee",
                      flags.Tracking.TRTStandalone.startAtOriginalPerigee)
    result.setPrivateTools(CompFactory.Reco.TrackToVertex(name, **kwargs))
    return result

def InDetTrackToVertexCfg(flags, name='InDetTrackToVertex', **kwargs):
    result = ComponentAccumulator()
    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    kwargs.setdefault("StartTRTStandaloneTracksAtOriginalPerigee",
                      flags.Tracking.TRTStandalone.startAtOriginalPerigee)
    result.setPrivateTools(CompFactory.Reco.TrackToVertex(name, **kwargs))
    return result
