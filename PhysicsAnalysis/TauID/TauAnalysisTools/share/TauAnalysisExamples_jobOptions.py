# Set up the file reading:
import os
FNAME = os.environ['ASG_TEST_FILE_MC']

import AthenaPoolCnvSvc.ReadAthenaPool
from AthenaCommon.AppMgr import ServiceMgr
ServiceMgr.EventSelector.InputCollections = [ FNAME ]

# Access the algorithm sequence:
from AthenaCommon.AlgSequence import AlgSequence
theJob = AlgSequence()

#Add the test algorithm:
from TauAnalysisTools.TauAnalysisToolsConf import TauAnalysisTools__TauSelectionTool
TauSelTool = TauAnalysisTools__TauSelectionTool()
TauSelTool.PtMin = 20
theJob += TauSelTool

# Do some additional tweaking:
from AthenaCommon.AppMgr import theApp
theApp.EvtMax = 10
ServiceMgr.MessageSvc.defaultLimit = 1000000

