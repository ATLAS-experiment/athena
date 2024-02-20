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
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawStatusSummaryToolCfg
        InDetStrawSummaryTool = acc.popToolsAndMerge(TRT_StrawStatusSummaryToolCfg(flags))
        kwargs.setdefault("TRTStrawSummaryTool", InDetStrawSummaryTool)
    if "NeighbourSvc" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawNeighbourSvcCfg
        kwargs.setdefault("NeighbourSvc", acc.getPrimaryAndMerge(TRT_StrawNeighbourSvcCfg(flags)))
    if "TRTCalDbTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_CalDbToolCfg
        kwargs.setdefault("TRT_CalDbTool", acc.popToolsAndMerge(TRT_CalDbToolCfg(flags)))

    acc.setPrivateTools(CompFactory.TRTCalibrator(name, **kwargs))
        
    return acc


    
# Steering algorithm. Either it fills track and hit ntuples, or it calls TRTCalibrator
def TRT_CalibrationMgrCfg(flags,name='TRT_CalibrationMgr',calibconstants='',**kwargs) :
    acc = ComponentAccumulator()
    
    kwargs.setdefault("DoCalibrate",False)

    from TRT_CalibTools.TRTCalibToolsConfig import FillAlignTrkInfoCfg, FillAlignTRTHitsCfg, FitToolCfg
    kwargs.setdefault("AlignTrkTools", [acc.addPublicTool(acc.popToolsAndMerge(FillAlignTrkInfoCfg(flags))), 
                                        acc.addPublicTool(acc.popToolsAndMerge(FillAlignTRTHitsCfg(flags)))] )      

    kwargs.setdefault("FitTools", [acc.popToolsAndMerge(FitToolCfg(flags))])
    
    # check this line below - needs the function
    from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg
    kwargs.setdefault("TrackFitter", acc.popToolsAndMerge(ActsFitterCfg(flags)))
    
    from InDetConfig.InDetTrackSelectorToolConfig import InDetDetailedTrackSelectorToolCfg
    kwargs.setdefault("TrackSelectorTool", acc.popToolsAndMerge(InDetDetailedTrackSelectorToolCfg(flags)))
    
    # FIXME! Let all straws participate in trackfinding as default - SERGI This is wrong and needs to be UPDATED @peter    
        # acc.merge(addOverride('/TRT/Cond/Status','TRTCondStatus-empty-00-00'))
        # TypeError: addOverride() missing 1 required positional argument: 'tag'  
                         
    # acc.merge(addOverride('/TRT/Cond/Status','TRTCondStatus-empty-00-00'))
                          
    # if a text file is in the arguments, use the constants in that instead of the DB
    # if not calibconstants=="":

    #     from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTCondWriteCfg
    #     acc.merge(TRTCondWriteCfg(flags,calibconstants))

    # add this algorithm to the configuration accumulator                       
    acc.addEventAlgo(CompFactory.TRTCalibrationMgr(name, **kwargs))

    return acc
        
# FIXME - where is this tool used? Needs some feedback                  
def TRT_TrackHoleSearch(flags,name="TRT_TrackHoleSearch",**kwargs):
                        
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg                          
    acc = AtlasExtrapolatorCfg(flags)
    kwargs.setdefault("extrapolator", acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    kwargs.setdefault("use_conditions_svc",True)
    kwargs.setdefault("do_dump_bad_straw_log",False)
    kwargs.setdefault("begin_at_first_trt_hit",False)
    kwargs.setdefault("end_at_last_trt_hit",False)
    kwargs.setdefault("max_trailing_holes",1)
    kwargs.setdefault("locR_cut",-1)
    kwargs.setdefault("locR_sigma_cut",-1)

    return acc

# we need to recheck this, not fully sure - Sergi                     
def TRT_StrawStatusCfg(flags,name='InDet__TRT_StrawStatus',**kwargs) :

    if "TRT_TrackHoleSearch" not in kwargs:
        kwargs.setdefault("trt_hole_finder", acc.popToolsAndMerge(CompFactory.TRT_TrackHoleSearchCfg(flags, name = name))) 

    acc.addEventAlgo(CompFactory.TRT_CalibrationMgr("TRT_StrawStatus",**kwargs))

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
    flags.lock()
    
    # Set up the main service "acc"
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))
    
    from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
    acc.merge(InDetTrackRecoCfg(flags))
    
    acc.merge(TRT_CalibrationMgrCfg(flags))
    
    import sys
    sys.exit(not acc.run().isSuccess())
    


    