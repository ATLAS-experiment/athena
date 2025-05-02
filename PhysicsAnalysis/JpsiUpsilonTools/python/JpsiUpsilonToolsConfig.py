# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PrimaryVertexRefittingToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    from TrkConfig.TrkVertexFitterUtilsConfig import TrackToVertexIPEstimatorCfg
    kwargs.setdefault( "TrackToVertexIPEstimator", acc.popToolsAndMerge( TrackToVertexIPEstimatorCfg(flags,**kwargs) ) )
    acc.setPrivateTools( CompFactory.Analysis.PrimaryVertexRefitter( **kwargs) )
    return acc


#A setup with nice friendly defaults
def JpsiFinderCfg(flags,name="JpsiFinder", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("useV0Fitter", False)
    kwargs.setdefault("V0VertexFitterTool", None)
    if "TrkVertexFitterTool" not in kwargs:
        from TrkConfig.TrkVKalVrtFitterConfig import BPHY_TrkVKalVrtFitterCfg
        kwargs.setdefault("TrkVertexFitterTool", acc.addPublicTool(acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags))))
    if "TrackSelectorTool" not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import BPHY_InDetDetailedTrackSelectorToolCfg
        kwargs.setdefault("TrackSelectorTool", acc.addPublicTool(acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags))))
    if "VertexPointEstimator" not in kwargs:
        from InDetConfig.InDetConversionFinderToolsConfig import BPHY_VertexPointEstimatorCfg
        kwargs.setdefault("VertexPointEstimator", acc.addPublicTool(acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags))))
    if "PartPropSvc" not in kwargs:
        from AthenaServices.PartPropSvcConfig import PartPropSvcCfg
        kwargs.setdefault("PartPropSvc", acc.getPrimaryAndMerge(PartPropSvcCfg(flags)))
    acc.setPrivateTools(CompFactory.Analysis.JpsiFinder(name, **kwargs))
    return acc


def JpsiAlgCfg(flags, name="JpsiAlg", **kwargs):
   acc = ComponentAccumulator()
   if "JpsiFinderName" not in kwargs:
       kwargs.setdefault("JpsiFinderName", acc.popToolsAndMerge(JpsiFinderCfg(flags)))
   acc.addEventAlgo(CompFactory.JpsiAlg(name, **kwargs))
   return acc
