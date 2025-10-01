"""Define methods to construct a configured TRT R-t calibration algorithm

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
    
# Steering algorithm. Either it fills track and hit ntuples, or it calls TRTCalibrator
def TRT_CalibrationMgrCfg(flags,name='TRT_CalibrationMgr',calibconstants='', Hittuple='', caltag='' ,**kwargs) :
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
        from TrkConfig.CommonTrackFitterConfig import InDetStandaloneTrackFitterCfg
        kwargs.setdefault("TrackFitter", acc.popToolsAndMerge(
            InDetStandaloneTrackFitterCfg(flags)))

    if "TrackSelectorTool" not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import TRT_InDetDetailedTrackSelectorToolCfg
        kwargs.setdefault("TrackSelectorTool", acc.popToolsAndMerge(TRT_InDetDetailedTrackSelectorToolCfg(flags)))
        
    if "TRTCalibrator" not in kwargs:
        from TRT_CalibTools.TRTCalibratorConfig import  TRTCalibratorCfg
        
        if not Hittuple:
            kwargs.setdefault("TRTCalibrator",[acc.addPublicTool(acc.popToolsAndMerge(TRTCalibratorCfg(flags)))])        
        else:
            # This changes the name of the input file used for the calibrator tool
            kwargs.setdefault("TRTCalibrator",[acc.addPublicTool(acc.popToolsAndMerge(TRTCalibratorCfg(flags, Hittuple=Hittuple, calTag=caltag)))])
        
    # if a text file is in the arguments, use the constants in that instead of the DB
    if calibconstants:

        # Renaming the keys stored by the CondInputLoader, since we will write the new constants using TRTCondWrite Alg.
        from IOVDbSvc.IOVDbSvcConfig import addOverride
        acc.merge(addOverride( flags, "/TRT/Calib/T0", "unused_condDBT0", "key" ))
        acc.merge(addOverride( flags, "/TRT/Calib/RT", "unused_condDBRT", "key" ))

        from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTCondWriteCfg
        acc.merge(TRTCondWriteCfg(flags,CalibInputFile=calibconstants))

    # add this algorithm to the configuration accumulator                       
    acc.addEventAlgo(CompFactory.TRTCalibrationMgr(name, **kwargs))

    return acc


def TRT_StrawStatusCfg(flags,name='InDet_TRT_StrawStatus',**kwargs) :
    
    acc = ComponentAccumulator()
    
    from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawStatusSummaryToolCfg
    kwargs.setdefault("TRT_StrawStatusSummaryTool", acc.popToolsAndMerge(TRT_StrawStatusSummaryToolCfg(flags)))

    from InDetConfig.TRT_TrackHoleSearchConfig import TRTTrackHoleSearchToolCfg
    kwargs.setdefault("trt_hole_finder", acc.popToolsAndMerge(TRTTrackHoleSearchToolCfg(flags)))
    
    from IOVDbSvc.IOVDbSvcConfig import addOverride
    acc.merge(addOverride(flags,'/TRT/Cond/Status','TRTCondStatus-empty-00-00'))

    acc.addEventAlgo(CompFactory.InDet.TRT_StrawStatus(name,**kwargs))
    return acc


def CalibConfig(flags):
    
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, ['ID'], toggle_geometry=True)
    
    # Reco
    flags.Reco.EnableTau=False
    flags.Reco.EnableCombinedMuon=False
    flags.Reco.EnableMet=False
    flags.Reco.EnableTrigger = False
    flags.Reco.EnableEgamma=False
    flags.Reco.EnableCaloRinger=False
    flags.Reco.EnableCaloExtension=False
    
    # Detector
    flags.Detector.EnableMuon=False
    flags.Detector.EnableCalo=False
    
    # DQ
    flags.DQ.doMonitoring=False
    flags.DQ.Steering.InDet.doPerfMon=False
    flags.DQ.Steering.InDet.doGlobalMon=False
    flags.DQ.Steering.doPixelMon=False
    flags.DQ.Steering.doSCTMon=False
    
    # Tracking
    flags.Tracking.doCaloSeededBrem=False
    flags.Tracking.doCaloSeededAmbi=False
    flags.Tracking.doTRTSegments=False
    flags.Tracking.doTRTStandalone=False
    flags.Tracking.doBackTracking=False
    
    
if __name__ == '__main__':
    
    import glob, argparse
    parser = argparse.ArgumentParser(prog='python -m TRT_CalibAlgs.TRTCalibrationMgrConfig',
                                   description="""Run R-t TRT calibration.\n\n
                                   Example: python -m TRT_CalibAlgs.TRTCalibrationMgrConfig --filesInput "/path/to/files/data22*" --evtMax 10""")
    
    parser.add_argument('--evtMax'      ,type=int,default=1,help="Number of events.")
    parser.add_argument('--filesInput'  , nargs='+', default=[],help="Input files. RAW data")
    parser.add_argument('--fileOutput'  , default="basic.root" ,help="Output file name. Flat Ntuple")
    parser.add_argument('--doCalibrator',action='store_true' ,help="Run the calibrator to obtain the constants")
    parser.add_argument('--calibconstants', default="" ,help="Input file constants for writer")
    args = parser.parse_args()
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles, defaultConditionsTags
    if not args.filesInput:
        flags.Input.Files = defaultTestFiles.RAW_RUN3
    else:
        flags.Input.Files = [file for x in args.filesInput for file in glob.glob(x)]
        
    flags.Output.HISTFileName = args.fileOutput
    flags.Exec.MaxEvents = args.evtMax
    
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA  
    
    CalibConfig(flags)  

    # For debug output INFO=3
    flags.Exec.OutputLevel = 3 
    
    # Reason why we need to clone and replace: https://gitlab.cern.ch/atlas/athena/-/merge_requests/68616#note_7614858
    flags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        f"Tracking.{flags.Tracking.PrimaryPassConfig.value}Pass",
        # Keep original flags as some of the subsequent passes use
        # lambda functions relying on them
        keepOriginal=True)  
     
    flags.lock()
    flags.dump()
    
    # Set up the main service "acc"
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))
    
    from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
    acc.merge(InDetTrackRecoCfg(flags))

    # Algorithm to generate the straw masking file
    acc.merge(TRT_StrawStatusCfg(flags)) 

    # Algorithm to create the basic.root ntuple file 
    acc.merge(TRT_CalibrationMgrCfg(flags,calibconstants=args.dbconst ,DoCalibrate=args.doCalibrator))

    with open("TRTCalibConfigCA.pkl", "wb") as f:
        acc.store(f)
        f.close()

    
    import sys
    sys.exit(not acc.run().isSuccess())

