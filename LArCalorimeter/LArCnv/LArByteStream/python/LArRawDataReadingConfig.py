# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from LArConfiguration.LArConfigFlags import RawChannelSource 


def LArRawDataReadingCfg(configFlags, **kwargs):
    acc=ComponentAccumulator()
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(configFlags))
    acc.merge(ByteStreamReadCfg(configFlags))

    if configFlags.Overlay.DataOverlay:
        kwargs.setdefault("LArDigitKey", configFlags.Overlay.BkgPrefix + "FREE")
        kwargs.setdefault("LArFebHeaderKey", "LArFebHeader")
    if configFlags.LAr.RawChannelSource is RawChannelSource.Calculated or configFlags.Overlay.DataOverlay:
        kwargs.setdefault("LArRawChannelKey", "")

    print('LArRawDataReadingCfg configFlags.LAr.RawChannelSource ',configFlags.LAr.RawChannelSource)

    acc.addEventAlgo(CompFactory.LArRawDataReadingAlg(**kwargs))
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
    acc.merge(LArRawDataReadingCfg(flags))
    
    DumpLArRawChannels=CompFactory.DumpLArRawChannels
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg 
    acc.merge(LArOnOffIdMappingCfg(flags))
    acc.addEventAlgo(DumpLArRawChannels(LArRawChannelContainerName="LArRawChannels",))

    acc.run(2)

