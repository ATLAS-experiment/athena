# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

def LArRawSCDataReadingCfg(configFlags, **kwargs):
    acc=ComponentAccumulator()
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(configFlags))
    from LArCabling.LArCablingConfig import LArLATOMEMappingCfg
    acc.merge(LArLATOMEMappingCfg(configFlags))

    acc.addEventAlgo(CompFactory.LArRawSCDataReadingAlg("LArRawSCDataReadingAlg",
                     LATOMEDecoder = CompFactory.LArLATOMEDecoder("LArLATOMEDecoder",ProtectSourceId = True), 
                     **kwargs)
                    )
    return acc


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

