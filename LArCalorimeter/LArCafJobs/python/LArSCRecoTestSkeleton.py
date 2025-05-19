# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import sys
import os

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

def fromRunArgs(runArgs):

    from AthenaCommon.Logging import logging
    mlog_SCD = logging.getLogger( 'LArSCRecoTestSkeleton' )

    from AthenaConfiguration.AllConfigFlags import initConfigFlags    
    flags=initConfigFlags()

    from LArCafJobs.LArSCDumperFlags import addSCDumpFlags
    addSCDumpFlags(flags)
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    addLArCalibFlags(flags, True)

    commonRunArgsToFlags(runArgs, flags)

    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.Input.Files=runArgs.inputBSFile
    flags.LArSCDump.outputNtup=runArgs.outputNTUP_SCRecoFile

    # real geom not working yet
    flags.LArSCDump.doGeom=False

    flags.LArSCDump.doBC=True

    from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
    try:
       runinfo=getLArDTInfoForRun(flags.Input.RunNumbers[0], connstring="COOLONL_LAR/CONDBR2")
    except Exception:
       mlog_SCD.warning("Could not get DT run info, using defaults !")   
       flags.LArSCDump.doEt=True
       flags.LArSCDump.nSamples=5
       flags.LArSCDump.nEt=1
       SCKey="SC_ET"    
       flags.LArSCDump.digitsKey="SC_ADC_BAS"
       fw=5
    else:   
       fw=runinfo.FWversion()
       flags.LArSCDump.digitsKey=""
       for i in range(0,len(runinfo.streamTypes())):
          if runinfo.streamTypes()[i] ==  "SelectedEnergy":
                SCKey = "SC_ET_ID"
                flags.LArSCDump.doEt=True
                flags.LArSCDump.nEt=runinfo.streamLengths()[i]
          elif runinfo.streamTypes()[i] ==  "Energy":
                SCKey = "SC_ET"
                flags.LArSCDump.doEt=True
                flags.LArSCDump.nEt=runinfo.streamLengths()[i]
          elif runinfo.streamTypes()[i] ==  "RawADC":
                flags.LArSCDump.digitsKey="SC"
                flags.LArSCDump.nSamples=runinfo.streamLengths()[i]
          elif runinfo.streamTypes()[i] ==  "ADC":
                flags.LArSCDump.digitsKey="SC_ADC_BAS"
                flags.LArSCDump.nSamples=runinfo.streamLengths()[i]
                   
    finally:
       flags.LArSCDump.doRawChan=True

    print("Autoconfigured: ",flags.LArSCDump.nEt, flags.LArSCDump.nSamples)   
    # To respect --athenaopts 
    flags.fillFromArgs()

    flags.lock()
    
    cfg=MainServicesCfg(flags)

    #Setup cabling
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg, LArOnOffIdMappingSCCfg
    cfg.merge(LArOnOffIdMappingCfg(flags))
    cfg.merge(LArOnOffIdMappingSCCfg(flags))

    from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBSCCfg
    cfg.merge(LArElecCalibDBSCCfg(flags, condObjs=["Ramp","DAC2uA", "Pedestal", "uA2MeV", "MphysOverMcal", "OFC", "Shape", "HVScaleCorr"]))

    from TrigConfigSvc.TrigConfigSvcCfg import BunchGroupCondAlgCfg
    cfg.merge( BunchGroupCondAlgCfg( flags ) )


    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))

    if fw==6:
       from IOVDbSvc.IOVDbSvcConfig import addOverride
       cfg.merge(addOverride(flags,"/LAR/Identifier/LatomeMapping","LARIdentifierLatomeMapping-fw6"))

    if flags.LArSCDump.doBC:
       from LArBadChannelTool.LArBadChannelConfig import  LArBadChannelCfg
       cfg.merge(LArBadChannelCfg(flags, isSC=True))


    from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
    cfg.merge(LArRawDataReadingCfg(flags))

    from LArROD.LArRawChannelBuilderAlgConfig import LArRawChannelBuilderAlgCfg
    cfg.merge(LArRawChannelBuilderAlgCfg(flags))

    from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
    SCData_acc =  LArRawSCDataReadingCfg(flags)
    SCData_acc.OutputLevel=3
    cfg.merge(SCData_acc)

    from AthenaConfiguration.ComponentFactory import CompFactory
    larLATOMEBuilderAlg=CompFactory.LArLATOMEBuilderAlg("LArLATOMEBuilderAlg",LArDigitKey=flags.LArSCDump.digitsKey, isADCBas="BAS" in flags.LArSCDump.digitsKey, nEnergies = flags.LArSCDump.nSamples - 3, startEnergy=runArgs.startSampleShift)
    cfg.addEventAlgo(larLATOMEBuilderAlg)

    cfg.addEventAlgo(CompFactory.LArSC2NtupleEB("LArSC2NtupleEB", isSC=True, AddBadChannelInfo=flags.LArSCDump.doBC, BadChanKey="LArBadChannelSC",
                            OffId=flags.LArSCDump.doOfflineId, AddHash=flags.LArSCDump.doHash, AddCalib=flags.LArSCDump.doCalib, RealGeometry=flags.LArSCDump.doGeom, ExpandId=flags.LArSCDump.expandId, # from LArCond2NtupleBase 
                            FillBCID=flags.LArSCDump.doBCID, EnergyContainerKey=SCKey,
                            scNet=flags.LArSCDump.nEt, EnergyCut=runArgs.energyCut,
                            RecoContainerKey="SC_ET_RECO", RawChanContainerKey="LArRawChannels",
                            OutputLevel=3))

    if os.path.exists(flags.LArSCDump.outputNtup):
          os.remove(flags.LArSCDump.outputNtup)
    cfg.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+flags.LArSCDump.outputNtup+"' OPT='NEW'" ]))
    cfg.setAppProperty("HistogramPersistency","ROOT")


    processPostInclude(runArgs, flags, cfg)
    processPostExec(runArgs, flags, cfg)

    #example how to dump the stores
    #cfg.getService("StoreGateSvc").Dump=True
    #from AthenaCommon.Constants import DEBUG
    #cfg.getService("MessageSvc").OutputLevel=DEBUG
    # Run the final accumulator
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
