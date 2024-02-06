def cosmic(config, inputfiles, calibconstants, nevents):

    print ("Loading from CosmicTemplate.py")
    ostring = """
#--------------------------------------------------------------
# Calibration stuff
#--------------------------------------------------------------

from AthenaCommon.AppMgr import ToolSvc

from AthenaCommon.AppMgr import ServiceMgr

from TrkDetDescrSvc.TrkDetDescrJobProperties import TrkDetFlags
TrkDetFlags.TRT_BuildStrawLayers = True

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
print      (TRTCalFitTool)

from OutputStreamAthenaPool.OutputStreamAthenaPoolConf import AthenaPoolOutputStreamTool
TRTCondStream=AthenaPoolOutputStreamTool(name="CondStream1",OutputFile="trtcalibout.pool.root")

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
                                      TRTCalDbTool=TRTCalibDBTool,
                                      TRTStrawSummaryTool = InDetStrawSummaryTool)
ToolSvc += FillAlignTRTHits
print      (FillAlignTRTHits)


from InDetTrackSelectorTool.InDetTrackSelectorToolConf import InDet__InDetCosmicTrackSelectorTool
TRTTrackSelectorTool = InDet__InDetCosmicTrackSelectorTool(name = "InDetCosmicTrackSelectorTool",
                                        TrackSummaryTool        = InDetTrackSummaryTool,
					maxZ0			= 10000,
					maxD0			= 10000,
					minPt			= 1000,
					numberOfPixelHits	= 0,
					numberOfSCTHits		= 0,
					numberOfTRTHits		= 20,
					numberOfSiliconHits	= 0,
					numberOfSiliconHitsTop	= -1,
					numberOfSiliconHitsBottom	=-1,
                                        )

ToolSvc += TRTTrackSelectorTool
if (InDetFlags.doPrintConfigurables()):
        print (TRTTrackSelectorTool)



#from InDetTrackSelectorTool.InDetTrackSelectorToolConf import InDet__InDetDetailedTrackSelectorTool
#TRTTrackSelectorTool = InDet__InDetDetailedTrackSelectorTool(name = "InDetDetailedTrackSelectorTool",
#        				OutputLevel = DEBUG,
#                                        pTMin                   =    100    ,
#                                        fitChi2OnNdfMax         = 50. ,
#                                        z0Max                   = 100000  ,
#					IPd0Max			= 100000  ,
#					IPz0Max			= 100000  ,
#					sigIPd0Max		= 1000,
#					sigIPz0Max		= 1000,
#                                        nHitBLayer              = 0 ,
#                                        nHitPix                 = 0 ,
#                                        nHitBLayerPlusPix       = 0 ,
#                                        nHitSct                 = 0 ,
#                                        nHitSi                  = 0 ,
#                                        nHolesPixel             = 0,
#					nHitSiPhysical		= 0,
#                                        nHitTrt                         = 0,
#                                        nHitTrtPlusOutliers             =0,
#                                        nHitTrtHighE                    =0,
#                                        nHitTrtPlusOutliersHighE        =0,
#                                        nHitTrtHighEFractionMax         =9999,
#                                        nHitTrtHighEFractionWithOutliersMax=9999
#                                        )
#
#ToolSvc += TRTTrackSelectorTool
#if (InDetFlags.doPrintConfigurables()):
#        print (TRTTrackSelectorTool)


from TRT_CalibAlgs.TRT_CalibAlgsConf import TRTCalibrationMgr
CosmicsTRTCalibMgr = TRTCalibrationMgr(name                = 'CosmicsTRTCalibMgr',
                                       TrackSelectorTool   = TRTTrackSelectorTool,
                                       AlignTrkTools       = [ FillAlignTrkInfo, FillAlignTRTHits ],
				       IsCosmic 	   = True,
                                       DoRefit             = True,
                                       TrackFitter         = InDetTrackFitter,
                                       FitTools            = [ TRTCalFitTool] )

topSequence += CosmicsTRTCalibMgr
print (CosmicsTRTCalibMgr)


 
#==============================
#====  Straw Status stuff  ====
#==============================

from TRT_CalibAlgs.TRT_CalibAlgsConf import InDet__TRT_StrawStatus
TRT_StrawStatus = InDet__TRT_StrawStatus(       name                    = "TRT_StrawStatus",
                                                outputFileName          = "TRT_StrawStatusOutput",
						  IsCosmic		  = True,                                          
                                                )
topSequence += TRT_StrawStatus
print (TRT_StrawStatus)
"""
    if not calibconstants == "":
        ostring += """
conddb.blockFolder("/TRT/Calib/RT" )
conddb.blockFolder("/TRT/Calib/T0" )
from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTCondWrite
TRTCondWrite = TRTCondWrite( name = "TRTCondWrite")
topSequence+=TRTCondWrite 
"""
    if not calibconstants == "":
        ostring += 'TRTCalibDBSvc.calibTextFile="%s"\n' % (calibconstants)
    ostring += "TRTCalibDBSvc.StreamTool=TRTCondStream"
    return ostring
