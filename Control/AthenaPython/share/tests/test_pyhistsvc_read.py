# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
## joboptions file to read ROOT objects via ITHistSvc

from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

from AthenaPython.tests.PyTHistTestsLib import PyHistReader
job += PyHistReader()

# define histsvc {in/out}put streams
from AthenaCommon.AppMgr import ServiceMgr as svcMgr
if not hasattr(svcMgr, 'THistSvc'):
    svcMgr += CfgMgr.THistSvc()
hsvc = svcMgr.THistSvc
hsvc.Input = [ "read1 DATAFILE='tuple1.root' OPT='READ'",
               "read2 DATAFILE='tuple2.root' OPT='READ'", ]
hsvc.PrintAll = True

from AthenaCommon.AppMgr import theApp
theApp.EvtMax = 1
