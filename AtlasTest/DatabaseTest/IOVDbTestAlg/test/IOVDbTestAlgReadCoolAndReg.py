# Copyright (C) 2002-2024 by CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG
from IOVDbTestAlg.IOVDbTestAlgConfig import IOVDbTestAlgFlags, IOVDbTestAlgReadCfg

flags = IOVDbTestAlgFlags()
flags.Exec.MaxEvents = 30
flags.lock()

acc = IOVDbTestAlgReadCfg(flags)
acc.getEventAlgo("IOVDbTestAlg").TwoStepWriteReg = True
acc.getEventAlgo("IOVDbTestAlg").PrintLB = False
acc.getEventAlgo('IOVDbTestAlg').RegisterIOV = True
acc.getEventAlgo('IOVDbTestAlg').TagID = "COOL-TEST-001"

acc.addService( CompFactory.IOVRegistrationSvc(OutputLevel = DEBUG) )

from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
acc.merge( PoolWriteCfg(flags) )

from EventSelectorAthenaPool.CondProxyProviderConfig import CondProxyProviderCfg
acc.merge( CondProxyProviderCfg (flags, ["SimplePoolFile.root"]) )

import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())
