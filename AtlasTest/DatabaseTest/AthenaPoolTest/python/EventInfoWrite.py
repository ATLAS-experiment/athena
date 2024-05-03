# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.TestDefaults import defaultGeometryTags
from AthenaConfiguration.Utils import setupLoggingLevels
from Campaigns.Utils import Campaign
import sys

#Set a number of flags to avoid input-file peeking
flags=initConfigFlags()
flags.Input.isMC=True
flags.Input.RunNumbers=[1]
flags.Input.TimeStamps=[0]
flags.IOVDb.GlobalTag = 'OFLCOND-SDR-BS14T-IBL-06'
flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN2
flags.LAr.doAlign=False
flags.Input.MCCampaign=Campaign.Unknown
flags.Input.TypedCollections=[]

flags.Exec.DebugMessageComponents=["TagInfoMgr",
                                   "EventSelector",
                                   "EventInfoWriter",
                                   "IOVDbMetaDataTool"]

flags.lock()
StreamName="EventInfoPoolFile1"

from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
acc=MainEvgenServicesCfg(flags)

from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
acc.merge(EventInfoCnvAlgCfg(flags, disableBeamSpot=True), sequenceName="AthAlgSeq")

#Add some LAr and Tile conditions,  this is the payload obj for this test
from LArGeoAlgsNV.LArGMConfig import LArGMCfg
acc.merge(LArGMCfg(flags))


from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg, LArCalibIdMappingCfg,LArFebRodMappingCfg
acc.merge(LArOnOffIdMappingCfg(flags))
acc.merge(LArCalibIdMappingCfg(flags))
acc.merge(LArFebRodMappingCfg(flags))

from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
acc.merge(TileEMScaleCondAlgCfg(flags))

EventSelector=acc.getService("EventSelector")
EventSelector.RunNumber         = 1
EventSelector.EventsPerRun      = 0x100000010
EventSelector.FirstEvent        = 0x100000000
EventSelector.EventsPerLB       = 1
EventSelector.FirstLB           = 1
EventSelector.InitialTimeStamp  = 0
EventSelector.TimeStampInterval = 5


from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
acc.merge(TagInfoMgrCfg(flags))

acc.addEventAlgo(CompFactory.EventInfoWriter(),sequenceName = 'AthAlgSeq')

from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
acc.merge(OutputStreamCfg(flags, StreamName, ItemList=[]))

# Change output file catalog to avoid races.
acc.getService("PoolSvc").WriteCatalog = 'file:EventInfoTests_catalog.xml'

setupLoggingLevels(flags,acc)

sys.exit(acc.run(25).isFailure())
