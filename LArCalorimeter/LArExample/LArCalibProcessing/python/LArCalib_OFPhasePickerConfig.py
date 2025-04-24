# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory 
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from LArCalibProcessing.utils import FolderTagResolver
from IOVDbSvc.IOVDbSvcConfig import addFolders

def _OFPhasePickerCfg(flags, inputSuffix="4samples3bins17phases",outputSuffix="4samples1phase",keySuffix="",nColl=0,loadInputs=True, storeShape=True, InputSCOFCPhaseDb = "", SCOFCPhaseTag = "", InputSCOFCWeightDb="", SCOFCWeightTag=""):

    result=ComponentAccumulator()
    FolderTagResolver._globalTag=flags.IOVDb.GlobalTag
    if flags.LArCalib.isSC:
       FolderTagResolver._defaultSuffix="-UPD3-00"
    rs=FolderTagResolver(dbname="sqlite://;schema=%s;dbname=CONDBR2"%flags.LArCalib.Input.Database)
    if nColl > 0:
       tagstr=rs.getFolderTag(flags.LArCalib.OFCPhys.Folder+inputSuffix)
       tagpref=tagstr[0:tagstr.find(inputSuffix)+len(inputSuffix)]
       tagpost=tagstr[tagstr.find(inputSuffix)+len(inputSuffix):]
       nc=int(nColl)
       if 'mu' not in tagstr:
          inputOFCTag=f'{tagpref}-mu-{nc}{tagpost}'
       else:   
          inputOFCTag=tagstr
       tagstr=rs.getFolderTag(flags.LArCalib.OFCPhys.Folder+outputSuffix)
       tagpref=tagstr[0:tagstr.find(outputSuffix)+len(outputSuffix)]
       tagpost=tagstr[tagstr.find(outputSuffix)+len(outputSuffix):]
       outputOFCTag=f'{tagpref}-mu-{nc}{tagpost}'   
    else:
       inputOFCTag=rs.getFolderTag(flags.LArCalib.OFCPhys.Folder+inputSuffix)
       outputOFCTag=rs.getFolderTag(flags.LArCalib.OFCPhys.Folder+outputSuffix)

    inputShapeTag=rs.getFolderTag(flags.LArCalib.Shape.Folder+inputSuffix)
    if flags.LArCalib.OFC.ShapeCorrection:
       tagstr=rs.getFolderTag(flags.LArCalib.Shape.Folder+outputSuffix)
       tagpref=tagstr[0:tagstr.find(outputSuffix)+len(outputSuffix)]
       tagpost=tagstr[tagstr.find(outputSuffix)+len(outputSuffix):]
       outputShapeTag=f'{tagpref}-corr{tagpost}'
    else:
       outputShapeTag=rs.getFolderTag(flags.LArCalib.Shape.Folder+outputSuffix)


    del rs #Close database

    from LArCalibProcessing.LArCalibBaseConfig import chanSelStr
    if loadInputs:
        result.merge(addFolders(flags,flags.LArCalib.OFCPhys.Folder+inputSuffix,detDb=flags.LArCalib.Input.Database, 
                                tag=inputOFCTag, modifiers=chanSelStr(flags)+"<key>LArOFC"+keySuffix+"</key>"))
        result.merge(addFolders(flags,flags.LArCalib.Shape.Folder+inputSuffix,detDb=flags.LArCalib.Input.Database, 
                                tag=inputShapeTag, modifiers=chanSelStr(flags)+"<key>LArShape"+keySuffix+"</key>"))

    LArOFPhasePick = CompFactory.LArOFPhasePicker("LArOFPhasePicker"+keySuffix)
    if flags.LArCalib.isSC:
        LArOFPhasePick.KeyPhase = "LArSCOFCPhase"

        if InputSCOFCPhaseDb != "" and SCOFCPhaseTag != "": #set up reading of phases from sqlite
           result.merge(addFolders(flags,"/LAR/ElecCalibOflSC/OFCBin/PhysShift",detDb=InputSCOFCPhaseDb,tag=SCOFCPhaseTag,
                                 modifiers="<key>LArSCOFCPhase</key>"))
        else:
           result.merge(addFolders(flags,"/LAR/ElecCalibOflSC/OFCBin/PhysShift", detDb="LAR_OFL",modifiers="<key>LArSCOFCPhase</key>"))

        if InputSCOFCWeightDb != "" and SCOFCWeightTag != "": # set up the weights for OFCPicker
           result.merge(addFolders(flags,"/LAR/ElecCalibOflSC/OFCFactor",detDb=InputSCOFCWeightDb,tag=SCOFCWeightTag,className="CondAttrListCollection"))
           LArOFCWeightCondAlg=CompFactory.getComp("LArFlatConditionsAlg<LArOFCweightSC>")
           result.addCondAlgo(LArOFCWeightCondAlg(ReadKey="/LAR/ElecCalibOflSC/OFCFactor", WriteKey="LArOFCbWSC"))      
           LArOFPhasePick.OFCbWeightKey="LArOFCbWSC"

    else:    
        LArOFPhasePick.KeyPhase = ""

    if nColl > 0:
       muSuffix="_mu"
    else:   
           muSuffix=""

    LArOFPhasePick.KeyOFC_new = "LArOFC"+muSuffix
    LArOFPhasePick.KeyOFC = "LArOFC"+keySuffix
    if storeShape:
       LArOFPhasePick.KeyShape_new = "LArShape"+"_uncorr" if flags.LArCalib.OFC.ShapeCorrection else  "LArShape"
       LArOFPhasePick.KeyShape = "LArShape"+keySuffix
    else:   
       LArOFPhasePick.doShape = False
    LArOFPhasePick.GroupingType = flags.LArCalib.GroupingType
    LArOFPhasePick.DefaultPhase = 4
    LArOFPhasePick.TimeOffsetCorrection = 12
    LArOFPhasePick.isSC = flags.LArCalib.isSC
    from AthenaCommon.Constants import DEBUG
    LArOFPhasePick.OutputLevel=DEBUG

    result.addEventAlgo(LArOFPhasePick)

    if flags.LArCalib.OFC.ShapeCorrection and storeShape:
        result.merge(addFolders(flags,"/LAR/ElecCalibOfl/Shape/Residuals/5samples","LAR_OFL"))
        resShapeCorr=CompFactory.LArShapeCorrector("LArShapeCorr"+keySuffix)
        resShapeCorr.KeyShape= "LArShape_uncorr" 
        resShapeCorr.KeyShape_newcorr="LArShape"
        result.addEventAlgo(resShapeCorr)

    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    Obj=["LArOFCComplete#LArOFC"+muSuffix+"#"+flags.LArCalib.OFCPhys.Folder+outputSuffix,]
    Tag=[outputOFCTag,]
    if storeShape:
       Obj+=["LArShapeComplete#LArShape#"+flags.LArCalib.Shape.Folder+outputSuffix,]
       Tag+=[outputShapeTag,]
    print('Obj: ',Obj)
    result.merge(OutputConditionsAlgCfg(flags,
                                        outputFile=flags.LArCalib.Output.POOLFile,
                                        ObjectList=Obj,
                                        IOVTagList=Tag,
                                        Run1=flags.LArCalib.IOVStart,
                                        Run2=flags.LArCalib.IOVEnd
                                    ))


    rootfile=flags.LArCalib.Output.ROOTFile
    if rootfile != "":
        bcKey = "LArBadChannelSC" if flags.LArCalib.isSC else "LArBadChannel"     
        OFC2Ntup=CompFactory.LArOFC2Ntuple("LArOFC2Ntuple"+muSuffix)
        OFC2Ntup.ContainerKey = "LArOFC"+muSuffix
        OFC2Ntup.NtupleName   = "OFC"+muSuffix
        OFC2Ntup.AddFEBTempInfo   = False   
        OFC2Ntup.AddCalib   = True   
        OFC2Ntup.isSC = flags.LArCalib.isSC
        OFC2Ntup.BadChanKey = bcKey
        result.addEventAlgo(OFC2Ntup)

        Shape2Ntup=CompFactory.LArShape2Ntuple("LArShape2Ntuple")
        Shape2Ntup.ContainerKey="LArShape"
        Shape2Ntup.NtupleName="SHAPE"
        Shape2Ntup.AddFEBTempInfo   = False
        Shape2Ntup.AddCalib   = True
        Shape2Ntup.isSC = flags.LArCalib.isSC
        Shape2Ntup.BadChanKey = bcKey
        result.addEventAlgo(Shape2Ntup)

    
    return result

