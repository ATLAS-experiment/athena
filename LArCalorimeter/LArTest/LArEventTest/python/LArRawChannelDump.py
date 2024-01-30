#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory

if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = ["myRDO.pool.root",]
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    cfg=MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    cfg.merge(LArOnOffIdMappingCfg(flags))
    cfg.addEventAlgo(CompFactory.DumpLArRawChannels(NtupStream="LARRC",OutputFileName="",ToLog=False))

    cfg.addService(CompFactory.THistSvc(Output = ["LARRC DATAFILE='LARRC.root', OPT='RECREATE'"]))

                   
    cfg.run(-1)

                    
