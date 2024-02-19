"""Define methods to construct a configured TRT R-t calibration algorithm

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
# from IOVDbSvc.IOVDbSvcConfig import addFolders
# from AthenaConfiguration.Enums import Format


# Tool to write a track-tuple with TRT hit info
def FillAlignTrkInfoCfg(flags,name='FillAlignTrkInfo',**kwargs) :
    acc = ComponentAccumulator()
    
    from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
    kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(InDetTrackSummaryToolCfg(flags)))
    
    acc.setPrivateTools(CompFactory.FillAlignTrkInfo(name, **kwargs))
    return acc


# SERGI - This function should be in the correct athena pkg.. not here
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
    if "TRTCaldbTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_CalDbToolCfg
        TRT_CalDbTool = acc.popToolsAndMerge(TRT_CalDbToolCfg(flags))
        acc.setPrivateTools(TRT_CalDbTool)
        
        
    return acc

# Tool to select tracks to be used for calibration
def InDetDetailedTrackSelectorToolCfg(flags,name="InDetDetailedTrackSelectorTool",**kwargs):
    
    from AthenaCommon.SystemOfUnits import GeV, mm
    
    acc = ComponentAccumulator()      
    kwargs.setdefault("name", name)    
    kwargs.setdefault("pTMin", 1.0*GeV)     
    # kwargs.setdefault("fitChi2OndfMax",50.0)  ---> This does not exist in the InDetDetailedTrackSelectorTool.cxx file
    kwargs.setdefault("z0Max",99999.0*mm)   
    kwargs.setdefault("IPd0Max",10.0*mm)    
    kwargs.setdefault("IPd0Max",300.0*mm)   
    kwargs.setdefault("etaMax",2.1)
    kwargs.setdefault("nHitBLayer",0)
    kwargs.setdefault("nHitPix",2)
    kwargs.setdefault("nHitBLayerPlusPix",0)
    kwargs.setdefault("nHitSct",0)
    kwargs.setdefault("nHitSi",7)
    kwargs.setdefault("nHitTrtPlusOutliers",20)
    kwargs.setdefault("nHitTrtPlusOutliersHighE",20)
    kwargs.setdefault("nHitTrtHighE",0)
    
    # from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
    # kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(InDetExtrapolatorCfg(flags, **kwargs)))
    
    
    from InDetConfig.InDetTrackSelectorToolConfig import InDetTrackSelectorToolCfg
    acc.setPrivateTools(acc.popToolsAndMerge(InDetTrackSelectorToolCfg(flags, **kwargs)))
    return acc
    
# Steering algorithm. Either it fills track and hit ntuples, or it calls TRTCalibrator
def TRT_CalibrationMgrCfg(flags,name='TRT_CalibrationMgr',calibconstants='',**kwargs) :
    acc = ComponentAccumulator()
    
    kwargs.setdefault("DoCalibrate",False)


    # FIXME - FillAlignTRTHitsCfg and FillAlignTrkInfoCfg functions should be moved to TRT_CalibTools (where the .cxx/.h are stored)
    kwargs.setdefault("AlignTrkTools", [acc.addPublicTool(acc.popToolsAndMerge(FillAlignTrkInfoCfg(flags))), 
                                        acc.addPublicTool(acc.popToolsAndMerge(FillAlignTRTHitsCfg(flags)))] )      

    # FIXME - FitToolCfg function should be moved to TRT_CalibTools (where the .cxx/.h are stored)
    kwargs.setdefault("FitTools", [acc.popToolsAndMerge(FitToolCfg(flags))])
    
    # check this line below - needs the function
    # kwargs.setdefault("TrackFitter", acc.setPrivateTools(CompFactory.ActsTrk.KalmanFitter("KalmanFitter")))
    from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg
    kwargs.setdefault("TrackFitter", acc.popToolsAndMerge(ActsFitterCfg(flags)))
    
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

# we need to recheck this not fully sure - Sergi                     
def TRT_StrawStatusCfg(flags,name='InDet__TRT_StrawStatus',**kwargs) :

    if "TRT_TrackHoleSearch" not in kwargs:
        kwargs.setdefault("trt_hole_finder", acc.popToolsAndMerge(CompFactory.TRT_TrackHoleSearchCfg(flags, name = name))) 

    acc.addEventAlgo(CompFactory.TRT_CalibrationMgr("TRT_StrawStatus",**kwargs))

    return acc



if __name__ == '__main__':
    print("start running")
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags 
    print(defaultTestFiles.RAW_RUN3)
    
    flags.Input.Files = defaultTestFiles.RAW_RUN3
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2023-03"
    flags.Exec.MaxEvents = 10
    
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, ['ID'], toggle_geometry=True)
    
    flags.fillFromArgs()
    flags.lock()
    
    print("start running 1")
    
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
    


    