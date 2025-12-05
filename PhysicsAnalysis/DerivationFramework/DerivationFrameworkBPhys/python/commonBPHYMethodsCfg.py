# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName, **kwargs):
    from TrkConfig.TrkVKalVrtFitterConfig import BPHY_TrkVKalVrtFitterCfg \
        as TrkVKalVrtFitterCfg
    return TrkVKalVrtFitterCfg(flags, name=BPHYDerivationName+"_VKalVrtFitter", **kwargs)

def BPHY_V0ToolCfg(flags, BPHYDerivationName):
    from TrkConfig.TrkVertexAnalysisUtilsConfig import V0ToolsCfg
    return V0ToolsCfg(flags, BPHYDerivationName+"_V0Tools")

def BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName):
    from InDetConfig.InDetConversionFinderToolsConfig import \
        BPHY_VertexPointEstimatorCfg as VertexPointEstimatorCfg
    return VertexPointEstimatorCfg(flags, BPHYDerivationName+"_VtxPointEstimator")

def BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName):
    from InDetConfig.InDetTrackSelectorToolConfig import \
        BPHY_InDetDetailedTrackSelectorToolCfg as \
        InDetDetailedTrackSelectorToolCfg
    return InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName+"_InDetDetailedTrackSelectorTool")

def Thin_vtxTrkCfg(flags, name, **kwargs):
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.Thin_vtxTrk(name, **kwargs))
    return acc

def AugOriginalCountsCfg(flags, name="AugOriginalCounts", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("TrackLRTContainer",
                      "InDetLargeD0TrackParticles"
                      if flags.Tracking.doLargeD0 else "")
    acc.setPrivateTools(
        CompFactory.DerivationFramework.AugOriginalCounts(name, **kwargs))
    return acc

def getDefaultAllVariables():
    return ["EventInfo"]
