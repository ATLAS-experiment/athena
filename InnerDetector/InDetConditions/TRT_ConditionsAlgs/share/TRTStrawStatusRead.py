##########################################################################################################
##													##
##													##
##     Script to read .db and .pool files and create .txt to check if 					##
##     the opoosite conversion has being done properly                                                  ##
##     or to dump the payload of an already merged tag 		      				        ##
##													##
##     If you have copied the pooloutputfile.root from somewhere, then do				##
##                   pool_insertFileToCatalog  pooloutputfile.root	                                ##
##													##
##     Output file:  StatusDump_Writer.txt	                                 			##
##													##
##     You need to set up the tag that you want to read                                                 ## 
##              to switch between data and MC                                                           ##
##              to choose the folder and from which DB the tag is read                                  ##
##													##
##########################################################################################################
#COOSE data or MC
isMC=False

#CHOOSE tag to be read and printed
tag         = "TRTStrawStatus-Test"
# standard tags for Status, StatusPermanent and StatusHT folders
#tag="TRTCondStatus-RUN2-BLK-UPD2-01-01"    
#tag="TRTCondStatusPermanent-RUN2-BLK-UPD4-02-00"
#tag="TrtStrawStatusHT-RUN2-Reprocessing"

# tags for alignment folders in data
DXTag="TRTCalibDX-RUN2-BLK-UPD4-03"
TRTL1Tag="TRTAlignL1_Run2_Legacy_looser"
TRTL2Tag="TRTAlignL2_Run2_Legacy_MDN_FullChain_GlobalSagitta"
IDL1Tag="IndetAlignL1ID_Run2_Legacy_looser"
PIXL2Tag="IndetAlignL2PIX_Run2_Legacy_looser"
SCTL2Tag="IndetAlignL2SCT_Run2_Legacy_looser"
IDL3Tag="IndetAlignL3_Run2_Legacy_MDN_FullChain_GlobalSagitta"

from AthenaCommon.GlobalFlags import globalflags
globalflags.DetGeo.set_Value_and_Lock("atlas")

# need to read one event
from AthenaCommon.AppMgr import theApp
theApp.EvtMax = 1



if not isMC:
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
    #conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=InDetAlignment.db;dbname=CONDBR2</dbConnection>/TRT/Cond/StatusHT',StatHTTag,className='TRTCond::StrawStatusMultChanContainer',force=True)
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTAlignCondAlg
    TRTAlignCondAlg = TRTAlignCondAlg(name = "TRTAlignCondAlg",UseDynamicFolders = True)
    TRTAlignCondAlg.ReadKeyDynamicGlobal="/TRT/AlignL1/TRT"
    TRTAlignCondAlg.ReadKeyDynamicRegular="/TRT/AlignL2"

#----> you may want to read some particular data
#    from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
#    athenaCommonFlags.FilesInput =["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data17_13TeV.00330470.physics_Main.daq.RAW._lb0310._SFO-1._0001.data"]
# -----------> End case of real data

if isMC:
    globalflags.DataSource.set_Value_and_Lock("geant4")
    from IOVDbSvc.CondDB import conddb
    conddb.setGlobalTag("OFLCOND-MC16-SDR-25")
# ----> you may want to read some particular data
#    athenaCommonFlags.FilesInput =["'root://eosatlas.cern.ch//eos/user/m/martis/data/InputFileForGridJobs/ZmumuMC16_AOD.18379878._000123.pool.root.1"]
# -----------> End case of MC

#CHOOSE which folder to block and instead take from local DB
conddb.blockFolder("/TRT/Cond/Status")
#conddb.blockFolder("/TRT/Cond/StatusPermanent")
#conddb.blockFolder("/TRT/Cond/StatusHT")




#turn on TRT
from AthenaCommon.DetFlags import DetFlags
DetFlags.TRT_setOn()
DetFlags.detdescr.TRT_setOn()

from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_StrawStatusSummaryTool
InDetStrawSummaryTool=TRT_StrawStatusSummaryTool(name = "TRT_StrawStatusSummaryTool",isGEANT4=isMC)

from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit

include ( "DetDescrCondAthenaPool/DetDescrCondAthenaPool_joboptions.py" )
include("RegistrationServices/RegistrationServices_jobOptions.py")

#CHOOSE a run in the range you are interested in
svcMgr.IOVDbSvc.forceRunNumber=310000



svcMgr.UseGlobalIOVForCollections = True
svcMgr.MessageSvc.OutputLevel      = INFO


from AthenaCommon.AppMgr import ServiceMgr
ServiceMgr.IOVSvc.preLoadData = True


from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTStrawStatusRead
TSR=TRTStrawStatusRead(name="TRTStrawStatusRead")


#CHOOSE here either Status, StatusHT or StatusPermanent
TSR.FolderToPrint = "Status" 
#TSR.FolderToPrint = "StatusPermanent" 
#TSR.FolderToPrint = "StatusHT" 
topSequence +=TSR

#CHOOSE database and tag

if isMC:
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=OFLP200</dbConnection>/TRT/Cond/Status',tag,force=True,className='TRTCond::StrawStatusMultChanContainer')
else:
    conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=CONDBR2</dbConnection>/TRT/Cond/Status',tag,force=True,className='TRTCond::StrawStatusMultChanContainer')
#conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=OFLP200</dbConnection>/TRT/Cond/StatusPermanent',tagPer,force=True,className='TRTCond::StrawStatusMultChanContainer');
#conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=OFLP200</dbConnection>/TRT/Cond/StatusHT',tagHT,force=True,className='TRTCond::StrawStatusMultChanContainer')
#conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=OFLP200</dbConnection>/TRT/Cond/StatusHT',tagHT,force=True,className='TRTCond::StrawStatusMultChanContainer')
#conddb.addFolderWithTag('','<dbConnection>sqlite://;schema=mycool.db;dbname=OFLP200</dbConnection>/TRT/Cond/StatusHT',tagHT,force=True)
#conddb.addFolderWithTag('','<dbConnection>COOLOFL_TRT/OFLP200</dbConnection>/TRT/Cond/StatusHT',tagHT,force=True,className='TRTCond::StrawStatusMultChanContainer');
#conddb.addFolderWithTag('','<dbConnection>COOLOFL_TRT/CONDBR2</dbConnection>/TRT/Cond/StatusHT',tagHT,force=True,className='TRTCond::StrawStatusMultChanContainer');






