# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory 

def LArPedestalAutoCorrCfg(flags):

    #Get basic services and cond-algos
    from LArCalibProcessing.LArCalibBaseConfig import LArCalibBaseCfg
    result=LArCalibBaseCfg(flags)

    #Add ByteStream reading
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    result.merge(ByteStreamReadCfg(flags))


    #Calibration runs are taken in fixed gain. 
    #The SG key of the digit-container is name of the gain
    gainStrMap={0:"HIGH",1:"MEDIUM",2:"LOW"}

    if not flags.LArCalib.isSC:
       digKey=gainStrMap[flags.LArCalib.Gain]
       result.addEventAlgo(CompFactory.LArRawCalibDataReadingAlg(LArAccDigitKey=digKey,
                                                              LArFebHeaderKey="LArFebHeader",
                                                              FailOnCorruption=False,
                                                              SubCaloPreselection=flags.LArCalib.Input.SubDet,
                                                              PosNegPreselection=flags.LArCalib.Preselection.Side,
                                                              BEPreselection=flags.LArCalib.Preselection.BEC,
                                                              FTNumPreselection=flags.LArCalib.Preselection.FT))

       from LArROD.LArFebErrorSummaryMakerConfig import LArFebErrorSummaryMakerCfg
       result.merge(LArFebErrorSummaryMakerCfg(flags))
       result.getEventAlgo("LArFebErrorSummaryMaker").CheckAllFEB=False
       result.addEventAlgo(CompFactory.LArBadEventCatcher(CheckAccCalibDigitCont=True,
                                                       CheckBSErrors=True,
                                                       KeyList=[digKey,],
                                                       StopOnError=False))

    else:   
       digKey="SC"
       theLArLATOMEDecoder = CompFactory.LArLATOMEDecoder("LArLATOMEDecoder",
                                                           IgnoreBarrelChannels = flags.LArCalib.SCIgnoreBarrelChannels,
                                                           IgnoreEndcapChannels = flags.LArCalib.SCIgnoreEndcapChannels)

       if flags.LArCalib.Input.isRawData:
          result.addEventAlgo(CompFactory.LArRawSCDataReadingAlg(adcCollKey = digKey, adcBasCollKey = "", etCollKey = "",
                                                               etIdCollKey = "", LATOMEDecoder = theLArLATOMEDecoder))
          result.addEventAlgo(CompFactory.LArDigitsAccumulator("LArDigitsAccumulator", KeyList = [digKey], 
                                                               LArAccuDigitContainerName = "", NTriggersPerStep = 100,
                                                               isSC = flags.LArCalib.isSC, DropPercentTrig = 0))
       else:   
          # this needs also legacy  maps
          from LArCabling.LArCablingConfig import LArCalibIdMappingCfg,LArOnOffIdMappingCfg,LArCalibIdMappingSCCfg,LArOnOffIdMappingSCCfg
          result.merge(LArOnOffIdMappingCfg(flags))
          result.merge(LArCalibIdMappingSCCfg(flags))
          result.merge(LArOnOffIdMappingSCCfg(flags))
          result.merge(LArCalibIdMappingCfg(flags))

          result.addEventAlgo(CompFactory.LArRawSCCalibDataReadingAlg(LArSCAccDigitKey = digKey, LATOMEDecoder = theLArLATOMEDecoder))

    LArPedACBuilder=CompFactory.LArPedestalAutoCorrBuilder()
    LArPedACBuilder.KeyList         = [digKey,]
    LArPedACBuilder.PedestalKey     = "Pedestal" 
    LArPedACBuilder.AutoCorrKey     = "LArAutoCorr"
    LArPedACBuilder.GroupingType    = flags.LArCalib.GroupingType

    result.addEventAlgo(LArPedACBuilder)

    #ROOT ntuple writing:
    rootfile=flags.LArCalib.Output.ROOTFile
    if flags.LArCalib.isSC: 
       bcKey = "LArBadChannelSC" 
    else: 
       bcKey = "LArBadChannel"

    if rootfile != "":
        result.addEventAlgo(CompFactory.LArPedestals2Ntuple(ContainerKey = "Pedestal",
                                                            AddFEBTempInfo = False, 
                                                            RealGeometry = True,
                                                            OffId = True,
                                                            isSC = flags.LArCalib.isSC,
                                                            BadChanKey = bcKey
                                                        )
                        )

        result.addEventAlgo(CompFactory.LArAutoCorr2Ntuple(ContainerKey = "LArAutoCorr",
                                                           AddFEBTempInfo  = False, isSC = flags.LArCalib.isSC,
                                                           BadChanKey = bcKey,
                                                           OffId=True
                                                       )
                        )

        import os
        if os.path.exists(rootfile):
            os.remove(rootfile)
        result.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+rootfile+"' OPT='NEW'" ]))
        result.setAppProperty("HistogramPersistency","ROOT")
        pass # end if ROOT ntuple writing



    #Get the current folder tag by interrogating the database:
    from LArCalibProcessing.utils import FolderTagResolver
    FolderTagResolver._globalTag=flags.IOVDb.GlobalTag
    tagResolver=FolderTagResolver()
    pedestalTag=tagResolver.getFolderTag(flags.LArCalib.Pedestal.Folder)
    autocorrTag=tagResolver.getFolderTag(flags.LArCalib.AutoCorr.Folder)
    del tagResolver


    #Output (POOL + sqlite) file writing:
    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    result.merge(OutputConditionsAlgCfg(flags,
                                        outputFile=flags.LArCalib.Output.POOLFile,
                                        ObjectList=["LArPedestalComplete#Pedestal#"+flags.LArCalib.Pedestal.Folder,
                                                    "LArAutoCorrComplete#LArAutoCorr#"+flags.LArCalib.AutoCorr.Folder],
                                        IOVTagList=[pedestalTag,autocorrTag],
                                        Run1=flags.LArCalib.IOVStart,
                                        Run2=flags.LArCalib.IOVEnd
                                    ))

    #RegistrationSvc    
    result.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = False))


    #Validation (comparision with reference):
    if flags.LArCalib.doValidation:
        from IOVDbSvc.IOVDbSvcConfig import addFolders 

        #result.merge(addFolders(flags,"/LAR/ElecCalibOnl/Pedestal<key>PedestalRef</key>","LAR_ONL"))
        result.merge(addFolders(flags,"/LAR/ElecCalibFlat/Pedestal<key>PedestalRefIn</key>","LAR_ONL","CondAttrListCollection"))
        LArPedestalCondAlg = CompFactory.getComp("LArFlatConditionsAlg<LArPedestalFlat>")
        result.addCondAlgo(LArPedestalCondAlg("LArFlatPedCondAlg",ReadKey="PedestalRefIn", WriteKey="PedestalRef"))
        from LArCalibDataQuality.Thresholds import pedThr,rmsThr, pedThrFEB,rmsThrFEB        
        from AthenaCommon.Constants import ERROR,WARNING

        thePedestalValidationAlg=CompFactory.LArPedestalValidationAlg("PedestalVal")
        #thePedestalValidationAlg.BadChannelMaskingTool=theLArPedValBCMask
        thePedestalValidationAlg.UseBadChannelInfo=False
        thePedestalValidationAlg.ValidationKey="Pedestal"
        thePedestalValidationAlg.ReferenceKey="PedestalRef"
        thePedestalValidationAlg.PedestalTolerance=pedThr
        thePedestalValidationAlg.PedestalRMSTolerance=rmsThr
        thePedestalValidationAlg.PedestalToleranceFEB=pedThrFEB
        thePedestalValidationAlg.PedestalRMSToleranceFEB=rmsThrFEB
        thePedestalValidationAlg.MsgLevelForDeviations=WARNING
        thePedestalValidationAlg.ListOfDevFEBs="pedFebs.txt"
        thePedestalValidationAlg.CheckCompletness=True
        thePedestalValidationAlg.PatchMissingFEBs=True
        thePedestalValidationAlg.UseCorrChannels=False #Corrections go into the regular data channels
        if flags.LArCalib.isSC:
           thePedestalValidationAlg.CablingKey = "LArOnOffIdMapSC"
           thePedestalValidationAlg.CalibLineKey = "LArCalibIdMapSC" 
           thePedestalValidationAlg.BadChanKey =  bcKey
        result.addEventAlgo(thePedestalValidationAlg)

        ## second instance of the validation tool to detect "bad" channel
        theBadPedestal=CompFactory.LArPedestalValidationAlg("PedestalFail")
        #theBadPedestal.BadChannelMaskingTool=theLArPedValBCMask
        theBadPedestal.UseBadChannelInfo=False
        theBadPedestal.ValidationKey="Pedestal"
        theBadPedestal.ReferenceKey="PedestalRef"
        theBadPedestal.PedestalTolerance       = ["10,10,10"]
        theBadPedestal.PedestalRMSTolerance    = ["0.5, 0.5, 0.5"]
        theBadPedestal.PedestalToleranceFEB    = ["8, 8, 8"]
        theBadPedestal.PedestalRMSToleranceFEB = ["1.2, 1.2, 1.2"]

        theBadPedestal.MsgLevelForDeviations=ERROR      
        theBadPedestal.CheckCompletness=False
        theBadPedestal.ListOfDevFEBs="Bad_pedFebs.txt"
        if flags.LArCalib.isSC:
           theBadPedestal.CablingKey = "LArOnOffIdMapSC"
           theBadPedestal.CalibLineKey = "LArCalibIdMapSC" 
           theBadPedestal.BadChanKey = bcKey 
        result.addEventAlgo(theBadPedestal)


        result.merge(addFolders(flags,"/LAR/ElecCalibOfl/AutoCorrs/AutoCorr<key>LArAutoCorrRef</key>","LAR_OFL"))
        from LArCalibDataQuality.Thresholds import acThr, acThrFEB
        theAutoCorrValidationAlg=CompFactory.LArAutoCorrValidationAlg("AutoCorrVal")
        theAutoCorrValidationAlg.UseBadChannelInfo=False
        theAutoCorrValidationAlg.ValidationKey="LArAutoCorr"
        theAutoCorrValidationAlg.ReferenceKey="LArAutoCorrRef"
        theAutoCorrValidationAlg.AutoCorrTolerance=acThr
        theAutoCorrValidationAlg.AutoCorrToleranceFEB=acThrFEB
        theAutoCorrValidationAlg.MsgLevelForDeviations=WARNING
        theAutoCorrValidationAlg.ListOfDevFEBs="ACFebs.txt"
        theAutoCorrValidationAlg.CheckCompletness=True
        theAutoCorrValidationAlg.PatchMissingFEBs=True
        theAutoCorrValidationAlg.UseCorrChannels=False #Corrections go into the regular data channels
        if flags.LArCalib.isSC:
           theAutoCorrValidationAlg.CablingKey = "LArOnOffIdMapSC"
           theAutoCorrValidationAlg.CalibLineKey = "LArCalibIdMapSC" 
           theAutoCorrValidationAlg.BadChanKey = bcKey 
        result.addEventAlgo(theAutoCorrValidationAlg)
      
        ## second instance of the validation tool to detect "bad" channel     
        theBadAutoCorr=CompFactory.LArAutoCorrValidationAlg("AutoCorrFail")
        theBadAutoCorr.UseBadChannelInfo=False
        theBadAutoCorr.ValidationKey="LArAutoCorr"
        theBadAutoCorr.ReferenceKey="LArAutoCorrRef"
        theBadAutoCorr.AutoCorrTolerance    = ["0.15, 0.15, 0.15"]
        theBadAutoCorr.AutoCorrToleranceFEB = ["0.15, 0.15, 0.15"]
        theBadAutoCorr.MsgLevelForDeviations=ERROR
        theBadAutoCorr.CheckFifthSample=True
        theBadAutoCorr.ListOfDevFEBs="Bad_ACFebs.txt"
        theBadAutoCorr.CheckCompletness=False
        if flags.LArCalib.isSC:
           theBadAutoCorr.CablingKey = "LArOnOffIdMapSC"
           theBadAutoCorr.CalibLineKey = "LArCalibIdMapSC" 
           theBadAutoCorr.BadChanKey = bcKey 
        result.addEventAlgo(theBadAutoCorr)

        result.getService("IOVDbSvc").DBInstance=""

    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    result.merge(PerfMonMTSvcCfg(flags))


    return result