def LArOFPhasePickerCfg(flags,loadInputs=True,InputSCOFCPhaseDb = "",SCOFCPhaseTag="",InputSCOFCWeightDb="",SCOFCWeightTag=""):

    #Get basic services and cond-algos
    from LArCalibProcessing.LArCalibBaseConfig import LArCalibBaseCfg
    result=LArCalibBaseCfg(flags)

    if flags.LArCalib.isSC:
       result.merge(_OFPhasePickerCfg(flags, inputSuffix=str(flags.LArCalib.OFC.Nsamples)+"samples",outputSuffix=str(flags.LArCalib.OFC.Nsamples)+"samples1phase",keySuffix="_1ns", nColl=0, loadInputs=loadInputs,InputSCOFCPhaseDb=InputSCOFCPhaseDb,SCOFCPhaseTag=SCOFCPhaseTag,InputSCOFCWeightDb=InputSCOFCWeightDb,SCOFCWeightTag=SCOFCWeightTag))
       if flags.LArCalib.OFC.Ncoll > 0:
          result.merge(_OFPhasePickerCfg(flags, inputSuffix=str(flags.LArCalib.OFC.Nsamples)+"samples",outputSuffix=str(flags.LArCalib.OFC.Nsamples)+"samples1phase",keySuffix="_1ns_mu", nColl=flags.LArCalib.OFC.Ncoll, loadInputs=loadInputs, storeShape=False,InputSCOFCPhaseDb=InputSCOFCPhaseDb,SCOFCPhaseTag=SCOFCPhaseTag,InputSCOFCWeightDb=InputSCOFCWeightDb,SCOFCWeightTag=SCOFCWeightTag))
    else:
       result.merge(_OFPhasePickerCfg(flags, inputSuffix=str(flags.LArCalib.OFC.Nsamples)+"samples3bins17phases",outputSuffix=str(flags.LArCalib.OFC.Nsamples)+"samples1phase",keySuffix="_3ns", nColl=0, loadInputs=loadInputs))
       if flags.LArCalib.OFC.Ncoll > 0:
          result.merge(_OFPhasePickerCfg(flags, inputSuffix=str(flags.LArCalib.OFC.Nsamples)+"samples3bins17phases",outputSuffix=str(flags.LArCalib.OFC.Nsamples)+"samples1phase",keySuffix="_3ns_mu", nColl=flags.LArCalib.OFC.Ncoll, loadInputs=loadInputs, storeShape=False))

    #RegistrationSvc    
    result.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = False))
    result.getService("IOVDbSvc").DBInstance=""

    #Ntuple writing
    rootfile=flags.LArCalib.Output.ROOTFile
    if rootfile != "":
        import os
        if os.path.exists(rootfile):
            os.remove(rootfile)
        result.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+rootfile+"' OPT='NEW'" ]))
        result.setAppProperty("HistogramPersistency","ROOT")
        pass # end if ROOT ntuple writing


    #MC Event selector since we have no input data file
    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    result.merge(McEventSelectorCfg(flags,
                                    RunNumber         = flags.LArCalib.Input.RunNumbers[0],
                                    EventsPerRun      = 1,
                                    FirstEvent	      = 1,
                                    InitialTimeStamp  = 0,
                                    TimeStampInterval = 1))

    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    result.merge(PerfMonMTSvcCfg(flags))
    

    
    return result

