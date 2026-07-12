#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaCommon.Constants import DEBUG

flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.RDO_RUN4 # this is completely dummy input to get event loop going
flags.Exec.MaxEvents = 10
flags.Exec.OutputLevel=DEBUG
flags.Concurrency.NumThreads=3
flags.lock()
acc = MainServicesCfg(flags)

acc.merge(PoolReadCfg(flags))

acc.addEventAlgo(CompFactory.AsyncgRPCComputeAlg("A1",
    OutputLevel=DEBUG ))

acc.addEventAlgo(CompFactory.AsyncgRPCComputeAlg("A2",
    OutputLevel=DEBUG ))

# ------------------------------------------------------------
# 5. Run
# ------------------------------------------------------------
if __name__ == "__main__":
    sc = acc.run()
    
    # exit code handling
    import sys
    sys.exit(not sc.isSuccess())