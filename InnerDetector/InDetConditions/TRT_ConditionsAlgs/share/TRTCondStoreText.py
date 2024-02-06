##########################################################################################################
##													##
##													##
##     Script to read text file and create .db and .pool files with calibration constants 		##
##													##
##													##
##													##
##     You have to set up the tag that you use to write mycool.db and pooloutputfile.root		## 
##													##
##													##
##########################################################################################################

#CHOOSE tag and text file
T0Tag="Textt0"
RtTag="Textrt"
DXTag="TRTCalibDX-RUN2-BLK-UPD4-03"
TRTL1Tag="TRTAlignL1_Run2_Legacy_looser"
TRTL2Tag="TRTAlignL2_Run2_Legacy_MDN_FullChain_GlobalSagitta"
IDL1Tag="IndetAlignL1ID_Run2_Legacy_looser"
PIXL2Tag="IndetAlignL2PIX_Run2_Legacy_looser"
SCTL2Tag="IndetAlignL2SCT_Run2_Legacy_looser"
IDL3Tag="IndetAlignL3_Run2_Legacy_MDN_FullChain_GlobalSagitta"
StatTag="TRTCondStatus-RUN2-BLK-UPD2-01-01"
StatPermTag="TRTCondStatusPermanent-RUN2-BLK-UPD4-02-00"
StatHTTag="TrtStrawStatusHT-RUN2-Reprocessing"
#errTag="Texterr"
#slopeTag="Textslope"
constants= "dbconst.439519.txt"

# only one event needed
from AthenaCommon.AppMgr import theApp
theApp.EvtMax = 1
#-------------- CHOOSE data or mc
isdata = True

from AthenaCommon.GlobalFlags import globalflags
globalflags.DetGeo.set_Value_and_Lock("atlas")
if isdata :
    globalflags.DataSource.set_Value_and_Lock("data")
else:
    globalflags.DataSource.set_Value_and_Lock("geant4")
from IOVDbSvc.CondDB import conddb
if isdata :
    conddb.setGlobalTag("CONDBR2-BLKPA-2022-10")
else :
    conddb.setGlobalTag("OFLCOND-MC16-SDR-28")
#--------------> Case of real data:

if isdata :
#    You may want to read some particular data
#    from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
#    athenaCommonFlags.FilesInput =["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data17_13TeV.00330470.physics_Main.daq.RAW._lb0310._SFO-1._0001.data"]

    conddb.blockFolder("/TRT/Calib/DX")
    conddb.blockFolder("/TRT/AlignL1/TRT")
    conddb.blockFolder("/TRT/AlignL2")
    conddb.blockFolder("/Indet/AlignL1/ID")
    conddb.blockFolder("/Indet/AlignL2/PIX")
    conddb.blockFolder("/Indet/AlignL2/SCT")
    conddb.blockFolder("/Indet/AlignL3")
    conddb.blockFolder("/TRT/Cond/Status")
    conddb.blockFolder("/TRT/Cond/StatusPermanent")
    conddb.blockFolder("/TRT/Cond/StatusHT")

    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/Calib/DX',DXTag,force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/AlignL1/TRT',TRTL1Tag,className="CondAttrListCollection",force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/AlignL2',TRTL2Tag,className="AlignableTransformContainer",force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/Indet/AlignL1/ID',IDL1Tag,className="CondAttrListCollection",force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/Indet/AlignL2/PIX',PIXL2Tag,className="CondAttrListCollection",force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/Indet/AlignL2/SCT',SCTL2Tag,className="CondAttrListCollection",force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/Indet/AlignL3',IDL3Tag,className="AlignableTransformContainer",force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/Cond/Status',StatTag,className='TRTCond::StrawStatusMultChanContainer',force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/Cond/StatusPermanent',StatPermTag,className='TRTCond::StrawStatusMultChanContainer',force=True)
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/Cond/StatusHT',StatHTTag,className='TRTCond::StrawStatusMultChanContainer',force=True)

    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTAlignCondAlg
    TRTAlignCondAlg = TRTAlignCondAlg(name = "TRTAlignCondAlg",UseDynamicFolders = True)
    TRTAlignCondAlg.ReadKeyDynamicGlobal="/TRT/AlignL1/TRT"
    TRTAlignCondAlg.ReadKeyDynamicRegular="/TRT/AlignL2"

#   (for some reasons the geometry folders are not found in ORACLE db, in case of data. So we read them from a local DB)
#--------------> End case of real data:

# block the folders that are being read from text file
conddb.blockFolder("/TRT/Calib/RT" )
conddb.blockFolder("/TRT/Calib/T0" )

# turn on trt
from AthenaCommon.DetFlags import DetFlags
DetFlags.TRT_setOn()
DetFlags.detdescr.TRT_setOn()

from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit

#turn on output services and algorithms
include ( "DetDescrCondAthenaPool/DetDescrCondAthenaPool_joboptions.py" )
include("RegistrationServices/RegistrationServices_jobOptions.py")

IOVSvc = Service("IOVSvc")
IOVSvc.preLoadData = True

#svcMgr.IOVDbSvc.forceRunNumber=430000
svcMgr.MessageSvc.OutputLevel      = 3

from AthenaCommon.AlgSequence import AthSequencer
condSeq = AthSequencer("AthCondSeq")
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

from AthenaServices.AthenaServicesConf import AthenaOutputStreamTool
TRTCondStream=AthenaOutputStreamTool(name="CondStream1",OutputFile="trtcalibout.pool.root")
ToolSvc += TRTCondStream

from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTCondStoreText
TRTCondStoreText=TRTCondStoreText( name = "TRTCondStoreText",
                           CalibInputFile=constants)
condSeq+=TRTCondStoreText

outputFile = "pooloutputfile.root"
objectList = [ "TRTCond::RtRelationMultChanContainer#/TRT/Calib/RT","TRTCond::StrawT0MultChanContainer#/TRT/Calib/T0"]
tagList = [RtTag,T0Tag]

from RegistrationServices.OutputConditionsAlg import OutputConditionsAlg
myOCA=OutputConditionsAlg(outputFile=outputFile)
myOCA.ObjectList=objectList
myOCA.IOVTagList=tagList
myOCA.WriteIOV=True

myOCA.Run1=0
myOCA.LB1=0
myOCA.Run2=2147483647
myOCA.LB2=4294967295

topSequence+=myOCA

