# Copyright (C) 2002-2024 by CERN for the benefit of the ATLAS collaboration

from IOVDbTestAlg.IOVDbTestAlgConfig import IOVDbTestAlgFlags, IOVDbTestAlgWriteCfg

flags = IOVDbTestAlgFlags()
flags.Exec.MaxEvents = 25
flags.lock()

acc = IOVDbTestAlgWriteCfg(flags)
acc.getEventAlgo("IOVDbTestAlg").PrintLB = False

import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())
