# from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaCommon.Constants import DEBUG, VERBOSE
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import (
    defaultConditionsTags,
    defaultGeometryTags,
    defaultTestFiles,
)
from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg

flags = initConfigFlags()

# This is completely dummy input to get event loop going
flags.Input.Files = defaultTestFiles.RAW_RUN3_DATA22
flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA22

flags.Exec.MaxEvents = -1

# flags.Exec.OutputLevel = VERBOSE
flags.Concurrency.NumThreads = 3
flags.Concurrency.NumOffloadThreads = 3
flags.lock()

LoopMgr = 'AthenaRemoteEventLoopMgr'
acc = MainServicesCfg(flags, LoopMgr=LoopMgr)
acc.merge(ByteStreamReadCfg(flags))
# acc.addService(CompFactory.TimelineSvc("TimelineSvc", RecordTimeline=True))

unpackEI = CompFactory.EventInfoPackagingTool("UnpackEI")
robsSvc = acc.addService(CompFactory.ROBDataProviderSvc())
# unpackBS = CompFactory.BSPackagingTool("UnpackBS", OutputLevel=DEBUG, ROBDataProvider=robsSvc)

execTool = CompFactory.ExecuteOngRPCCall(UnpackingTools=[unpackEI])

# TODO, this would be configured in advance, when ELMgr is configured
el = acc.getService(LoopMgr)
el.eventExecTool = execTool

# from TrigT2CaloCommon.TrigCaloDataAccessConfig import trigCaloDataAccessSvcCfg
# acc.merge(trigCaloDataAccessSvcCfg(flags))
# from CaloRec.CaloRecoConfig import CaloRecoCfg
# acc.merge(CaloRecoCfg(flags))

# from TileRecUtils.TileCellMakerConfig import TileCellMakerCfg
# acc.merge(TileCellMakerCfg(flags))


acc.run()