if __name__ == "__main__":

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    ConfigFlags=initConfigFlags()
    addLArCalibFlags(ConfigFlags)

    ConfigFlags.LArCalib.Input.Dir = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/LArCalibProcessing"
    
    ConfigFlags.LArCalib.Input.Type="calibration_LArElec-Pedestal"
    ConfigFlags.LArCalib.Input.RunNumbers=[441236,]
    
    ConfigFlags.Input.Files=ConfigFlags.LArCalib.Input.Files
    
    ConfigFlags.LArCalib.Output.ROOTFile="larpededest.root"
    ConfigFlags.LArCalib.Output.POOLFile="larpedestal.pool.root"
    ConfigFlags.IOVDb.DBConnection="sqlite://;schema=output.sqlite;dbname=CONDBR2"
    ConfigFlags.IOVDb.GlobalTag="LARCALIB-RUN2-00"
    ConfigFlags.IOVDb.DatabaseInstance="CONDBR2"
    ConfigFlags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3

    ConfigFlags.fillFromArgs()
    ConfigFlags.lock()

    from AthenaCommon import Logging
    log = Logging.logging.getLogger( 'Athena' )
    log.info("Input files to be processed:")
    for f in ConfigFlags.Input.Files:
        log.info(f)

    cfg=MainServicesCfg(ConfigFlags)
    cfg.merge(LArPedestalAutoCorrCfg(ConfigFlags))

    log.info("Start running...")

    cfg.run()
