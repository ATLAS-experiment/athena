# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
from LArConfiguration.LArConfigFlags import RawChannelSource 


def LArRawDataReadingCfg(flags, **kwargs):
    acc=ComponentAccumulator()
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))
    acc.merge(ByteStreamReadCfg(flags))

    if flags.Common.ProductionStep is ProductionStep.MinbiasPreprocessing:
        kwargs.setdefault("LArDigitKey", f"{flags.Overlay.BkgPrefix}LArDigitContainer_data")
        kwargs.setdefault("LArFebHeaderKey", "LArFebHeader")
    if flags.LAr.RawChannelSource is RawChannelSource.Calculated or flags.Overlay.DataOverlay:
        kwargs.setdefault("LArRawChannelKey", "")

    print('LArRawDataReadingCfg flags.LAr.RawChannelSource ',flags.LAr.RawChannelSource)

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

