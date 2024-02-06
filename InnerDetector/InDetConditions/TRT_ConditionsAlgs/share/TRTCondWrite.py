##########################################################################################################
##													##
##													##
##     Script to read a text file with calibration constants and publish them in CondStore              ##
##     where they can be accessed with TRT_CalDbTool. This script is useful for itertaions              ##
##                                                                                                      ##
##													##
##     The optional output file should be a copy of the input file (but not in same sequence):	        ##
##						   -caliboutput.txt or                                  ##
##                                                 -erroroutput.txt					##
##													##
##													##
##													##
##########################################################################################################


constants= "dbconst_HTcorrection.txt"
DXTag = "TRTCalibDX-RUN2-BLK-UPD4-03"
TRTL1Tag="TRTAlignL1_Run2_Legacy_looser"
TRTL2Tag="TRTAlignL2_Run2_Legacy_MDN_FullChain_GlobalSagitta"
IDL1Tag="IndetAlignL1ID_Run2_Legacy_looser"
PIXL2Tag="IndetAlignL2PIX_Run2_Legacy_looser"
SCTL2Tag="IndetAlignL2SCT_Run2_Legacy_looser"
IDL3Tag="IndetAlignL3_Run2_Legacy_MDN_FullChain_GlobalSagitta"
StatTag="TRTCondStatus-RUN2-BLK-UPD2-01-01"
StatPermTag="TRTCondStatusPermanent-RUN2-BLK-UPD4-02-00"
StatHTTag="TrtStrawStatusHT-RUN2-Reprocessing"



from AthenaCommon.GlobalFlags import globalflags
globalflags.DetGeo.set_Value_and_Lock("atlas")
theApp.EvtMax = 1
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags

# --------------->CHOOSE data or mc
isdata=True
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

# --------------->Case of real data 
if isdata :
# you may want to read some particular data

#    athenaCommonFlags.FilesInput =["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data17_13TeV.00330470.physics_Main.daq.RAW._lb0310._SFO-1._0001.data"]
    globalflags.DataSource.set_Value_and_Lock("data")

    conddb.setGlobalTag("CONDBR2-BLKPA-2022-10")
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
# (for some reason the alignment folders, needed for geometry, cannot be read for data from ORACLE in this script)

# block reading R-t folders from CondStore. Take them from "constants" instead.
conddb.blockFolder("/TRT/Calib/T0")
conddb.blockFolder("/TRT/Calib/RT")

#turn on TRT
from AthenaCommon.DetFlags import DetFlags
DetFlags.TRT_setOn()
DetFlags.detdescr.TRT_setOn()

from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit


from AthenaCommon.AppMgr import ServiceMgr as svcMgr
svcMgr.MessageSvc.OutputLevel      = INFO
svcMgr.IOVSvc.preLoadData = True
#svcMgr.IOVDbSvc.forceRunNumber=430000


from AthenaCommon.AlgSequence import AthSequencer
condSeq = AthSequencer("AthCondSeq")
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()



 # Setup the CondAlg that transfers the folders in the text file to CondStore
from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_CalDbTool
InDetTRTCalDbTool = TRT_CalDbTool(name = "TRT_CalDbTool")

from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTCondWrite
TRTCondWrite = TRTCondWrite( name = "TRTCondWrite",
                             CalibInputFile=constants)
if not hasattr(condSeq, "TRTAlignCondAlg"):
    condSeq += TRTAlignCondAlg
condSeq+=TRTCondWrite

# Setup the algorithem that dumps the folders in CondStore to text file
from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTCondPrint
TRTCondPrint = TRTCondPrint( name = "TRTCondPrint",
                             TRTCalDbTool=InDetTRTCalDbTool,
                             CalibOutputFile="caliboutput.txt")
topSequence+=TRTCondPrint



