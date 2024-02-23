# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

# Setup flags
flags = initConfigFlags()
flags.Common.MsgSuppression = False
flags.Input.Files = ["myStream1.pool.root"]
# to avoid input file peeking:
flags.Input.RunNumbers = [1]
flags.Input.ProcessingTags = []
flags.Input.TypedCollections = []
flags.lock()

# Main services
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)

# Pool reading and writing
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge( PoolReadCfg(flags) )

from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
acc.merge( PoolWriteCfg(flags) )

acc.addEventAlgo( CompFactory.AthenaPoolTestDataWriter(OutputLevel = DEBUG,
                                                       PartialCreate = True,
                                                       ReadOtherHalf = True),
                  sequenceName = 'AthAlgSeq' )

from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
acc.merge( OutputStreamCfg(
   flags,
   streamName = "Stream4",
   disableEventTag = True,
   ItemList = ["EventInfo#*",
               # Write out only a new set of cols 1,2,3, and matrix
               "IAthenaPoolTestCollection#AthenaPoolTestCollection_1",
               "IAthenaPoolTestCollection#AthenaPoolTestCollection_2",
               "IAthenaPoolTestCollection#AthenaPoolTestCollection_3",
               "AthenaPoolTestMatrix#*"]) )

# Run
import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())
