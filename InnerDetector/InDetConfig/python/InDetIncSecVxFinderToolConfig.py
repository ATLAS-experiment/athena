# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# Define method to construct configures Sec Vtx Finder alg
# attempted by N Ribaric (@LancasterUNI) neza.ribaric@cern.ch


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.Constants as Lvl


def InDetIterativeSecVtxFinderToolCfg(flags, name="InDetIterativeSecVtxFinderTool", **kwargs):


  acc = ComponentAccumulator()
  
  from TrkConfig.TrkVertexSeedFinderToolsConfig import IndexedCrossDistancesSeedFinderCfg
  kwargs.setdefault("SeedFinder",acc.popToolsAndMerge(IndexedCrossDistancesSeedFinderCfg(flags)))
 
  from TrkConfig.TrkVertexFittersConfig import AdaptiveVxFitterToolIncSecVtxCfg
  kwargs.setdefault("VertexFitterTool",acc.popToolsAndMerge(AdaptiveVxFitterToolIncSecVtxCfg(flags)))

  from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
    InDetTrackSelectionTool_AMSVF_Cfg)
  kwargs.setdefault("BaseTrackSelector",acc.popToolsAndMerge(InDetTrackSelectionTool_AMSVF_Cfg(flags)))

  from InDetConfig.InDetSecVtxTrackSelectionToolConfig import InDetSecVtxTrackSelectionToolCfg
  kwargs.setdefault("SecVtxTrackSelector",acc.popToolsAndMerge(InDetSecVtxTrackSelectionToolCfg(flags)))

  from TrkConfig.TrkVertexFitterUtilsConfig import AtlasImpactPoint3dEstimatorCfg
  kwargs.setdefault("ImpactPoint3dEstimator",acc.popToolsAndMerge(AtlasImpactPoint3dEstimatorCfg(flags)))

  from TrkConfig.TrkVertexFitterUtilsConfig import FullLinearizedTrackFactoryCfg
  kwargs.setdefault("LinearizedTrackFactory",acc.popToolsAndMerge(FullLinearizedTrackFactoryCfg(flags)))

  kwargs.setdefault("doMaxTracksCut",flags.Tracking.PriVertex.doMaxTracksCut)
  kwargs.setdefault("MaxTracks",flags.Tracking.PriVertex.maxTracks)

  kwargs["VertexFilterLevel"] = 0
  kwargs.setdefault("OutputLevel",Lvl.INFO)

  acc.setPrivateTools(CompFactory.InDet.InDetIterativeSecVtxFinderTool(name, **kwargs))
  return acc
