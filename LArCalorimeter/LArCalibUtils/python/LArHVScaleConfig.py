# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod


def LArHVScaleCfg(configFlags):
    result=ComponentAccumulator()

    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg, LArBadFebCfg

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(configFlags))

    from LArCabling.LArHVCablingConfig import LArHVCablingCfg
    result.merge(LArHVCablingCfg(configFlags))

    result.merge(LArBadFebCfg(configFlags))
    
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    LArHVCondAlg=CompFactory.LArHVCondAlg

    if configFlags.Input.isMC:
        result.addCondAlgo(LArHVCondAlg(doHV=False, doAffectedHV=False))

    elif not configFlags.Common.isOnline:
        result.merge(addFolders(configFlags,["/LAR/DCS/HV/BARREl/I16"], "DCS_OFL", className="CondAttrListCollection"))
        result.merge(addFolders(configFlags,["/LAR/DCS/HV/BARREL/I8"],  "DCS_OFL", className="CondAttrListCollection"))

        result.merge(addFolders(configFlags,["/LAR/HVPathologiesOfl/Pathologies"], "LAR_OFL", className="AthenaAttributeList"))
        if configFlags.GeoModel.Run is not LHCPeriod.Run1:
            result.merge(addFolders(configFlags,["/LAR/HVPathologiesOfl/Rvalues"], "LAR_OFL", className="AthenaAttributeList"))

        result.merge(LArBadChannelCfg(configFlags))

        LArHVPathologyDbCondAlg=CompFactory.LArHVPathologyDbCondAlg
        hvpath = LArHVPathologyDbCondAlg(PathologyFolder="/LAR/HVPathologiesOfl/Pathologies",
                                         HVMappingKey="LArHVIdMap",
                                         HVPAthologyKey="LArHVPathology")
        result.addCondAlgo(hvpath)

        from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
        result.merge(LArElecCalibDBCfg(configFlags,["HVScaleCorr",]))

        if configFlags.GeoModel.Run is not LHCPeriod.Run1:
           hvcond = LArHVCondAlg(HVPathologies="LArHVPathology")
        else:   
           hvcond = LArHVCondAlg(HVPathologies="LArHVPathology",doR=False)

        hvcond.UndoOnlineHVCorr=True
        hvcond.keyOutputCorr= "LArHVScaleCorrRecomputed"
        result.addCondAlgo(hvcond)

    return result

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags=initConfigFlags()

    nThreads=1
    flags.Concurrency.NumThreads = nThreads
    if nThreads>0:
        flags.Scheduler.ShowDataDeps = True
        flags.Scheduler.ShowDataFlow = True
        flags.Scheduler.ShowControlFlow = True
        flags.Concurrency.NumConcurrentEvents = nThreads

    flags.Input.Files = ["myESD-data.pool.root"]
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg=MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    cfg.merge( LArHVScaleCfg(flags) )

    cfg.run(10)
