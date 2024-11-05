#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def LArDigits2NtupleCfg(flags, isEMF=False, **kwargs):
       cfg=ComponentAccumulator()
       if 'isSC' in kwargs and kwargs['isSC']:
          if isEMF:
             # FIXME there could be also another digits types....
             cfg.addEventAlgo(CompFactory.LArRawSCCalibDataReadingAlg(LArSCAccCalibDigitKey = flags.LArSCDump.accdigitsKey,
                                                                       CalibCablingKeyLeg = "LArCalibLineMap",
                                                                       OnOffMapLeg = "LArOnOffIdMap",
                                                                       LATOMEDecoder = CompFactory.LArLATOMEDecoder("LArLATOMEDecoder")))
          else:
             from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
             cfg.merge(LArRawSCDataReadingCfg(flags))
       else:
          if isEMF:
             Digit = kwargs['ContainerKey'] if 'ContainerKey' in kwargs else "" 
             accDigit = kwargs['AccContainerKey'] if 'AccContainerKey' in kwargs else ""
             accCalibDigit = kwargs['AccCalibContainerKey'] if 'AccCalibContainerKey'in kwargs else ""
             from LArByteStream.LArRawCalibDataReadingConfig import LArRawCalibDataReadingCfg 
             cfg.merge(LArRawCalibDataReadingCfg(flags,gain = flags.LArSCDump.digitsKey,
                                                 doDigit = Digit,doAccDigit = accDigit, 
                                                 doAccCalibDigit = accCalibDigit)) 
          else:
             from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
             cfg.merge(LArRawDataReadingCfg(flags))

       cfg.addEventAlgo(CompFactory.LArDigits2Ntuple("LArDigits2Ntuple",**kwargs))

       return cfg
