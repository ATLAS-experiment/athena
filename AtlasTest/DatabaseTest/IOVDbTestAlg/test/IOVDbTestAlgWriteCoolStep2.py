# Copyright (C) 2002-2024 by CERN for the benefit of the ATLAS collaboration

from IOVDbTestAlg.IOVDbTestAlgConfig import IOVDbTestAlgFlags, IOVDbTestAlgWriteCfg

flags = IOVDbTestAlgFlags()
flags.Exec.MaxEvents = 25
flags.lock()

acc = IOVDbTestAlgWriteCfg(flags, registerIOV=True)
acc.getEventAlgo("IOVDbTestAlg").RegTime = 3
acc.getService("IOVRegistrationSvc").BeginRun = 3
acc.getService("IOVRegistrationSvc").BeginLB = 3
acc.getService("IOVRegistrationSvc").IOVDbTag = "DC1"

import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())
