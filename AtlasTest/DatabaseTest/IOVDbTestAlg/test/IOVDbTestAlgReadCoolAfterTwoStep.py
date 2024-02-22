# Copyright (C) 2002-2024 by CERN for the benefit of the ATLAS collaboration

from IOVDbTestAlg.IOVDbTestAlgConfig import IOVDbTestAlgFlags, IOVDbTestAlgReadCfg

flags = IOVDbTestAlgFlags()
flags.Exec.MaxEvents = 30
flags.lock()

acc = IOVDbTestAlgReadCfg(flags)
acc.getEventAlgo("IOVDbTestAlg").TwoStepWriteReg = True
acc.getEventAlgo("IOVDbTestAlg").PrintLB = False
acc.getService("EventSelector").EventsPerRun = 5

import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())
