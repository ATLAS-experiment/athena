# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
## Joboptions file to write ROOT objects via ITHistSvc

from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

from AthenaPython.tests.PyTHistTestsLib import PyHistWriter
job += PyHistWriter()

# define histsvc {in/out}put streams
from AthenaCommon.AppMgr import ServiceMgr as svcMgr
if not hasattr(svcMgr, 'THistSvc'):
    svcMgr += CfgMgr.THistSvc()
hsvc = svcMgr.THistSvc
hsvc.Output = [ "upd DATAFILE='tuple1.root' OPT='UPDATE'",
                "rec DATAFILE='tuple2.root' OPT='RECREATE'", ]
hsvc.PrintAll = True

from AthenaCommon.AppMgr import theApp
theApp.EvtMax = 20
