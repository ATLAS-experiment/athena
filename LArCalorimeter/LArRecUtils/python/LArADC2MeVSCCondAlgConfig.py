# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def LArADC2MeVSCCondAlgCfg(flags):
    from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg, LArElecCalibDBMCSCCfg
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg

    result=ComponentAccumulator()
    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
    result.merge(LArOnOffIdMappingSCCfg(flags))
    result.merge(LArGMCfg(flags)) #Needed for identifier helpers

    theADC2MeVCondAlg=CompFactory.LArADC2MeVCondAlg(name = "LArADC2MeVSCCondAlg", LArADC2MeVKey = 'LArADC2MeVSC')

    if flags.Input.isMC:
        requiredConditions=["RampSC","DAC2uASC","uA2MeVSC"]
        theADC2MeVCondAlg.LAruA2MeVKey="LAruA2MeVSC"
        theADC2MeVCondAlg.LArDAC2uAKey="LArDAC2uASC"
        theADC2MeVCondAlg.LArRampKey="LArRampSC"
        theADC2MeVCondAlg.LArMphysOverMcalKey=""
        theADC2MeVCondAlg.LArHVScaleCorrKey=""
        theADC2MeVCondAlg.UseFEBGainTresholds=False
        theADC2MeVCondAlg.LArOnOffIdMappingKey="LArOnOffIdMapSC"
        theADC2MeVCondAlg.isSuperCell=True
        result.merge(LArElecCalibDBMCSCCfg(flags,requiredConditions))
    else: # not MC:
        requiredConditions=["Ramp","DAC2uA","uA2MeV","MphysOverMcal","HVScaleCorr"]
        from LArRecUtils.LArFebConfigCondAlgConfig import LArFebConfigCondAlgCfg
        if 'COMP200' in flags.IOVDb.DatabaseInstance: # Run1 case
            theADC2MeVCondAlg.LAruA2MeVKey="LAruA2MeVSC"
            theADC2MeVCondAlg.LArDAC2uAKey="LArDAC2uASC"
        result.merge(LArFebConfigCondAlgCfg(flags))
        result.merge(LArElecCalibDBCfg(flags,requiredConditions))
    result.addCondAlgo(theADC2MeVCondAlg,primary=True)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags

    print ('--- data')
    flags1 = initConfigFlags()
    flags1.Input.Files = defaultTestFiles.RAW_RUN2
    flags1.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags1.lock()
    acc1 = LArADC2MeVSCCondAlgCfg (flags1)
    acc1.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc1.getService('IOVDbSvc').Folders)
    acc1.wasMerged()

    print ('--- mc')
    flags2 = initConfigFlags()
    flags2.Input.Files = defaultTestFiles.ESD
    flags2.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags2.lock()

    acc2 = LArADC2MeVSCCondAlgCfg (flags2)
    acc2.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc2.getService('IOVDbSvc').Folders)
    acc2.wasMerged()
