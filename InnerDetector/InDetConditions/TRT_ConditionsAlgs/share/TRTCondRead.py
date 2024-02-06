##########################################################################################################
##													##
##													##
##     Script to read .db and .pool files and create .txt with the 					##
##     calibration contants, to see the payload and check                 				##
##     if the conversion from text file to db has being done properly 				        ##
##													##
##     Poolfile should be inserted in the catalog:							##
##                                                 -pool_insertFileToCatalog  pooloutputfile.root	##
##													##
##     Output file:											##
##						   -caliboutput.txt					##
##													##
##     You need to set up the tag that you want to read and the DB it is read from (eg mycool.db)       ## 
##              to choose between data and MC						          	##
##													##
##########################################################################################################
#CHOOSE data or MC
isMC=False
# CHOOSE tag
T0Tag="Textt0"
RtTag="Textrt"
# tags for folders that are hard to find in the data DB
DXTag = "TRTCalibDX-RUN2-BLK-UPD4-03"
TRTL1Tag="TRTAlignL1_Run2_Legacy_looser"
TRTL2Tag="TRTAlignL2_Run2_Legacy_MDN_FullChain_GlobalSagitta"
IDL1Tag="IndetAlignL1ID_Run2_Legacy_looser"
PIXL2Tag="IndetAlignL2PIX_Run2_Legacy_looser"
SCTL2Tag="IndetAlignL2SCT_Run2_Legacy_looser"
IDL3Tag="IndetAlignL3_Run2_Legacy_MDN_FullChain_GlobalSagitta"
StatPermTag="TRTCondStatusPermanent-RUN2-BLK-UPD4-02-00"
StatHTTag="TrtStrawStatusHT-RUN2-Reprocessing"


from AthenaCommon.GlobalFlags import globalflags
globalflags.DetGeo.set_Value_and_Lock("atlas")
theApp.EvtMax = 1

if not isMC:
    globalflags.DataSource.set_Value_and_Lock("data")



# Optionally read one event from somewhere
#    from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
#athenaCommonFlags.FilesInput =["'root://eosatlas.cern.ch//eos/user/m/martis/data/InputFileForGridJobs/ZmumuMC16_AOD.18379878._000123.pool.root.1"]

    from IOVDbSvc.CondDB import conddb
# folders to dump
    conddb.blockFolder("/TRT/Calib/T0")
    conddb.blockFolder("/TRT/Calib/RT")

    conddb.setGlobalTag("CONDBR2-BLKPA-2018-06")

# folders that are hard to find in case of real data (for some reason)
    conddb.blockFolder("/TRT/Calib/DX")
    conddb.blockFolder("/TRT/AlignL1/TRT")
    conddb.blockFolder("/TRT/AlignL2")
    conddb.blockFolder("/Indet/AlignL1/ID")
    conddb.blockFolder("/Indet/AlignL2/PIX")
    conddb.blockFolder("/Indet/AlignL2/SCT")
    conddb.blockFolder("/Indet/AlignL3")
    conddb.blockFolder("/TRT/Cond/StatusPermanent")
    conddb.blockFolder("/TRT/Cond/StatusHT")


    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=CONDBR2</dbConnection>/TRT/Calib/T0',T0Tag,force=True,className='TRTCond::StrawT0MultChanContainer')
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=CONDBR2</dbConnection>/TRT/Calib/RT',RtTag,force=True,className='TRTCond::RtRelationMultChanContainer')

    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/Calib/DX',DXTag,force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/AlignL1/TRT',TRTL1Tag,className="CondAttrListCollection",force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/AlignL2',TRTL2Tag,className="AlignableTransformContainer",force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/Indet/AlignL1/ID',IDL1Tag,className="CondAttrListCollection",force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/Indet/AlignL2/PIX',PIXL2Tag,className="CondAttrListCollection",force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/Indet/AlignL2/SCT',SCTL2Tag,className="CondAttrListCollection",force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/Indet/AlignL3',IDL3Tag,className="AlignableTransformContainer",force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/Cond/StatusPermanent',StatPermTag,className='TRTCond::StrawStatusMultChanContainer',force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/Cond/StatusHT',StatHTTag,className='TRTCond::StrawStatusMultChanContainer',force=True)

    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTAlignCondAlg
    TRTAlignCondAlg = TRTAlignCondAlg(name = "TRTAlignCondAlg",UseDynamicFolders = True)
    TRTAlignCondAlg.ReadKeyDynamicGlobal="/TRT/AlignL1/TRT"
    TRTAlignCondAlg.ReadKeyDynamicRegular="/TRT/AlignL2"

#-----------> End case of real data


if isMC:
    globalflags.DataSource.set_Value_and_Lock("geant4")
    from IOVDbSvc.CondDB import conddb
# folders to dump
    conddb.blockFolder("/TRT/Calib/T0")
    conddb.blockFolder("/TRT/Calib/RT")
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=OFLP200</dbConnection>/TRT/Calib/T0',T0Tag,force=True,className='TRTCond::StrawT0MultChanContainer')
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=OFLP200</dbConnection>/TRT/Calib/RT',RtTag,force=True,className='TRTCond::RtRelationMultChanContainer')
    conddb.setGlobalTag("OFLCOND-MC16-SDR-25")
#-----------> End case of MC


# Set this if you have several run ranges in the chosen db
svcMgr.IOVDbSvc.forceRunNumber=340000

#turn on trt
from AthenaCommon.DetFlags import DetFlags
DetFlags.TRT_setOn()
DetFlags.detdescr.TRT_setOn()

from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit

#setup algorithm to dump the folder payloads to text file
from AthenaCommon.AppMgr import ServiceMgr as svcMgr
svcMgr.IOVSvc.preLoadData = True
svcMgr.MessageSvc.OutputLevel      = INFO

from AthenaCommon.AlgSequence import AthSequencer
condSeq = AthSequencer("AthCondSeq")
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()


from AthenaCommon.AppMgr import ServiceMgr as svcMgr

from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTCondRead
TRTCondRead = TRTCondRead( name = "TRTCondRead",
                          CalibOutputFile="caliboutput.txt")
topSequence+=TRTCondRead





