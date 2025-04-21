# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory 

def LArShapeDumperCfg(flags):

    result=ComponentAccumulator()

    
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))

    #Setup cabling
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    result.merge(LArOnOffIdMappingCfg(flags))

    from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
    result.merge(LArRawDataReadingCfg(flags))

    from LArROD.LArRawChannelBuilderAlgConfig import LArRawChannelBuilderAlgCfg
    result.merge(LArRawChannelBuilderAlgCfg(flags))
    result.getEventAlgo("LArRawChannelBuilder").TimingContainerKey="LArOFIterResult"
    
    from LArCellRec.LArTimeVetoAlgConfig import LArTimeVetoAlgCfg
    result.merge(LArTimeVetoAlgCfg(flags))

    from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
    result.merge(BunchCrossingCondAlgCfg(flags))
    
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    result.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))

    from LArROD.LArFebErrorSummaryMakerConfig import LArFebErrorSummaryMakerCfg
    result.merge(LArFebErrorSummaryMakerCfg(flags))
    result.getEventAlgo("LArFebErrorSummaryMaker").CheckAllFEB=False

    from IOVDbSvc.IOVDbSvcConfig import addFolders 
    result.merge(addFolders(flags,
                            '/LAR/ElecCalibOfl/Shape/RTM/5samples3bins17phases<tag>LARElecCalibOflShapeRTM5samples3bins17phases-RUN2-UPD3-00</tag><key>LArShape17phases</key>',
                            'LAR_OFL'))

    result.getService("PoolSvc").ReadCatalog += ["apcfile:poolcond/PoolCat_comcond_castor.xml"]

    if flags.LArShapeDump.doTrigger:

        from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamDecoderCfg
        result.merge(L1TriggerByteStreamDecoderCfg(flags))

        from LArCafJobs.LArSCDumperSkeleton import L1CaloMenuCfg
        result.merge(L1CaloMenuCfg(flags))

        from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
        result.merge(TrigDecisionToolCfg(flags))

    
    result.merge(addFolders(flags,'/LAR/ElecCalibOfl/AutoCorrs/AutoCorr<tag>LARElecCalibOflAutoCorrsAutoCorr-RUN2-UPD3-00</tag>','LAR_OFL',className='LArAutoCorrComplete'))
    result.getService("IOVDbSvc").overrideTags+=['<prefix>/LAR/ElecCalibOfl/Shape/RTM/5samples1phase</prefix><tag>LARElecCalibOflShapeRTM5samples1phase-RUN2-UPD1-04</tag>']
    # for splashes: FIXME later
    result.getService("IOVDbSvc").overrideTags+=['<prefix>/LAR/ElecCalibOfl/OFC/PhysWave/RTM/4samples3bins17phases</prefix><tag>LARElecCalibOflOFCPhysWaveRTM4samples3bins17phases-RUN2-UPD3-00</tag>']
    result.getService("IOVDbSvc").overrideTags+=['<prefix>/LAR/ElecCalibOfl/Shape/RTM/4samples3bins17phases</prefix><tag>LARElecCalibOflShapeRTM4samples3bins17phases-RUN2-UPD3-00</tag>']

    if(flags.LArShapeDump.digitsKeySC != ""):
       #Setup cabling
       from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
       result.merge(LArOnOffIdMappingSCCfg(flags))
       from LArBadChannelTool.LArBadChannelConfig import  LArBadChannelCfg
       result.merge(LArBadChannelCfg(flags,isSC=True))
       from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBSCCfg
       result.merge(LArElecCalibDBSCCfg(flags,["Pedestal"]))
       from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
       result.merge(LArRawSCDataReadingCfg(flags))
               

       
    
    print("Dumping flags: ")
    flags.dump()
    dumperAlg=CompFactory.LArShapeDumper("LArShapeDumper")
    dumperAlg.CaloType = flags.LArShapeDump.caloType
    dumperAlg.Prescale = flags.LArShapeDump.prescale
    dumperAlg.NoiseSignifCut = flags.LArShapeDump.noiseSignifCut
    dumperAlg.DoTrigger = flags.LArShapeDump.doTrigger
    dumperAlg.DoStream = flags.LArShapeDump.doStream
    dumperAlg.DoOFCIter = flags.LArShapeDump.doOFCIter
    dumperAlg.DumpChannelInfos = flags.LArShapeDump.dumpChannelInfos
    dumperAlg.DumpDisconnected = False
    dumperAlg.DigitsKey = flags.LArShapeDump.digitsKey
    dumperAlg.ProblemsToMask=['deadReadout', 'deadPhys','almostDead', 'short',
                              'highNoiseHG','highNoiseMG','highNoiseLG']
    dumperAlg.LArShapeDumperTool=CompFactory.LArShapeDumperTool(DoShape=True)
    dumperAlg.LArShapeDumperTool=CompFactory.LArShapeDumperTool("LArShapeDumperToolSC",DoShape=False,IsSC=True)
    dumperAlg.FileName=flags.LArShapeDump.outputNtup
    dumperAlg.TriggerNames = flags.LArShapeDump.triggerNames
    dumperAlg.TrigDecisionTool = result.getPublicTool('TrigDecisionTool')
    from LArConfiguration.LArConfigFlags import RawChannelSource 
    if flags.LAr.RawChannelSource == RawChannelSource.Calculated:
       dumperAlg.ChannelsKey = "LArRawChannels_FromDigits"

    dumperAlg.EnergyCutSC = flags.LArShapeDump.energySCCut   
    dumperAlg.MinADCMaxSC = flags.LArShapeDump.adcSCCut

    result.addEventAlgo(dumperAlg)

    if (flags.LArShapeDump.HECNoiseNtup!=""):
        hns=CompFactory.LArHECNoise()
        hns.TrigDecisionTool = result.getPublicTool('TrigDecisionTool')
        result.addEventAlgo(hns)
        result.addService(CompFactory.THistSvc(Output=["HEC DATAFILE='"+flags.LArShapeDump.HECNoiseNtup+"' OPT='RECREATE'",]))

    return result


if __name__=="__main__":
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags=initConfigFlags()

    from LArShapeDumperFlags import addShapeDumpFlags
    addShapeDumpFlags(flags)

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files=defaultTestFiles.RAW_RUN2
    flags.LAr.ROD.forceIter=True

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    
    cfg=MainServicesCfg(flags)
    cfg.merge(LArShapeDumperCfg(flags))

    cfg.run(10)
