# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Utils import setupLoggingLevels

def eventInfoTestCfg(step=1):
    flags=initConfigFlags()
    #Set a number of flags to avoid input-file peeking
    flags.Input.isMC=True
    flags.Input.RunNumbers=[1]
    flags.LAr.doAlign=False

    flags.Exec.DebugMessageComponents=["TagInfoMgr",
                                   "EventInfoWriter",
                                   "IOVDbMetaDataTool"]

    if (step>0):
        flags.Input.Files=["myEventInfoPoolFile%i.pool.root" % (step-1),]

    StreamName="EventInfoPoolFile%i" % step
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


    writer=CompFactory.EventInfoWriter()
    if (step==3):
        writer.CreateDummyTags = True
    if (step==4):
        writer.CreateDummyOverrideTags = True
    if (step==5):
        writer.RemoveDummyTags = True

    acc.addEventAlgo(writer,sequenceName = 'AthAlgSeq')

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc.merge(OutputStreamCfg(flags, StreamName, ItemList=[]))

    # Change output file catalog to avoid races.
    acc.getService("PoolSvc").WriteCatalog = 'file:EventInfoTests_catalog.xml'

    #Tag differences are intentional here
    acc.getService("GeoModelSvc").IgnoreTagDifference = True 

    setupLoggingLevels(flags,acc)

    return acc.run().isFailure()
