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

    if "TRTCalDbTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import (
            TRT_CalDbToolCfg)
        kwargs.setdefault("TRTCalDbTool", acc.popToolsAndMerge(
            TRT_CalDbToolCfg(flags)))

    if "TRTStrawSummaryTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import (
            TRT_StrawStatusSummaryToolCfg)
        kwargs.setdefault("TRTStrawSummaryTool", acc.popToolsAndMerge(
            TRT_StrawStatusSummaryToolCfg(flags)))

    if "NeighbourSvc" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import (
            TRT_StrawNeighbourSvcCfg)
        kwargs.setdefault("NeighbourSvc", acc.getPrimaryAndMerge(
            TRT_StrawNeighbourSvcCfg(flags)))

    if "TRTDriftFunctionTool" not in kwargs:
        from InDetConfig.TRT_DriftFunctionToolConfig import (
            TRT_DriftFunctionToolCfg)
        kwargs.setdefault("TRTDriftFunctionTool", acc.popToolsAndMerge(
            TRT_DriftFunctionToolCfg(flags)))
    
    if flags.Output.HISTFileName:
        kwargs.setdefault("NtupleName", flags.Output.HISTFileName)
    
    acc.setPrivateTools(CompFactory.FillAlignTRTHits(name, **kwargs))
    
    return acc


# Tool to refit tracks
def FitToolCfg(flags, name = "FitToolCfg" ,**kwargs):
    acc = ComponentAccumulator()  
    acc.setPrivateTools(CompFactory.FitTool(name, **kwargs))
    return acc
