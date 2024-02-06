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

tagList         = [ "TrtStrawStatusHT-Scenario7"]
objectList      = [ "TRTCond::StrawStatusMultChanContainer#TRT/Cond/StatusHT"]

inputfileHT  = "scenario7.txt"

# only one event needed
from AthenaCommon.AppMgr import theApp
theApp.EvtMax = 1


from AthenaCommon.GlobalFlags import globalflags

# setup for data or mc
globalflags.DetGeo.set_Value_and_Lock("atlas")
globalflags.DataSource.set_Value_and_Lock("geant4")
#globalflags.DataSource.set_Value_and_Lock("data")

from IOVDbSvc.CondDB import conddb
#choose data or mc
conddb.setGlobalTag("OFLCOND-MC16-SDR-28")
#conddb.setGlobalTag("CONDBR2-BLKPA-2022-10")
#svcMgr.IOVDbSvc.forceRunNumber=290000

# block the folders that are being read from text file
conddb.blockFolder("/TRT/Cond/StatusHT" )

# turn on trt
from AthenaCommon.DetFlags import DetFlags
DetFlags.TRT_setOn()
DetFlags.detdescr.TRT_setOn()

from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit

IOVSvc = Service("IOVSvc")
IOVSvc.preLoadData = True


#turn on output services and algorithms
include ( "DetDescrCondAthenaPool/DetDescrCondAthenaPool_joboptions.py" )
include("RegistrationServices/RegistrationServices_jobOptions.py")
svcMgr.MessageSvc.OutputLevel      = 3

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

from AthenaServices.AthenaServicesConf import AthenaOutputStreamTool
TRTCondStream=AthenaOutputStreamTool(name="CondStream1",OutputFile="tpooloutputfile.root")
ToolSvc += TRTCondStream

from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTStrawStatusWrite
TSW=TRTStrawStatusWrite(name="TSW")
TSW.StatusInputFile		=""
TSW.StatusInputFileHT		=inputfileHT
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

