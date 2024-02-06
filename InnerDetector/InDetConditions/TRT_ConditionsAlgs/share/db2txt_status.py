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
StatusTag="TRTCondStatus-RUN2-BLK-UPD2-01-01"
StatPermTag="TRTStrawStatusPermanent-RUN2-BLK-UPD4-02-00"
StatHTTag="TrtStrawStatusHT-RUN2-BLK-UPD4-02-00"

# CHOOSE MC or data (remember also to change the RAW input file to be either data or MC)
isMC=False
from IOVDbSvc.CondDB import conddb
# CHOOSE db and folder to dump
conddb.blockFolder("/TRT/Cond/Status")
#conddb.blockFolder("/TRT/Cond/StatusPermanent")
#conddb.blockFolder("/TRT/Cond/StatusHT")
if not isMC:
    conddb.addFolderWithTag('','<dbConnection>COOLOFL_TRT/CONDBR2</dbConnection>/TRT/Cond/Status',StatusTag,force=True,className='TRTCond::StrawStatusMultChanContainer')
    #conddb.addFolderWithTag('','<dbConnection>COOLOFL_TRT/CONDBR2</dbConnection>/TRT/Cond/StatusPermanent',StatPermTag,force=True,className='TRTCond::StrawStatusMultChanContainer')
    #conddb.addFolderWithTag('','<dbConnection>COOLOFL_TRT/CONDBR2</dbConnection>/TRT/Cond/StatusHT',StatHTTag,force=True,className='TRTCond::StrawStatusMultChanContainer')
    #conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=CONDBR2</dbConnection>/TRT/Cond/Status',tag,force=True,className='TRTCond::StrawStatusMultChanContainer');
    conddb.blockFolder("/Indet/Onl/Beampos")
    conddb.addFolderSplitOnline("INDET", "/Indet/Onl/Beampos", "/Indet/Beampos", className="AthenaAttributeList")
else:
    conddb.addFolderWithTag('','<dbConnection>COOLOFL_TRT/OFLP200</dbConnection>/TRT/Cond/Status',T0Tag,force=True,className='TRTCond::StrawStatusMultChanContainer')

# Set this to CHOOSE a certain IoV in the chosen db
svcMgr.IOVDbSvc.forceRunNumber=456346

from AthenaCommon.AlgSequence import AthSequencer
condSeq = AthSequencer("AthCondSeq")
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

from AthenaCommon.AppMgr import ServiceMgr as svcMgr

from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTStrawStatusRead
TSR = TRTStrawStatusRead( name = "TRTStrawStatusRead")

#CHOOSE again
TSR.FolderToPrint = "Status" 
#TSR.FolderToPrint = "StatusPermanent" 
#TSR.FolderToPrint = "StatusHT" 
topSequence +=TSR





