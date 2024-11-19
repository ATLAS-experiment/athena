#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory

def LArSC2NtupleCfg(flags, isEmf=False, **kwargs):

       kwargs['isSC'] = True

       from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
       cfg = ByteStreamReadCfg(flags)

       if kwargs.get("ContainerKey","") != "" or "SC_ADC_BAS" in kwargs.get('SCContainerKeys') or "SC_ET" in kwargs.get('SCContainerKeys'):
          from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
          cfg.merge(LArRawSCDataReadingCfg(flags,OutputLevel=kwargs['OutputLevel']))
       else:
          theLArLATOMEDecoder = CompFactory.LArLATOMEDecoder("LArLATOMEDecoder")
          from LArCabling.LArCablingConfig import LArCalibIdMappingCfg,LArOnOffIdMappingCfg
          cfg.merge(LArOnOffIdMappingCfg(flags))
          cfg.merge(LArCalibIdMappingCfg(flags))
          cfg.addEventAlgo(CompFactory.LArRawSCCalibDataReadingAlg(LArSCAccCalibDigitKey = flags.LArSCDump.acccalibdigitsKey,
                                                                      LArSCAccDigitKey = flags.LArSCDump.accdigitsKey,
                                                                      CalibCablingKeyLeg="LArCalibLineMap",
                                                                      OnOffMapLeg="LArOnOffIdMap",
                                                                      LATOMEDecoder = theLArLATOMEDecoder, ))

       from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg,LArCalibIdMappingSCCfg,LArLATOMEMappingCfg
       cfg.merge(LArOnOffIdMappingSCCfg(flags))
       cfg.merge(LArCalibIdMappingSCCfg(flags))
       cfg.merge(LArLATOMEMappingCfg(flags))
       if isEmf:
          # hack for different mapping from EMF
          cil=cfg.getCondAlgo('CondInputLoader')
          iovdbsvc=cfg.getService('IOVDbSvc') 
          folder='/LAR/Identifier/LatomeMapping'
          for i in range(0,len(iovdbsvc.Folders)):
             if (iovdbsvc.Folders[i].find(folder)>=0):
                del iovdbsvc.Folders[i]
                break
     
          remove_folder = False
          for cil_Loadval in cil.Load:
             if folder in cil_Loadval:
                print(f"Removing {cil_Loadval} from cil/Load")
                remove_folder = True
                break
          if remove_folder: cil.Load.remove(cil_Loadval)
          from IOVDbSvc.IOVDbSvcConfig import addFolders
          cfg.merge(addFolders(flags,'/LAR/Identifier/LatomeMapping',tag='LARIdentifierLatomeMapping-EMF',className="CondAttrListCollection",detDb='/afs/cern.ch/user/p/pavol/w0/public/DB_update_24/SCcalib/LatomeMapping_EMF.db'))

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


