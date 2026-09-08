#!/usr/bin/env athena.py
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator 
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

def LArR4ElecCalibCalcCfg(flags):
    #Get basic services and cond-algos
    result=ComponentAccumulator()

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg 
    result.merge(LArOnOffIdMappingCfg(flags))


    from LArRecUtils.LArRecUtilsConfig import LArMCSymCondAlgCfg
    result.merge (LArMCSymCondAlgCfg (flags))

    from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
    result.merge(LArElecCalibDBCfg(flags,["DAC2uA","uA2MeV"]))

                           
    theLArElecCalibCalculator = CompFactory.LArR4ElecCalibCalculator(OutputLevel=1,
                                                                    CRESTDB="crest_fs:./larCalibTest")

    result.addEventAlgo(theLArElecCalibCalculator)


    #Output (POOL + sqlite) file writing:
    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    result.merge(OutputConditionsAlgCfg(flags,
                                        outputFile=flags.LArCalib.Output.POOLFile,
                                        ObjectList=["LArRampMC#LArRamp#/LAR/ElecCalibMC/Ramp",
                                                    "LArPedestalMC#LArPedestal#/LAR/ElecCalibMC/Pedestal",
                                                    "LArNoiseMC#LArNoise#/LAR/ElecCalibMC/Noise"],
                                        IOVTagList=["LARElecCalibMCRamp-R4-00",
                                                    "LARElecCalibMCPedestal-R4-00",
                                                    "LARElecCalibMCNoise-R4-00"],
                                        Run1=flags.LArCalib.IOVStart,
                                        Run2=flags.LArCalib.IOVEnd
                                    ))

    #RegistrationSvc    
    result.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = True))


    #ROOT ntuple writing:
    rootfile=flags.LArCalib.Output.ROOTFile
    
    if rootfile != "":
        result.addEventAlgo(CompFactory.LArNoise2Ntuple(ContainerKey="LArNoiseSym",
                                                        nGains=2,
                                                        AddFEBTempInfo = False,
                                                        RealGeometry = False,
                                                        OffId = True,
                                                        BadChanKey = "",
                                                        ))

        
        result.addEventAlgo(CompFactory.LArRamps2Ntuple( RampKey="LArRampSym",
                                                         nGains=2,
                                                         ContainerKey = [], #for RawRamp
                                                         AddFEBTempInfo = False,
                                                         RealGeometry = False,
                                                         OffId = True,
                                                         AddCalib = False,
                                                         RawRamp = False,
                                                         SaveAllSamples =  False,
                                                         BadChanKey = "",
                                                         ApplyCorr=False,
                                                     ))
        import os
        if os.path.exists(rootfile):
            os.remove(rootfile)
        result.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+rootfile+"' OPT='NEW'" ]))
        result.setAppProperty("HistogramPersistency","ROOT")
        pass # end if ROOT ntuple writing


    from LArRecUtils.LArADC2MeVCondAlgConfig import LArADC2MeVCondAlgCfg
    result.merge(LArADC2MeVCondAlgCfg(flags))
    result.getCondAlgo("LArADC2MeVCondAlg").OutputLevel=2
    result.addEventAlgo(CompFactory.LArADC2MeV2Ntuple( OffId=True,
                                                       AddBadChannelInfo=False,
                                                        nGains=2,
                                                      ))

    
    result.addEventAlgo(CompFactory.LAruA2MeV2Ntuple(uA2MeVKey="LAruA2MeVSym",
                                                     DAC2uAKey="LArDAC2uASym",
                                                     isSC = False,
                                                     OffId=True,
                                                     nGains=2,
                                                     AddBadChannelInfo=False
                                                ))


    #MC Event selector since we have no input data file
    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    result.merge(McEventSelectorCfg(flags,
                                    RunNumber         = 500000,
                                    EventsPerRun      = 1,
                                    FirstEvent        = 1,
                                    InitialTimeStamp  = 0,
                                    TimeStampInterval = 1))

    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    result.merge(PerfMonMTSvcCfg(flags))

    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    import sys
    ConfigFlags=initConfigFlags()
    addLArCalibFlags(ConfigFlags)

    ConfigFlags.LArCalib.Input.Database="LArR4ElecCalib.sqlite"
    ConfigFlags.LArCalib.Output.POOLFile="LArR4ElecCalib.pool.root"
    ConfigFlags.LArCalib.Output.ROOTFile="LArR4ElecCalib.root"

    ConfigFlags.IOVDb.DBConnection="sqlite://;schema=output.sqlite;dbname=OFLP200"
    ConfigFlags.IOVDb.GlobalTag="OFLCOND-MC23-SDR-RUN3-11"
    #ConfigFlags.IOVDb.GlobalTag="LARCALIB-RUN2-00"
    ConfigFlags.GeoModel.AtlasVersion=defaultGeometryTags.RUN4
    ConfigFlags.Input.isMC=True
    ConfigFlags.Common.MsgSuppression = False
    ConfigFlags.fillFromArgs()

    print ("Input files to be processed:")
    for f in ConfigFlags.Input.Files:
        print (f)

    ConfigFlags.lock()
    cfg=MainServicesCfg(ConfigFlags)
    cfg.merge(LArR4ElecCalibCalcCfg (ConfigFlags))
    cfg.getService("IOVDbSvc").DBInstance=""
    print("Start running...")
    sys.exit(cfg.run(1).isFailure())
    
