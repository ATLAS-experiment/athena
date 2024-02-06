import os,sys,time,glob,fnmatch


def collision(config,inputfiles,calibconstants,nevents):
#    print "Loading from CollisionTemplate.py"
#    print "Based on Thjis jO: "
    ostring="""

from AthenaCommon.AppMgr import ToolSvc

from AthenaCommon.AppMgr import ServiceMgr

from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_StrawNeighbourSvc
TRTStrawNeighbourSvc=TRT_StrawNeighbourSvc()
ServiceMgr += TRTStrawNeighbourSvc

from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_CalDbTool
InDetCalDbTool=TRT_CalDbTool(name = "TRT_CalDbTool")

from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_StrawStatusSummaryTool
InDetStrawSummaryTool=TRT_StrawStatusSummaryTool(name = "TRT_StrawStatusSummaryTool",
                             isGEANT4=(globalflags.DataSource == 'geant4'))

from TRT_CalibTools.TRT_CalibToolsConf import FitTool
TRTCalFitTool = FitTool (name = 'TRTCalFitTool')
ToolSvc += TRTCalFitTool
print	   (TRTCalFitTool)

from AthenaServices.AthenaServicesConf import AthenaOutputStreamTool
TRTCondStream=AthenaOutputStreamTool(name="CondStream1",OutputFile="trtcalibout.pool.root")

ToolSvc += TRTCondStream
print (TRTCondStream)

from TRT_CalibTools.TRT_CalibToolsConf import FillAlignTrkInfo 
FillAlignTrkInfo = FillAlignTrkInfo ( name = 'FillAlignTrkInfo',
                                      TrackSummaryTool = InDetTrackSummaryTool)
ToolSvc += FillAlignTrkInfo
print      (FillAlignTrkInfo)

from TRT_CalibTools.TRT_CalibToolsConf import FillAlignTRTHits 
FillAlignTRTHits = FillAlignTRTHits ( name = 'FillAlignTRTHits',
                                      minTimebinsOverThreshold=0, 
                                      NeighbourSvc=TRTStrawNeighbourSvc,
                                      TRTCalDbTool = InDetCalDbTool,
                                      TRTStrawSummaryTool = InDetStrawSummaryTool)

ToolSvc += FillAlignTRTHits
print      (FillAlignTRTHits)


from TRT_CalibTools.TRT_CalibToolsConf import TRTCalibrator 
TRTCalibrator = TRTCalibrator ( name = 'TRTCalibrator',
                                MinRt               = %s,""" % config["MinRT"]
    ostring+="""
                                MinT0               = %s,""" % config["MinT0"]
    ostring+="""
                                Nevents             = -1,\n"""

    ostring+="""
                                Hittuple            = 'merged.root',
                                RtRel               = '%s',""" % config["RtRelation"]
    ostring+="""
                                RtBinning           = '%s',""" % config["RtBinning"]
    ostring+="""
                                UseP0               = %s,""" % "True"#config["UsePol0"]
    ostring+="""
                                FloatP3             = %s,""" % config["FloatPol3"]
    ostring+="""
                                T0Offset             = %s,""" % config["T0Offset"]
    ostring+="""
                                TrtManagerLocation  = InDetKeys.TRT_Manager(),
				DoShortStrawCorrection = False,
                                TRTStrawSummaryTool = InDetStrawSummaryTool,
                                NeighbourSvc=TRTStrawNeighbourSvc,
"""	
    if config["DoArXe"]:
    	ostring+="""                                DoArXenonSep    = True,
"""
    ostring+="""                                TRTCalDbTool=InDetCalDbTool)
ToolSvc += TRTCalibrator
print      (TRTCalibrator)



from InDetTrackSelectorTool.InDetTrackSelectorToolConf import InDet__InDetDetailedTrackSelectorTool
TRTTrackSelectorTool = InDet__InDetDetailedTrackSelectorTool(name = "InDetDetailedTrackSelectorTool",
                                        pTMin                   =    1. *GeV ,
                                        fitChi2OnNdfMax         = 50. ,
                                        z0Max                   = 9999. *mm ,
                                        IPd0Max                 = 10.   *mm ,
                                        IPz0Max                 = 300.  *mm ,                                                                          
                                        etaMax                  = 2.1 ,                                                                          
                                        nHitBLayer              = 0 ,
                                        nHitPix                 = 2 ,
                                        nHitBLayerPlusPix       = 0 ,
                                        nHitSct                 = 0 ,
                                        nHitSi                  = 7 ,
                                        nHitTrt                 = 20,
                                        nHitTrtPlusOutliers     = 20,
                                        nHitTrtHighE              =0,
                                        nHitTrtPlusOutliersHighE  =0
                                        )

ToolSvc += TRTTrackSelectorTool
if (InDetFlags.doPrintConfigurables()):
        print (TRTTrackSelectorTool)




from TRT_CalibAlgs.TRT_CalibAlgsConf import TRTCalibrationMgr
CosmicsTRTCalibMgr = TRTCalibrationMgr(name                = 'CosmicsTRTCalibMgr',
                                       StreamTool          =  TRTCondStream,
                                       TrackSelectorTool   = TRTTrackSelectorTool,
                                       AlignTrkTools       = [ FillAlignTrkInfo, FillAlignTRTHits ],
                                       TrackFitter         = InDetTrackFitter,
                                       FitTools            = [ TRTCalFitTool] )

topSequence += CosmicsTRTCalibMgr
print (CosmicsTRTCalibMgr)


#==============================
#====  Straw Status stuff  ====
#==============================
from TrkExTools.AtlasExtrapolator import AtlasExtrapolator
theAtlasExtrapolator = AtlasExtrapolator()
ToolSvc += theAtlasExtrapolator
 
from TRT_TrackHoleSearch.TRT_TrackHoleSearchConf import TRTTrackHoleSearchTool
theTRTTrackHoleSearchTool = TRTTrackHoleSearchTool(
              OutputLevel = WARNING,
             extrapolator = theAtlasExtrapolator,
             #conditions_svc = InDetTRTConditionsSummaryService, # defined in InDetRec_all.py
             use_conditions_svc = True,
             do_dump_bad_straw_log = False,
             begin_at_first_trt_hit = False, # if not, extrapolate from last Si hit
             end_at_last_trt_hit = False, # if not, continue hole search to the edge of the TRT
             max_trailing_holes = 1, # only used if end_at_last_trt_hit=False
             locR_cut = -1, # 1.4*mm  # negative means no cut
             locR_sigma_cut = -1)
 
ToolSvc += theTRTTrackHoleSearchTool


from TRT_CalibAlgs.TRT_CalibAlgsConf import InDet__TRT_StrawStatus
TRT_StrawStatus = InDet__TRT_StrawStatus(       name                    = "TRT_StrawStatus",
                                                outputFileName          = "TRT_StrawStatusOutput",
                                                trt_hole_finder         = theTRTTrackHoleSearchTool,
                                               )
topSequence += TRT_StrawStatus
print (TRT_StrawStatus)


#conddb.addOverride('/TRT/Calib/DX','TRTCalibDX-RUN2-BLK-UPD4-03')
conddb.addOverride('/TRT/Cond/Status','TRTCondStatus-empty-00-00')
#conddb.addOverride('/Indet/Beampos','IndetBeampos-ES1-UPD2') 

# DCS Data Folders
if (globalflags.InputFormat() == 'bytestream' and globalflags.DataSource() == 'data'):
    if InDetFlags.useTrtDCS():
        conddb.addFolder('DCS_OFL',"/TRT/DCS/HV/BARREL <cache>600</cache>")#,classname='CondAttrListCollection')
        conddb.addFolder('DCS_OFL',"/TRT/DCS/HV/ENDCAPA <cache>600</cache>")#,classname='CondAttrListCollection')
        conddb.addFolder('DCS_OFL',"/TRT/DCS/HV/ENDCAPC <cache>600</cache>")#,classname='CondAttrListCollection')

"""
    if not calibconstants=="":
        ostring+="""
from AthenaCommon.AlgSequence import AthSequencer
condSequence=AthSequencer('AthCondSeq')
conddb.blockFolder("/TRT/Calib/RT")
conddb.blockFolder("/TRT/Calib/T0")
from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTCondWrite
"""
        ostring += """TRTCondWrite = TRTCondWrite( name = "TRTCondWrite",
                                     CalibInputFile='%s')\n"""% calibconstants
        ostring += """condSequence+=TRTCondWrite """

    return ostring


