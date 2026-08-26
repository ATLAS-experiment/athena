from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags, defaultConditionsTags
# from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaCommon.Constants import DEBUG, VERBOSE

from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg

flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.RAW_RUN3_DATA22 # this is completely dummy input to get event loop going
flags.GeoModel.AtlasVersion =  defaultGeometryTags.RUN3
flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA22

flags.Exec.MaxEvents = -1

# flags.Exec.OutputLevel=INFO
flags.Concurrency.NumThreads=3
flags.Concurrency.NumOffloadThreads=3
flags.lock()


acc = MainServicesCfg(flags, forceRemoteELMgr=True)


# TODO, this would be configured in advance, when ELMgr is configured
robsSvc = acc.addService(CompFactory.ROBDataProviderSvc())
el = acc.getService("AthenaRemoteEventLoopMgr")
el.OutputLevel=VERBOSE
unpackEI = CompFactory.EventInfoPackagingTool("UnpackEI")
unpackBS = CompFactory.BSPackagingTool("UnpackBS", OutputLevel=DEBUG, ROBDataProvider=robsSvc)

execTool = CompFactory.ExecuteOngRPCCall( UnpackingTools=[unpackEI, unpackBS])
el.eventExecTool=execTool

from TrigT2CaloCommon.TrigCaloDataAccessConfig import trigCaloDataAccessSvcCfg
acc.merge(trigCaloDataAccessSvcCfg(flags))
from CaloRec.CaloRecoConfig import CaloRecoCfg
acc.merge(CaloRecoCfg(flags))

# from TileRecUtils.TileCellMakerConfig import TileCellMakerCfg
# acc.merge(TileCellMakerCfg(flags))


acc.run()
