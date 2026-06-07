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
flags.lock()
acc = MainServicesCfg(flags)

acc.merge(PoolReadCfg(flags))

acc.addEventAlgo(CompFactory.TestSender("Sender1",
    OutputLevel=DEBUG,
    ValueToSend=7, 
    SizeToSend=12, 
    OutputKey="Data1"
))

acc.addEventAlgo(CompFactory.TestSender("Sender2",
    OutputLevel=DEBUG,
    ValueToSend=-20, 
    SizeToSend=2, 
    OutputKey="Data2"
))



acc.addEventAlgo(CompFactory.TestReciever("Reciever1",
    OutputLevel=DEBUG,
    InputKey="Data1" # this is the synchronisation mechanism between the Sender1 and this alg
))


acc.addEventAlgo(CompFactory.TestReciever("Reciever2",
    OutputLevel=DEBUG,
    InputKey="Data2" # this is the synchronisation mechanism between the Sender2 and this alg
))







# ------------------------------------------------------------
# 5. Run
# ------------------------------------------------------------
if __name__ == "__main__":
    sc = acc.run()
    
    # exit code handling
    import sys
    sys.exit(not sc.isSuccess())