def LArCaliOFPhasePickerCfg(flags,loadInputs=True,DefaultPhase=23):

    #Get basic services and cond-algos
    from LArCalibProcessing.LArCalibBaseConfig import LArCalibBaseCfg
    result=LArCalibBaseCfg(flags)

    FolderTagResolver._globalTag=flags.IOVDb.GlobalTag
    if flags.LArCalib.isSC:
       FolderTagResolver._defaultSuffix="-UPD3-00"
    rs=FolderTagResolver(dbname="sqlite://;schema=%s;dbname=CONDBR2"%flags.LArCalib.Input.Database)
    inputOFCTag=rs.getFolderTag(flags.LArCalib.OFCCali.Folder)
    outputOFCTag=rs.getFolderTag(flags.LArCalib.OFCCali.Folder+"1phase")
    del rs #Close database

    from LArCalibProcessing.LArCalibBaseConfig import chanSelStr
    if loadInputs:
        result.merge(addFolders(flags,flags.LArCalib.OFCCali.Folder,detDb=flags.LArCalib.Input.Database, 
                                tag=inputOFCTag, modifiers=chanSelStr(flags)))

    LArOFPhasePick = CompFactory.LArOFPhasePicker("LArCaliOFPhasePicker")
    LArOFPhasePick.KeyPhase = ""
    LArOFPhasePick.KeyOFC_new = "LArOFC1phase"
    LArOFPhasePick.KeyOFC = "LArOFCCali"
    LArOFPhasePick.doShape = False
    LArOFPhasePick.GroupingType = flags.LArCalib.GroupingType
    LArOFPhasePick.DefaultPhase = DefaultPhase
    LArOFPhasePick.isSC = flags.LArCalib.isSC
    from AthenaCommon.Constants import DEBUG
    LArOFPhasePick.OutputLevel=DEBUG

    result.addEventAlgo(LArOFPhasePick)

    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    Obj=["LArOFCComplete#LArOFC1phase#"+flags.LArCalib.OFCCali.Folder+"1phase",]
    Tag=[outputOFCTag,]
    print('Obj: ',Obj)
    result.merge(OutputConditionsAlgCfg(flags,
                                        outputFile=flags.LArCalib.Output.POOLFile,
                                        ObjectList=Obj,
                                        IOVTagList=Tag,
                                        Run1=flags.LArCalib.IOVStart,
                                        Run2=flags.LArCalib.IOVEnd
                                    ))

    rootfile=flags.LArCalib.Output.ROOTFile
    if rootfile != "":
        bcKey = "LArBadChannelSC" if flags.LArCalib.isSC else "LArBadChannel"     
        OFC2Ntup=CompFactory.LArOFC2Ntuple("LArCaliOFC2Ntuple")
        OFC2Ntup.ContainerKey = "LArOFC1phase"
        OFC2Ntup.NtupleName   = "OFC1phase"
        OFC2Ntup.AddFEBTempInfo   = False   
        OFC2Ntup.AddCalib   = True   
        OFC2Ntup.isSC = flags.LArCalib.isSC
        OFC2Ntup.BadChanKey = bcKey
        result.addEventAlgo(OFC2Ntup)
        import os
        if os.path.exists(rootfile):
            print("Warning, removing existing file: ",rootfile)
            os.remove(rootfile)
        result.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+rootfile+"' OPT='NEW'" ]))
        result.setAppProperty("HistogramPersistency","ROOT")
        pass # end if ROOT ntuple writing


    #RegistrationSvc    
    result.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = False))
    result.getService("IOVDbSvc").DBInstance=""

    #MC Event selector since we have no input data file
    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    result.merge(McEventSelectorCfg(flags,
                                    RunNumber         = flags.LArCalib.Input.RunNumbers[0],
                                    EventsPerRun      = 1,
                                    FirstEvent	      = 1,
                                    InitialTimeStamp  = 0,
                                    TimeStampInterval = 1))

    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    result.merge(PerfMonMTSvcCfg(flags))
    

    
    return result
