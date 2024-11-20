# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory 
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

def LArRampCfg(flags):
    #Get basic services and cond-algos
    from LArCalibProcessing.LArCalibBaseConfig import LArCalibBaseCfg,chanSelStr
    result=LArCalibBaseCfg(flags)

    #Add ByteStream reading
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    result.merge(ByteStreamReadCfg(flags))

    #Calibration runs are taken in fixed gain. 
    #The SG key of the digit-container is name of the gain
    gainStrMap={0:"HIGH",1:"MEDIUM",2:"LOW"}
    digKey=gainStrMap[flags.LArCalib.Gain]

    from LArCalibProcessing.utils import FolderTagResolver
    FolderTagResolver._globalTag=flags.IOVDb.GlobalTag
    tagResolver=FolderTagResolver()
    pedestalTag=tagResolver.getFolderTag(flags.LArCalib.Pedestal.Folder)
    caliOFCTag=tagResolver.getFolderTag(flags.LArCalib.OFCCali.Folder)

    rampTag=tagResolver.getFolderTag(flags.LArCalib.Ramp.Folder)
    del tagResolver
    
    print("pedestalTag",pedestalTag)
    print("rampTag",rampTag)


    from IOVDbSvc.IOVDbSvcConfig import addFolders
    result.merge(addFolders(flags,flags.LArCalib.Pedestal.Folder,detDb=flags.LArCalib.Input.Database, tag=pedestalTag,  modifiers=chanSelStr(flags), 
                            className="LArPedestalComplete"))
    result.merge(addFolders(flags,flags.LArCalib.OFCCali.Folder,detDb=flags.LArCalib.Input.Database2, tag=caliOFCTag, modifiers=chanSelStr(flags)))
    

    if not flags.LArCalib.isSC:
       result.addEventAlgo(CompFactory.LArRawCalibDataReadingAlg(LArAccCalibDigitKey=digKey,
                                                              LArFebHeaderKey="LArFebHeader",
                                                              SubCaloPreselection=flags.LArCalib.Input.SubDet,
                                                              PosNegPreselection=flags.LArCalib.Preselection.Side,
                                                              BEPreselection=flags.LArCalib.Preselection.BEC,
                                                              FTNumPreselection=flags.LArCalib.Preselection.FT))
    
       from LArROD.LArFebErrorSummaryMakerConfig import LArFebErrorSummaryMakerCfg
       result.merge(LArFebErrorSummaryMakerCfg(flags))
       result.getEventAlgo("LArFebErrorSummaryMaker").CheckAllFEB=False

       if flags.LArCalib.Input.SubDet == "EM":
          from LArCalibProcessing.LArStripsXtalkCorrConfig import LArStripsXtalkCorrCfg
          result.merge(LArStripsXtalkCorrCfg(flags,[digKey,]))

          theLArCalibShortCorrector = CompFactory.LArCalibShortCorrector(KeyList = [digKey,])
          result.addEventAlgo(theLArCalibShortCorrector)
    else:   
       digKey="SC"
       theLArLATOMEDecoder = CompFactory.LArLATOMEDecoder("LArLATOMEDecoder")
       if flags.LArCalib.Input.isRawData:
          result.addEventAlgo(CompFactory.LArRawSCDataReadingAlg(adcCollKey = digKey, adcBasCollKey = "", etCollKey = "",
                                                               etIdCollKey = "", LATOMEDecoder = theLArLATOMEDecoder))
          from LArCalibProcessing.LArCalib_CalibDigitsMakerConfig import LArCalibDigitsMakerCfg
          result.merge(LArCalibDigitsMakerCfg(flags,digKey))

       else:   
          # this needs also legacy  maps
          from LArCabling.LArCablingConfig import LArCalibIdMappingCfg,LArOnOffIdMappingCfg
          result.merge(LArOnOffIdMappingCfg(flags))
          result.merge(LArCalibIdMappingCfg(flags))

          result.addEventAlgo(CompFactory.LArRawSCCalibDataReadingAlg(LArSCAccCalibDigitKey = digKey, LATOMEDecoder = theLArLATOMEDecoder))

    pass


    bcKey = "LArBadChannelSC" if flags.LArCalib.isSC else "LArBadChannel"     


    theLArRampBuilder = CompFactory.LArRampBuilder()
    theLArRampBuilder.KeyList      = [digKey,]
    theLArRampBuilder.SubtractDac0 = False
    if flags.LArCalib.isSC:
       theLArRampBuilder.ProblemsToMask=["deadCalib","deadReadout","deadPhys","maskedOSUM","OffOFCs","transmissionErrorFibre",]
    else:
       theLArRampBuilder.ProblemsToMask=["deadCalib","deadReadout","deadPhys","almostDead","short"]

    theLArRampBuilder.RecoType = "OF"
    theLArRampBuilder.PeakOFTool=CompFactory.LArOFPeakRecoTool(UseShape = False,OutputLevel=2)

    theLArRampBuilder.DAC0         = 4294967294
    theLArRampBuilder.StoreRawRamp = True
    theLArRampBuilder.StoreRecRamp = True
    theLArRampBuilder.Polynom      = 1    
    theLArRampBuilder.RampRange    = 3600 # Check on the raw data ADC sample before ped subtraction and pulse reconstruction to include point in fit
    theLArRampBuilder.correctBias  = False
    #theLArRampBuilder.ConsecutiveADCs = 0
    theLArRampBuilder.minDAC = 10      # minimum DAC value to use in fit
    theLArRampBuilder.KeyOutput = "LArRamp"
    theLArRampBuilder.DeadChannelCut = -9999
    theLArRampBuilder.GroupingType = flags.LArCalib.GroupingType
    #theLArRampBuilder.LongNtuple = False

    theLArRampBuilder.isSC = flags.LArCalib.isSC
    theLArRampBuilder.BadChanKey = bcKey

    if "HEC" in flags.LArCalib.Input.SubDet:
        theLArRampBuilder.isHEC = True
        theLArRampBuilder.HECKey = "LArHEC_PAmap"
        result.merge(addFolders(flags,'/LAR/ElecCalibOfl/HecPAMap','LAR_OFL'))
    
    result.addEventAlgo(theLArRampBuilder)


    # Bad-channel patching 
    if flags.LArCalib.CorrectBadChannels:
        LArRampPatcher=CompFactory.getComp("LArCalibPatchingAlg<LArRampComplete>")
        theLArRampPatcher=LArRampPatcher("LArRampPatcher")
        theLArRampPatcher.ContainerKey="LArRamp"
        theLArRampPatcher.BadChanKey=bcKey
        theLArRampPatcher.PatchMethod="PhiAverage"
        theLArRampPatcher.SuperCells=flags.LArCalib.isSC
   
        if flags.LArCalib.isSC:
          theLArRampPatcher.ProblemsToPatch=["deadCalib","deadReadout","deadPhys","maskedOSUM","OffOFCs","transmissionErrorFibre"]
          theLArRampPatcher.OnOffMap="LArOnOffIdMapSC"
          theLArRampPatcher.CalibLineKey="LArCalibIdMapSC"
        else:  
          theLArRampPatcher.ProblemsToPatch=["deadCalib","deadReadout","deadPhys","almostDead","short"]
        theLArRampPatcher.UseCorrChannels=False
        result.addEventAlgo(theLArRampPatcher)

    # Validation + CB patching 
    if flags.LArCalib.doValidation:
       
       if flags.LArCalib.isSC:
         fldr="/LAR/ElecCalibFlatSC/Ramp"
         rmpFlt=CompFactory.getComp("LArFlatConditionsAlg<LArRampSC>")("RampFltVal")
       else:  
         fldr="/LAR/ElecCalibFlat/Ramp"
         rmpFlt=CompFactory.getComp("LArFlatConditionsAlg<LArRampFlat>")("RampFltVal")
       result.merge(addFolders(flags,fldr,"LAR_ONL"))
       condLoader=result.getCondAlgo("CondInputLoader")
       condLoader.Load.add(("CondAttrListCollection",fldr))

       rmpFlt.ReadKey=fldr
       rmpFlt.WriteKey="LArRampRef"
       result.addCondAlgo(rmpFlt)

       from LArCalibDataQuality.Thresholds import rampThr, rampThrFEB
       from AthenaCommon.Constants import WARNING
 
       theRampValidationAlg=CompFactory.LArRampValidationAlg("RampVal")
       theRampValidationAlg.RampTolerance=rampThr
       theRampValidationAlg.RampToleranceFEB=rampThrFEB
       if flags.LArCalib.isSC:
          theRampValidationAlg.ProblemsToMask=["deadCalib","deadReadout","deadPhys","maskedOSUM",
                                               "OffOFCs","transmissionErrorFibre"]
       else:   
          theRampValidationAlg.ProblemsToMask=["deadReadout","deadCalib","deadPhys","almostDead",
                                               "highNoiseHG","highNoiseMG","highNoiseLG"]
       theRampValidationAlg.KeyList=["LArRamp"+digKey,]
       if flags.LArCalib.isSC:
          theRampValidationAlg.PatchMissingFEBs = False
          theRampValidationAlg.CheckCompletness = False
       else:   
          theRampValidationAlg.PatchMissingFEBs=True
       theRampValidationAlg.UseCorrChannels=False
       theRampValidationAlg.ValidationKey="LArRamp"
       theRampValidationAlg.ReferenceKey="LArRampRef"
  
       theRampValidationAlg.MsgLevelForDeviations=WARNING
       theRampValidationAlg.ListOfDevFEBs="rampFebs.txt"

       theRampValidationAlg.BadChanKey =  bcKey
       theRampValidationAlg.SuperCells = flags.LArCalib.isSC 

       if flags.LArCalib.isSC:
          theRampValidationAlg.CablingKey = "LArOnOffIdMapSC"
          theRampValidationAlg.CalibLineKey = "LArCalibIdMapSC"

       result.addEventAlgo(theRampValidationAlg)

    #Output (POOL + sqlite) file writing:
    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    result.merge(OutputConditionsAlgCfg(flags,
                                        outputFile=flags.LArCalib.Output.POOLFile,
                                        ObjectList=["LArRampComplete#LArRamp#"+flags.LArCalib.Ramp.Folder,],
                                        IOVTagList=[rampTag,],
                                        Run1=flags.LArCalib.IOVStart,
                                        Run2=flags.LArCalib.IOVEnd
                                    ))

    #RegistrationSvc    
    result.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = False))


    #ROOT ntuple writing:
    rootfile=flags.LArCalib.Output.ROOTFile
    
    ntupKey = digKey
    if flags.LArCalib.isSC:
        ntupKey = "HIGH" # Modification to avoid problems in LArRampBuilder
    if rootfile != "":
        result.addEventAlgo(CompFactory.LArRamps2Ntuple( ContainerKey = ["LArRamp"+ntupKey], #for RawRamp
                                                         AddFEBTempInfo = False,
                                                         RealGeometry = True,
                                                         OffId = True,
                                                         AddCalib = True,
                                                         RawRamp = True,
                                                         SaveAllSamples =  True,
                                                         BadChanKey = bcKey,
                                                         ApplyCorr=True,
                                                         isSC = flags.LArCalib.isSC
                                                     ))
        if flags.LArCalib.Output.SaveAverages:
           result.addEventAlgo(CompFactory.LArAverages2Ntuple( "LArAverages2Ntuple"+digKey,
                                                               ContainerKey = "SC" if flags.LArCalib.isSC else digKey, 
                                                               BadChanKey = bcKey,
                                                               isSC = flags.LArCalib.isSC
                                                              ))

        import os
        if os.path.exists(rootfile):
            os.remove(rootfile)
        result.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+rootfile+"' OPT='NEW'" ]))
        result.setAppProperty("HistogramPersistency","ROOT")
        pass # end if ROOT ntuple writing


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

    ConfigFlags.LArCalib.Input.Dir = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/LArCalibProcessing"
    ConfigFlags.LArCalib.Input.Type="calibration_LArElec-Ramp"
    ConfigFlags.LArCalib.Input.RunNumbers=[441252,]
    ConfigFlags.LArCalib.Input.SubDet="EM"
    ConfigFlags.Input.Files=ConfigFlags.LArCalib.Input.Files
    ConfigFlags.LArCalib.Input.Database="output.sqlite"
    ConfigFlags.LArCalib.Output.POOLFile="larramp.pool.root"
    ConfigFlags.LArCalib.Output.ROOTFile="larramp.root"

    ConfigFlags.IOVDb.DBConnection="sqlite://;schema=output.sqlite;dbname=CONDBR2"
    ConfigFlags.IOVDb.GlobalTag="LARCALIB-RUN2-02"
    ConfigFlags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
    #ConfigFlags.Exec.OutputLevel=1
    ConfigFlags.fillFromArgs()

    print ("Input files to be processed:")
    for f in ConfigFlags.Input.Files:
        print (f)

    ConfigFlags.lock()
    cfg=MainServicesCfg(ConfigFlags)
    cfg.merge(LArRampCfg(ConfigFlags))

    print("Start running...")
    sys.exit(cfg.run().isFailure())
    
