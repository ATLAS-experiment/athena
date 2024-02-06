##########################################################################################################
##													##
##													##
##     Script to read text file and create .db and .pool files with status constants 		        ##
##													##
##													##
##													##
##     You have to set up the tag that you use to write mycool.db and pooloutputfile.root		## 
##													##
##													##
##########################################################################################################

import AthenaCommon.AtlasUnixStandardJob
tagList         = [ "TRTCondStatusPermanent-TEST-00-00"]
objectList      = [ "TRTCond::StrawStatusMultChanContainer#/TRT/Cond/StatusPermanent"]

inputfilePer = "/afs/cern.ch/user/i/idcalib/w0/TRT_Calibration/uploadedDB/Status/2015_07_15_PermanentDeadList/StrawsToMask_Boards.txt"

# only one event needed
from AthenaCommon.AppMgr import theApp
theApp.EvtMax = 1

# setup for data or mc
from AthenaCommon.GlobalFlags import globalflags

globalflags.DetGeo.set_Value_and_Lock("atlas")
globalflags.DataSource.set_Value_and_Lock("geant4")
#globalflags.DataSource.set_Value_and_Lock("data")

from IOVDbSvc.CondDB import conddb
conddb.setGlobalTag("OFLCOND-MC16-SDR-28")
#conddb.setGlobalTag("CONDBR2-BLKPA-2018-06")
#svcMgr.IOVDbSvc.forceRunNumber=290000

# block the folders that are being read from text file
conddb.blockFolder("/TRT/Cond/StatusPermanent" )

from AthenaCommon.AppMgr import ServiceMgr
ServiceMgr.IOVSvc.preLoadData = True

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
TRTCondStream=AthenaOutputStreamTool(name="CondStream2",OutputFile="trtcalibout.pool.root")
ToolSvc += TRTCondStream

from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTStrawStatusWrite
TSW=TRTStrawStatusWrite(name="TSW")
TSW.StatusInputFile		=""
TSW.StatusInputFileHT		=""
TSW.StatusInputFilePermanent	=inputfilePer
topSequence +=TSW

outputFile = "pooloutputfile.root"

from RegistrationServices.OutputConditionsAlg import OutputConditionsAlg
myOCA=OutputConditionsAlg(outputFile=outputFile)
myOCA.ObjectList=objectList
myOCA.IOVTagList=tagList
myOCA.WriteIOV=True

myOCA.Run1=0
myOCA.LB1=0
myOCA.Run2=2147483647
myOCA.LB2=0

topSequence+=myOCA

