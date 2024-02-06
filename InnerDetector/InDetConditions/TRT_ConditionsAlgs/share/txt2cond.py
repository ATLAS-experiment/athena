##########################################################################################################
##													##
##													##
##     Script to read a text file with calibration constants and publish them in CondStore              ##
##     where they can be accessed with TRT_CalDbTool. This script is useful for itertaions              ##
##                                                                                                      ##
##													##
##     The optional output file should be a copy of the input file (but not in same sequence):	        ##
##						   -caliboutput.txt or                                  ##
##													##
##########################################################################################################
constants= "dbconst_data23_start.txt"

# CHOOSE MC or data (remember also the RAW input file)
isMC=False; 
from IOVDbSvc.CondDB import conddb
# folders to replace
conddb.blockFolder("/TRT/Calib/T0")
conddb.blockFolder("/TRT/Calib/RT")
if not isMC:
    conddb.blockFolder("/Indet/Onl/Beampos")
    conddb.addFolderSplitOnline("INDET", "/Indet/Onl/Beampos", "/Indet/Beampos", className="AthenaAttributeList")

from AthenaCommon.AlgSequence import AthSequencer
condSeq = AthSequencer("AthCondSeq")
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTCondWrite
TRTCondWrite = TRTCondWrite( name = "TRTCondWrite",
                             CalibInputFile=constants)

condSeq += TRTCondWrite

# As a check, dump the folders in CondStore to text file (turned off here)
#from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_CalDbTool
#InDetTRTCalDbTool = TRT_CalDbTool(name = "TRT_CalDbTool")
#from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTCondPrint
#TRTCondPrint = TRTCondPrint( name = "TRTCondPrint",
#                             TRTCalDbTool=InDetTRTCalDbTool,
#                             CalibOutputFile="caliboutput.txt")
#topSequence+=TRTCondPrint







