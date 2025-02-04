# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

def LArRawSCDataReadingCfg(configFlags, ROBList=None, name="LArRawSCDataReadingAlg", **kwargs):
    acc=ComponentAccumulator()
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(configFlags))
    from LArCabling.LArCablingConfig import LArLATOMEMappingCfg
    acc.merge(LArLATOMEMappingCfg(configFlags))

    if ( not (ROBList is None) ):
       acc.addEventAlgo(CompFactory.LArRawSCDataReadingAlg(name,
                     LATOMEDecoder = CompFactory.LArLATOMEDecoder("LArLATOMEDecoder",ProtectSourceId = True), 
                     ROBList=ROBList,
                     **kwargs)
                    )
    else : 
       acc.addEventAlgo(CompFactory.LArRawSCDataReadingAlg(name,
                     LATOMEDecoder = CompFactory.LArLATOMEDecoder("LArLATOMEDecoder",ProtectSourceId = True), 
                     **kwargs)
                    )

    return acc

def LArRawSCDataReadingInRoICfg(configFlags,name="LArRawSCDataReadingInRoI",etIdCollKey="SC_ET_ID_RoI",ROBList=[]):
    return LArRawSCDataReadingCfg(configFlags,name=name,adcCollKey="",adcBasCollKey="",etCollKey="",LArLATOMEHeaderKey="",etIdCollKey=etIdCollKey,ROBList=ROBList)


if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags=initConfigFlags()
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.LAr.doAlign=False
    flags.Exec.OutputLevel=DEBUG
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()

    acc = MainServicesCfg( flags )
    acc.merge(ByteStreamReadCfg(flags))
    acc.merge(LArRawSCDataReadingCfg(flags))
    
    acc.run(2)

