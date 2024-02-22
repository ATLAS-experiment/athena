# Copyright (C) 2002-2024 by CERN for the benefit of the ATLAS collaboration

from IOVDbSvc.IOVDbSvcConfig import addFolders
from IOVDbTestAlg.IOVDbTestAlgConfig import IOVDbTestAlgFlags, IOVDbTestAlgReadCfg

flags = IOVDbTestAlgFlags()
flags.Input.Files = ["SimpleEventPoolFile.root"]
flags.Exec.MaxEvents = -1
flags.lock()

acc = IOVDbTestAlgReadCfg(flags, overrideTag=False)

acc.merge(addFolders(flags, ["/Simulation/Parameters",
                             "/Digitization/Parameters"]))

import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())
