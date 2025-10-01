
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

def LArScIdVsIdTestCfg(flags):
    result=ComponentAccumulator()

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))

    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
    result.merge(LArOnOffIdMappingSCCfg(flags))
          
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    result.merge(LArOnOffIdMappingCfg(flags))

    result.addEventAlgo(CompFactory.LArSCIdvsIdTest())

    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    result.merge(McEventSelectorCfg(flags,
                                    RunNumber         = flags.Input.RunNumbers[0],
                                    EventsPerRun      = 1,
                                    FirstEvent        = 1,
                                    InitialTimeStamp  = 0,
                                    TimeStampInterval = 1))
    
    return result


if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags=initConfigFlags()
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
    flags.IOVDb.DatabaseInstance="CONDBR2"
    flags.LAr.doAlign=False
    flags.Input.Files=[]
    flags.Input.isMC=False
    flags.Input.RunNumbers=[483000,]
    flags.IOVDb.GlobalTag="CONDBR2-BLKPA-RUN2-09"
    flags.lock()
    cfg=MainServicesCfg(flags)
    cfg.merge(LArScIdVsIdTestCfg(flags))
    cfg.getService("MessageSvc").errorLimit=99999999
    cfg.run(1)
    
