from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
# from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaCommon.Constants import DEBUG

from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg

flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.RAW_RUN3 # this is completely dummy input to get event loop going
flags.Exec.MaxEvents = 10
# flags.Exec.OutputLevel=INFO
flags.Concurrency.NumThreads=3
flags.Concurrency.NumOffloadThreads=3
flags.lock()


acc = MainServicesCfg(flags)


# TODO, this should be configured in advance, when ELMgr is configure
robsSvc = acc.addService(CompFactory.ROBDataProviderSvc())
el =acc.getService("AthenaHiveEventLoopMgr")
unpackBS = CompFactory.BSPackagingTool("UnpackBS", OutputLevel=DEBUG, ROBDataProvider=robsSvc)
execTool = CompFactory.ExecuteOngRPCCall( PackagingTools=[unpackBS])
el.eventExecTool=execTool

acc.run()
