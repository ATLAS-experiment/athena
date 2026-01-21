# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory 

def LArReadCellsCfg(flags):

    result=ComponentAccumulator()

    #setup Calo reco
    from CaloRec.CaloRecoConfig import CaloRecoCfg
    result.merge(CaloRecoCfg(flags))
    result.getEventAlgo("LArRawChannelBuilder").TimingContainerKey="LArOFIterResult"
    
    from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamDecoderCfg
    result.merge(L1TriggerByteStreamDecoderCfg(flags))

    from LArCafJobs.LArSCDumperSkeleton import L1CaloMenuCfg
    result.merge(L1CaloMenuCfg(flags))

    from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
    result.merge(BunchCrossingCondAlgCfg(flags))
    
    from IOVDbSvc.IOVDbSvcConfig import addFolders 
    result.merge(addFolders(flags,
                            '/LAR/ElecCalibOfl/Shape/RTM/4samples3bins17phases<tag>LARElecCalibOflShapeRTM4samples3bins17phases-RUN2-UPD3-00</tag><key>LArShape17phases</key>',
                            'LAR_OFL'))

    result.getService("PoolSvc").ReadCatalog += ["apcfile:poolcond/PoolCat_comcond_castor.xml"]

    result.merge(addFolders(flags,'/LAR/ElecCalibOfl/AutoCorrs/AutoCorr<tag>LARElecCalibOflAutoCorrsAutoCorr-RUN2-UPD3-00</tag>','LAR_OFL'))
    result.getService("IOVDbSvc").overrideTags+=['<prefix>/LAR/ElecCalibOfl/Shape/RTM/5samples1phase</prefix><tag>LARElecCalibOflShapeRTM5samples1phase-RUN2-UPD1-04</tag>']
    # for splashes: FIXME later
    result.getService("IOVDbSvc").overrideTags+=['<prefix>/LAR/ElecCalibOfl/OFC/PhysWave/RTM/4samples3bins17phases</prefix><tag>LARElecCalibOflOFCPhysWaveRTM4samples3bins17phases-RUN2-UPD3-00</tag>']
    result.getService("IOVDbSvc").overrideTags+=['<prefix>/LAR/ElecCalibOfl/Shape/RTM/4samples3bins17phases</prefix><tag>LARElecCalibOflShapeRTM4samples3bins17phases-RUN2-UPD3-00</tag>']

    print("Dumping flags: ")
    flags.dump()
    dumperAlg=CompFactory.LArReadCells("LArReadCells")
    dumperAlg.output = flags.LArShapeDump.outputNtup
    dumperAlg.etCut = -1500.
    dumperAlg.etCut2 = -1500.

    result.addEventAlgo(dumperAlg)

    return result

def LArReadSCCfg(flags):

    result=ComponentAccumulator()
    from AthenaCommon.Logging import logging
    mlog = logging.getLogger( 'LArReadSCCfg' )

    #setup SC reading
    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
    result.merge(LArOnOffIdMappingSCCfg(flags))
    from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
    result.merge(LArRawSCDataReadingCfg(flags))
    result.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg('CaloSuperCellAlignCondAlg'))
    from LArCellRec.LArRAWtoSuperCellConfig import LArRAWtoSuperCellCfg
    result.merge(LArRAWtoSuperCellCfg(flags,mask=True, SCellContainerOut="SCell") )

    from LArCafJobs.LArSCDumperSkeleton import L1CaloMenuCfg
    result.merge(L1CaloMenuCfg(flags))

    from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
    result.merge(BunchCrossingCondAlgCfg(flags))
    
    from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBSCCfg
    result.merge(LArElecCalibDBSCCfg(flags, condObjs=["Pedestal"]))

    #setup SC reco
    if flags.LArShapeDump.doSCReco:
       # and elec. calib. coeffs
       result.merge(LArElecCalibDBSCCfg(flags, condObjs=["Ramp","DAC2uA", "uA2MeV", "MphysOverMcal", "OFC", "Shape", "HVScaleCorr"]))
       larLATOMEBuilderAlg=CompFactory.LArLATOMEBuilderAlg("LArLATOMEBuilderAlg")

    dumperAlg=CompFactory.LArReadSC("LArReadSC")

    from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
    try:
        runinfo=getLArDTInfoForRun(flags.Input.RunNumbers[0], connstring="COOLONL_LAR/CONDBR2")
        streamTypes=runinfo.streamTypes()
    except Exception as e:
        mlog.warning("Could not get DT run info, using defaults !")
        mlog.warning(e)
        streamTypes=["RawADC"]
    
    for i in range(0,len(streamTypes)):
        if streamTypes[i] ==  "RawADC":
            dumperAlg.DigitsKey = "SC"
            if flags.LArShapeDump.doSCReco:
               larLATOMEBuilderAlg.LArDigitKey = "SC"
               larLATOMEBuilderAlg.isADCBas = False
        if streamTypes[i] ==  "ADC":
            if flags.LArShapeDump.doSCReco:
               larLATOMEBuilderAlg.isADCBas = True
               larLATOMEBuilderAlg.LArDigitKey = "SC_ADC_BAS"
            dumperAlg.DigitsKey = "SC_ADC_BAS"

    if flags.LArShapeDump.doSCReco:    
       result.addEventAlgo(larLATOMEBuilderAlg)
       result.merge(LArRAWtoSuperCellCfg(flags,name="LArRAWRecotoSuperCell",mask=True,doReco=True,SCIn="SC_ET_RECO",SCellContainerOut="SCell_RECO") )    

    dumperAlg.output = flags.LArShapeDump.outputNtup
    dumperAlg.SCContainerKey = "SCell"
    if flags.LArShapeDump.doSCReco:
       dumperAlg.SCRecoContainerKey = "SCell_RECO"
    dumperAlg.etCut = -1500.

    result.addEventAlgo(dumperAlg)

    return result


if __name__=="__main__":
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags=initConfigFlags()
    from LArShapeDumperFlags import addShapeDumpFlags
    addShapeDumpFlags(flags)

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files=defaultTestFiles.RAW_RUN2
    flags.LAr.ROD.forceIter=True
    flags.LArShapeDump.outputNtup="SPLASH"
    flags.lock()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    
    cfg=MainServicesCfg(flags)
    cfg.addService(CompFactory.THistSvc(Output=["SPLASH DATAFILE='ntuple.root' OPT='RECREATE'",]))
    cfg.merge(LArReadCellsCfg(flags))


    cfg.run(10)
