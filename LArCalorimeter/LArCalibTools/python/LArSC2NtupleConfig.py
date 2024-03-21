#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def LArSC2NtupleCfg(flags, **kwargs):

       kwargs['isSC'] = True

       cfg=ComponentAccumulator()

       from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
       cfg.merge(LArRawSCDataReadingCfg(flags,OutputLevel=kwargs['OutputLevel']))

       from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg,LArCalibIdMappingSCCfg,LArLATOMEMappingCfg
       cfg.merge(LArOnOffIdMappingSCCfg(flags))
       cfg.merge(LArCalibIdMappingSCCfg(flags))
       cfg.merge(LArLATOMEMappingCfg(flags))

       if flags.LArSCDump.doRawChan:
          from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
          cfg.merge(LArRawDataReadingCfg(flags))
          from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
          cfg.merge(LArOnOffIdMappingCfg(flags))
          from LArConfiguration.LArConfigFlags import RawChannelSource
          if flags.LAr.RawChannelSource is RawChannelSource.Calculated:
             from LArROD.LArRawChannelBuilderAlgConfig import LArRawChannelBuilderAlgCfg
             cfg.merge(LArRawChannelBuilderAlgCfg(flags))

             cfg.getEventAlgo("LArRawChannelBuilder").LArRawChannelKey="LArRawChannels"

       if 'FillLB' in kwargs and kwargs['FillLB']: #we are filling per event tree
          from LArCellRec.LArTimeVetoAlgConfig import LArTimeVetoAlgCfg
          cfg.merge(LArTimeVetoAlgCfg(flags))
          if flags.LArSCDump.fillNoisyRO: # should also config reco
             from CaloRec.CaloRecoConfig import CaloRecoCfg
             cfg.merge(CaloRecoCfg(flags))
             from LArCellRec.LArNoisyROSummaryConfig import LArNoisyROSummaryCfg
             cfg.merge(LArNoisyROSummaryCfg(flags))      

       if 'FillTriggerTowers' in kwargs and kwargs['FillTriggerTowers']: #confiigure decoding
          from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamDecoderCfg
          from TrigT1CaloByteStream.LVL1CaloRun2ByteStreamConfig import LVL1CaloRun2ReadBSCfg
          cfg.merge(L1TriggerByteStreamDecoderCfg(flags) )
          cfg.merge(LVL1CaloRun2ReadBSCfg(flags))
          from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg, HLTConfigSvcCfg, L1PrescaleCondAlgCfg, HLTPrescaleCondAlgCfg
          cfg.merge( L1ConfigSvcCfg(flags) )
          cfg.merge( HLTConfigSvcCfg(flags) )
          cfg.merge( L1PrescaleCondAlgCfg(flags) )
          cfg.merge( HLTPrescaleCondAlgCfg(flags) )

       alg=CompFactory.LArSC2Ntuple('LArSC2Ntuple',**kwargs)

       print(alg)

       cfg.addEventAlgo(alg)

       return cfg


