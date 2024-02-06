##########################################################################################################
##													##
##													##
##     Script to read text file and create .db and .pool files with status constants 		        ##
##													##
##													##
##													##
##     You have to set up the tag that you use to write mycool.db and pooloutputfile.root		## 
##     You must also choose between data and MC								##
##													##
##########################################################################################################

#--------------------> CHOOSE tag and text file
tagList         = [ "TRTStrawStatus-Test"]
#tagList         = [ "TRTStrawStatus-MC-run2-run3_00-00"]
objectList      = [ "TRTCond::StrawStatusMultChanContainer#/TRT/Cond/Status"]
#tags for folders that for some reason are hard-to-find in case of real data
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

inputfileTmp = "/afs/cern.ch/user/h/hansenph/work/public/strawstatus/output/athenaFormat_runDependentInactiveStraws_run0359310.txt"
#inputfileTmp = "/afs/cern.ch/user/h/hansenph/work/public/calibration/newtest/run/status/StrawStatusMCRun3.txt"



# only one event needed
from AthenaCommon.AppMgr import theApp
theApp.EvtMax = 1

from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
from AthenaCommon.GlobalFlags import globalflags
globalflags.DetGeo.set_Value_and_Lock("atlas")


#----------> CHOOSE for data or mc
# -----------> Case of real data 
globalflags.DataSource.set_Value_and_Lock("data")
from IOVDbSvc.CondDB import conddb
conddb.setGlobalTag("CONDBR2-BLKPA-2018-06")
#----> block the folders that are being read from sqlite DB
conddb.blockFolder("/TRT/Calib/DX")
conddb.blockFolder("/TRT/AlignL1/TRT")
conddb.blockFolder("/TRT/AlignL2")
conddb.blockFolder("/Indet/AlignL1/ID")
conddb.blockFolder("/Indet/AlignL2/PIX")
conddb.blockFolder("/Indet/AlignL2/SCT")
conddb.blockFolder("/Indet/AlignL3")
conddb.blockFolder("/TRT/Cond/StatusHT")

conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/Calib/DX',DXTag,force=True)
conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/AlignL1/TRT',TRTL1Tag,className="CondAttrListCollection",force=True)
conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/AlignL2',TRTL2Tag,className="AlignableTransformContainer",force=True)
conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/Indet/AlignL1/ID',IDL1Tag,className="CondAttrListCollection",force=True)
conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/Indet/AlignL2/PIX',PIXL2Tag,className="CondAttrListCollection",force=True)
conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/Indet/AlignL2/SCT',SCTL2Tag,className="CondAttrListCollection",force=True)
conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/Indet/AlignL3',IDL3Tag,className="AlignableTransformContainer",force=True)
conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/Cond/StatusHT',StatHTTag,className='TRTCond::StrawStatusMultChanContainer',force=True)

from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTAlignCondAlg
TRTAlignCondAlg = TRTAlignCondAlg(name = "TRTAlignCondAlg",UseDynamicFolders = True)
TRTAlignCondAlg.ReadKeyDynamicGlobal="/TRT/AlignL1/TRT"
TRTAlignCondAlg.ReadKeyDynamicRegular="/TRT/AlignL2"
# (for some reason the alignment folders, needed for geometry, cannot be read for data from ORACLE in this script)
#---------------------> End case of real data

# -----------> Case of MC 
#globalflags.DataSource.set_Value_and_Lock("geant4")
#from IOVDbSvc.CondDB import conddb
#conddb.setGlobalTag("OFLCOND-MC16-SDR-28")
# -----------> End case of MC 


#svcMgr.IOVDbSvc.forceRunNumber=352436

from AthenaCommon.AppMgr import ServiceMgr
ServiceMgr.IOVSvc.preLoadData = True

# block the folders that are being read from text file
conddb.blockFolder("/TRT/Cond/Status" )

# turn on trt
from AthenaCommon.DetFlags import DetFlags
DetFlags.TRT_setOn()
DetFlags.detdescr.TRT_setOn()

from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit


#turn on output services and algorithms
include ( "DetDescrCondAthenaPool/DetDescrCondAthenaPool_joboptions.py" )
include("RegistrationServices/RegistrationServices_jobOptions.py")
svcMgr.MessageSvc.OutputLevel      = INFO

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

from AthenaServices.AthenaServicesConf import AthenaOutputStreamTool
TRTCondStream=AthenaOutputStreamTool(name="CondStream1",OutputFile="trtcalibout.pool.root")
ToolSvc += TRTCondStream

from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTStrawStatusWrite
TSW=TRTStrawStatusWrite(name="TSW")
TSW.StatusInputFile		=inputfileTmp
TSW.StatusInputFileHT		=""
TSW.StatusInputFilePermanent	=""
topSequence +=TSW

outputFile = "pooloutputfile.root"

from RegistrationServices.OutputConditionsAlg import OutputConditionsAlg
myOCA=OutputConditionsAlg(outputFile=outputFile)
myOCA.ObjectList=objectList
myOCA.IOVTagList=tagList
myOCA.WriteIOV=True

myOCA.Run1=0
myOCA.LB1=0
myOCA.RUN2=2147483647
myOCA.LB2=4294967295

topSequence+=myOCA

