# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Utils import setupLoggingLevels
import sys

flags=initConfigFlags()
#Set a number of flags to avoid input-file peeking
flags.Input.isMC=True
flags.Input.RunNumbers=[1]
flags.LAr.doAlign=False
flags.Input.Files=["myEventInfoPoolFile5.pool.root",]
flags.Exec.DebugMessageComponents=["TagInfoMgr",
                                   "EventInfoReader",]


flags.lock()

    
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc=MainServicesCfg(flags)

from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge(PoolReadCfg(flags))

#Add some LAr and Tile conditions,  this is the payload obj for this test
from LArGeoAlgsNV.LArGMConfig import LArGMCfg
acc.merge(LArGMCfg(flags))


from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg, LArCalibIdMappingCfg,LArFebRodMappingCfg
acc.merge(LArOnOffIdMappingCfg(flags))
acc.merge(LArCalibIdMappingCfg(flags))
acc.merge(LArFebRodMappingCfg(flags))
    
from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
acc.merge(TileEMScaleCondAlgCfg(flags))

from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
acc.merge(TagInfoMgrCfg(flags))

acc.addEventAlgo(CompFactory.EventInfoReader(),sequenceName = 'AthAlgSeq')

# Change output file catalog to avoid races.
acc.getService("PoolSvc").WriteCatalog = 'file:EventInfoTests_catalog.xml'

#Tag differnces are intentional here
acc.getService("GeoModelSvc").IgnoreTagDifference = True 

setupLoggingLevels(flags,acc)

sys.exit(acc.run().isFailure())
