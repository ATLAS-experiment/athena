##########################################################################################################
##													##
##													##
##     Script to read any db or tag and create .txt with the 					##
##     with the payload calibration contants                				##
##													##
##     if db is mycool.db, its Pool file should be inserted in the catalog:				##
##            -pool_insertFileToCatalog  pooloutputfile.root	                                        ##
##													##
##     Output file:											##
##						   -caliboutput.txt					##
##													##
##     You need to set up the tag that you want to read and the DB it is read from (eg mycool.db)       ## 
##													##
##########################################################################################################

# CHOOSE tag
#T0Tag="TrtCalibT0-MC-run2-run3_00-01"
#RtTag="TrtCalibRt-MC-run2-run3_00-01"
T0Tag="TrtCalibT0-RUN2-Physics-BLK-UPD4-00-03"
RtTag="TrtCalibRt-RUN2-Physics-BLK-UPD4-00-03"

# CHOOSE MC or data
isMC=False
from IOVDbSvc.CondDB import conddb
# folders to dump
conddb.blockFolder("/TRT/Calib/T0")
conddb.blockFolder("/TRT/Calib/RT")
if not isMC:
    conddb.addFolderWithTag('','<dbConnection>COOLOFL_TRT/CONDBR2</dbConnection>/TRT/Calib/T0',T0Tag,force=True,className='TRTCond::StrawT0MultChanContainer')
    conddb.addFolderWithTag('','<dbConnection>COOLOFL_TRT/CONDBR2</dbConnection>/TRT/Calib/RT',RtTag,force=True,className='TRTCond::RtRelationMultChanContainer')
    #conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=CONDBR2</dbConnection>/TRT/Calib/T0',T0Tag,force=True,className='TRTCond::StrawT0MultChanContainer')
    #conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=CONDBR2</dbConnection>/TRT/Calib/RT',RtTag,force=True,className='TRTCond::RtRelationMultChanContainer')
    conddb.blockFolder("/Indet/Onl/Beampos")
    conddb.addFolderSplitOnline("INDET", "/Indet/Onl/Beampos", "/Indet/Beampos", className="AthenaAttributeList")
else:
    conddb.addFolderWithTag('','<dbConnection>COOLOFL_TRT/OFLP200</dbConnection>/TRT/Calib/T0',T0Tag,force=True,className='TRTCond::StrawT0MultChanContainer')
    conddb.addFolderWithTag('','<dbConnection>COOLOFL_TRT/OFLP200</dbConnection>/TRT/Calib/RT',RtTag,force=True,className='TRTCond::RtRelationMultChanContainer')
    #conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=OFLP200</dbConnection>/TRT/Calib/T0',T0Tag,force=True,className='TRTCond::StrawT0MultChanContainer')
    #conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=OFLP200</dbConnection>/TRT/Calib/RT',RtTag,force=True,className='TRTCond::RtRelationMultChanContainer')

# Set this if you have several run ranges in the chosen db
svcMgr.IOVDbSvc.forceRunNumber=456346

from AthenaCommon.AlgSequence import AthSequencer
condSeq = AthSequencer("AthCondSeq")
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

from AthenaCommon.AppMgr import ServiceMgr as svcMgr

from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTCondRead
TRTCondRead = TRTCondRead( name = "TRTCondRead",
                          CalibOutputFile="caliboutput.txt")
topSequence+=TRTCondRead





