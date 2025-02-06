#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def LArDigits2NtupleCfg(flags, isEMF=False, isCalib=False, **kwargs):
       cfg=ComponentAccumulator()
       if 'isSC' in kwargs and kwargs['isSC']:
          from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
          cfg.merge(LArOnOffIdMappingSCCfg(flags))
          if kwargs['AddCalib']:
             from LArCabling.LArCablingConfig import LArCalibIdMappingSCCfg
             cfg.merge(LArCalibIdMappingSCCfg(flags))
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
          from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
          cfg.merge(LArOnOffIdMappingCfg(flags))
          if kwargs['AddCalib']:
             from LArCabling.LArCablingConfig import LArCalibIdMappingCfg
             cfg.merge(LArCalibIdMappingCfg(flags))
          if isCalib:
             from LArByteStream.LArRawCalibDataReadingConfig import LArRawCalibDataReadingCfg 
             if 'ContainerKey' in kwargs and kwargs['ContainerKey'] != "":
                Digit=True
                gain = kwargs['ContainerKey']
             else:
                Digit = False 
             if 'AccContainerKey' in kwargs and kwargs['AccContainerKey'] != "":
                accDigit = True
                gain = kwargs['AccContainerKey']
             else:
                accDigit = False
             if 'AccCalibContainerKey' in kwargs and kwargs['AccCalibContainerKey'] != "":
                accCalibDigit = True
                gain = kwargs['AccCalibContainerKey']
             else:
                accCalibDigit = False
             print('keys: c ',Digit, ' acc ',accDigit,' acccalib ', accCalibDigit)
             cfg.merge(LArRawCalibDataReadingCfg(flags,gain = gain,
                                                  doDigit = Digit, doAccDigit = accDigit, 
                                                  doAccCalibDigit = accCalibDigit)) 
          else:
             from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
             cfg.merge(LArRawDataReadingCfg(flags))

       cfg.addEventAlgo(CompFactory.LArDigits2Ntuple("LArDigits2Ntuple",**kwargs))

       return cfg
