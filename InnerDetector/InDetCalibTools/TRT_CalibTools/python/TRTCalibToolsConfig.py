"""
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


# Tool to write a track-tuple with TRT hit info
def FillAlignTrkInfoCfg(flags,name='FillAlignTrkInfo',**kwargs) :
    acc = ComponentAccumulator()
    
    from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
    kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(InDetTrackSummaryToolCfg(flags)))
    
    acc.setPrivateTools(CompFactory.FillAlignTrkInfo(name, **kwargs))
    return acc


# Tool to write a hit-tuple with R-t info  
def FillAlignTRTHitsCfg(flags,name='FillAlignTRTHits',**kwargs) :
    acc = ComponentAccumulator()
    
    kwargs.setdefault("minTimebinsOverThreshold", 0)
    
    from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_CalDbToolCfg, TRT_StrawStatusSummaryToolCfg, TRT_StrawNeighbourSvcCfg
    kwargs.setdefault("TRTCalDbTool", acc.popToolsAndMerge(TRT_CalDbToolCfg(flags)))
    kwargs.setdefault("TRTStrawSummaryTool", acc.popToolsAndMerge(TRT_StrawStatusSummaryToolCfg(flags)))
    kwargs.setdefault("NeighbourSvc", acc.getPrimaryAndMerge(TRT_StrawNeighbourSvcCfg(flags)))
    
    acc.setPrivateTools(CompFactory.FillAlignTRTHits(name, **kwargs))
    
    return acc


# Tool to refit tracks
def FitToolCfg(flags, name = "FitToolCfg" ,**kwargs):
    acc = ComponentAccumulator()  
    acc.setPrivateTools(CompFactory.FitTool(name, **kwargs))
    return acc
