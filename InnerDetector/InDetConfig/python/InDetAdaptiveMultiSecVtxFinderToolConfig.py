# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Define method to configure AMVF
# attempted by N Ribaric (@LancasterUNI) neza.ribaric@cern.ch


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def InDetAdaptiveMultiSecVtxFinderToolCfg(
    flags, name="InDetAdaptiveMultiSecVtxFinderTool", **kwargs):
  acc = ComponentAccumulator()

  from TrkConfig.TrkVertexFittersConfig import AdaptiveMultiSecVertexFitterCfg
  kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(
    AdaptiveMultiSecVertexFitterCfg(flags)))

  from InDetConfig.InDetTrackSelectionToolConfig import (
    InDetTrackSelectionTool_AMSVF_Cfg)
  kwargs.setdefault("BaseTrackSelector",acc.popToolsAndMerge(
    InDetTrackSelectionTool_AMSVF_Cfg(flags)))

  from InDetConfig.InDetSecVtxTrackSelectionToolConfig import (
    InDetSecVtxTrackSelectionToolCfg)
  kwargs.setdefault("SecVtxTrackSelector",acc.popToolsAndMerge(
    InDetSecVtxTrackSelectionToolCfg(flags)))

  from TrkConfig.TrkVertexSeedFinderToolsConfig import (
    IndexedCrossDistancesSeedFinderCfg)
  kwargs.setdefault("SeedFinder",acc.popToolsAndMerge(
    IndexedCrossDistancesSeedFinderCfg(flags)))

  from TrkConfig.TrkVertexFitterUtilsConfig import (
    AtlasImpactPoint3dEstimatorCfg)
  kwargs.setdefault("ImpactPoint3dEstimator", acc.popToolsAndMerge(
    AtlasImpactPoint3dEstimatorCfg(flags)))

  acc.setPrivateTools(
    CompFactory.InDet.InDetAdaptiveMultiSecVtxFinderTool(name, **kwargs))
  return acc
