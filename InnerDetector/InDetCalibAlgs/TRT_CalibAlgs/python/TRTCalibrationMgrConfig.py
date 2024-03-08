"""Define methods to construct a configured TRT R-t calibration algorithm

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Tool to process R-t ntuple. Produces histograms and calibration text files.
def TRTCalibratorCfg(flags, name="TRTCalibrator", **kwargs) :
    acc = ComponentAccumulator()
    kwargs.setdefault("MinRt",500)
    kwargs.setdefault("MinT0",1000)
    kwargs.setdefault("Hittuple","merged.root")
    kwargs.setdefault("RtRelation","basic")
    kwargs.setdefault("RtBinning","t")
    kwargs.setdefault("FloatP3",True)
    kwargs.setdefault("T0Offset",0.0)
    kwargs.setdefault("DoShortStrawCorrection",False)
    kwargs.setdefault("DoArgonXenonSep",True)                
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
    if "TRTCalDbTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_CalDbToolCfg
        kwargs.setdefault("TRT_CalDbTool", acc.popToolsAndMerge(TRT_CalDbToolCfg(flags)))

    acc.setPrivateTools(CompFactory.TRTCalibrator(name, **kwargs))
        
    return acc


    
# Steering algorithm. Either it fills track and hit ntuples, or it calls TRTCalibrator
def TRT_CalibrationMgrCfg(flags,name='TRT_CalibrationMgr',calibconstants='',**kwargs) :
    acc = ComponentAccumulator()

    kwargs.setdefault("DoCalibrate",False)
    kwargs.setdefault("DoRefit",False)

    if "AlignTrkTools" not in kwargs:
        from TRT_CalibTools.TRTCalibToolsConfig import (
            FillAlignTrkInfoCfg, FillAlignTRTHitsCfg)
        kwargs.setdefault("AlignTrkTools", [
            acc.addPublicTool(acc.popToolsAndMerge(FillAlignTrkInfoCfg(flags))),
            acc.addPublicTool(acc.popToolsAndMerge(FillAlignTRTHitsCfg(flags))) ] )

    if "FitTools" not in kwargs:
        from TRT_CalibTools.TRTCalibToolsConfig import FitToolCfg
        kwargs.setdefault("FitTools", [acc.popToolsAndMerge(FitToolCfg(flags))])

    if "TrackFitter" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import InDetTrackFitterCfg
        kwargs.setdefault("TrackFitter", acc.popToolsAndMerge(InDetTrackFitterCfg(flags))) 

    if "TrackSelectorTool" not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import TRT_InDetDetailedTrackSelectorToolCfg
        kwargs.setdefault("TrackSelectorTool", acc.popToolsAndMerge(
            TRT_InDetDetailedTrackSelectorToolCfg(flags)))
    
    # FIXME! Let all straws participate in trackfinding as default - SERGI This is wrong and needs to be UPDATED @peter    
        # acc.merge(addOverride('/TRT/Cond/Status','TRTCondStatus-empty-00-00'))
        # TypeError: addOverride() missing 1 required positional argument: 'tag'  
                         
    # acc.merge(addOverride('/TRT/Cond/Status','TRTCondStatus-empty-00-00'))
                          
    # if a text file is in the arguments, use the constants in that instead of the DB
    if not calibconstants=="":

        from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTCondWriteCfg
        acc.merge(TRTCondWriteCfg(flags,calibconstants))

    # add this algorithm to the configuration accumulator                       
    acc.addEventAlgo(CompFactory.TRTCalibrationMgr(name, **kwargs))

    return acc


def TRT_StrawStatusCfg(flags,name='InDet_TRT_StrawStatus',**kwargs) :
    
    acc = ComponentAccumulator()
    
    from TRT_ConditionsServices.TRT_ConditionsServicesConfig import (
        TRT_StrawStatusSummaryToolCfg)
    kwargs.setdefault("TRT_StrawStatusSummaryTool", acc.popToolsAndMerge(
        TRT_StrawStatusSummaryToolCfg(flags)))

    from InDetConfig.TRT_TrackHoleSearchConfig import TRTTrackHoleSearchToolCfg
    kwargs.setdefault("trt_hole_finder", acc.popToolsAndMerge(
        TRTTrackHoleSearchToolCfg(flags)))

    acc.addEventAlgo(CompFactory.InDet.TRT_StrawStatus(name,**kwargs))
    return acc



if __name__ == '__main__':
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags 
    flags.Input.Files = defaultTestFiles.RAW_RUN3
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    
    flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2023-03"
    flags.Exec.MaxEvents = 10 
    
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, ['ID'], toggle_geometry=True)
    
    flags.fillFromArgs()
    
    # Reason why we need to clone and replace: https://gitlab.cern.ch/atlas/athena/-/merge_requests/68616#note_7614858
    flags = flags.cloneAndReplace( "Tracking.ActiveConfig", f"Tracking.{flags.Tracking.PrimaryPassConfig.value}Pass")    
    flags.lock()
    
    # Set up the main service "acc"
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))
    
    from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
    acc.merge(InDetTrackRecoCfg(flags))
    
    # Algorithm to create the basic.root ntuple file 
    acc.merge(TRT_CalibrationMgrCfg(flags))
    
    # Algorithm to generate the straw masking file
    acc.merge(TRT_StrawStatusCfg(flags))
    
    import sys
    sys.exit(not acc.run().isSuccess())

