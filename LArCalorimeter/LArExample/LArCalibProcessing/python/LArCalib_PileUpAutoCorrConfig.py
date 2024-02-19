
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory 
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

def LArPileUpAutoCorrCfg(flags):

    #Get basic services and cond-algos
    from LArCalibProcessing.LArCalibBaseConfig import LArCalibBaseCfg,chanSelStr
    result=LArCalibBaseCfg(flags)

    from IOVDbSvc.IOVDbSvcConfig import addFolders

    #load fsampl, MinBias Average and PulseShape 32 samples from OFLP200
    if flags.LArCalib.isSC:
       from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBMCSCCfg
       # for the SC we need uA2MeV also from MC, because the one used for data are weighted with E->ET conversion
       result.merge(LArElecCalibDBMCSCCfg(flags,["fSamplSC","ShapeSC","MinBiasSC","uA2MeVSC"]))
    else:
       result.merge(addFolders(flags,"/LAR/ElecCalibMC/Shape",detDb="LAR_OFL", db="OFLP200",tag="LARElecCalibMCShapeLArPileupShape-RUN2-2018",className="LArShape32MC"))
       result.merge(addFolders(flags,"/LAR/ElecCalibMC/fSampl",detDb="LAR_OFL", db="OFLP200",tag="LARElecCalibMCfSampl-G4101-20371-FTFP_BERT_BIRK_v2",className="LArfSamplMC"))
       result.merge(addFolders(flags,"/LAR/ElecCalibMC/MinBias",detDb="LAR_OFL", db="OFLP200",tag="LARElecCalibMCMinBias-mc16-Epos-A3-s3687",className="LArMinBiasMC"))

       result.addCondAlgo(CompFactory.LArMCSymCondAlg())

       LArShapeSymAlg =  CompFactory.getComp("LArSymConditionsAlg<LArShape32MC, LArShape32Sym>")
       result.addCondAlgo(LArShapeSymAlg(ReadKey="LArShape",WriteKey="LArShapeSym"))

       LArfSamplSymAlg =  CompFactory.getComp("LArSymConditionsAlg<LArfSamplMC, LArfSamplSym>")
       result.addCondAlgo(LArfSamplSymAlg(ReadKey="LArfSampl", WriteKey="LArfSamplSym"))

       LArMinBiasSymAlg =  CompFactory.getComp("LArSymConditionsAlg<LArMinBiasMC, LArMinBiasSym>")    
       result.addCondAlgo(LArMinBiasSymAlg(ReadKey="LArMinBias",WriteKey="LArMinBiasSym"))

    from LArCalibProcessing.utils import FolderTagResolver
    FolderTagResolver._globalTag=flags.IOVDb.GlobalTag
    rs=FolderTagResolver(dbname="sqlite://;schema=%s;dbname=CONDBR2"%flags.LArCalib.Input.Database)
    AutoCorrTag=rs.getFolderTag(flags.LArCalib.AutoCorr.Folder)
    # FIXME these tags has to be re-enabled in 2024 also for main readout:
    if flags.LArCalib.isSC:
       PedestalTag=rs.getFolderTag(flags.LArCalib.Pedestal.Folder)
       RampTag=rs.getFolderTag(flags.LArCalib.Ramp.Folder)
       MpMcTag=rs.getFolderTag(flags.LArCalib.MPhysOverMCal.Folder)
    rsac=FolderTagResolver(dbname="COOLOFL_LAR/CONDBR2")
    PhysAutoCorrTag= rsac.getFolderTag(flags.LArCalib.PhysAutoCorr.Folder)
    nColl=flags.LArCalib.OFC.Ncoll
    if (nColl>0 and "mu" not in PhysAutoCorrTag):
        #Insert mu in tag-name:
        elems=PhysAutoCorrTag.split("-")
        PhysAutoCorrTag="-".join([elems[0]+"_mu_%i"%nColl,]+elems[1:])
        del elems

    del rs

    result.merge(addFolders(flags,flags.LArCalib.AutoCorr.Folder,detDb=flags.LArCalib.Input.Database, tag=AutoCorrTag, modifiers=chanSelStr(flags), 
                            className="LArAutoCorrComplete"))
    # FIXME these folders has to be re-enabled in 2024 also for Main readout:
    if flags.LArCalib.isSC:
        result.merge(addFolders(flags,flags.LArCalib.Pedestal.Folder,detDb=flags.LArCalib.Input.Database, tag=PedestalTag, modifiers=chanSelStr(flags), 
                            className="LArPedestalComplete"))
        result.merge(addFolders(flags,flags.LArCalib.Ramp.Folder,detDb=flags.LArCalib.Input.Database, tag=RampTag, modifiers=chanSelStr(flags), 
                            className="LArRampComplete"))
        result.merge(addFolders(flags,flags.LArCalib.MPhysOverMCal.Folder,detDb=flags.LArCalib.Input.Database, tag=MpMcTag, modifiers=chanSelStr(flags), 
                            className="LArMphysOverMcalComplete"))

    #Need ADC2MeV values for AutoCorrCondAlg ... 
    #use current production values as input conditions
    if flags.LArCalib.isSC:
       requiredConditions=["DAC2uA","HVScaleCorr"]
    else:   
       requiredConditions=["DAC2uA","uA2MeV","HVScaleCorr","Ramp","Pedestal","MphysOverMcal"]
       # in 2024 put:
       #requiredConditions=["DAC2uA","uA2MeV","HVScaleCorr"]
    if flags.LArCalib.isSC:
       from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBSCCfg
       result.merge(LArElecCalibDBSCCfg(flags,requiredConditions))
       mapKey="LArOnOffIdMapSC"
       bcKey = "LArBadChannelSC"
       uAKey = "LAruA2MeVSC"
       DACKey = "LArDAC2uASC"
       HVKey = "LArHVScaleCorrSC"
    else:
       from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
       result.merge(LArElecCalibDBCfg(flags,requiredConditions))
       mapKey="LArOnOffIdMap"
       bcKey = "LArBadChannel"
       uAKey = "LAruA2MeV"
       DACKey = "LArDAC2uA"
       HVKey = "LArHVScaleCorr"

    result.addCondAlgo(CompFactory.LArADC2MeVCondAlg(UseFEBGainTresholds=False,LArOnOffIdMappingKey=mapKey,CompleteDetector=False,LAruA2MeVKey=uAKey,LArDAC2uAKey=DACKey,LArHVScaleCorrKey=HVKey,isSuperCell=flags.LArCalib.isSC))

    theLArAutoCorrTotalCondAlg=CompFactory.LArAutoCorrTotalCondAlg()
    theLArAutoCorrTotalCondAlg.Nsamples=flags.LArCalib.OFC.Nsamples  
    from AthenaCommon.SystemOfUnits import ns
    theLArAutoCorrTotalCondAlg.deltaBunch=int(flags.Beam.BunchSpacing/( 25.*ns)+0.5)
    theLArAutoCorrTotalCondAlg.isSuperCell=flags.LArCalib.isSC
    theLArAutoCorrTotalCondAlg.isMC=False
    theLArAutoCorrTotalCondAlg.LArAutoCorrObjKey="LArAutoCorr"
    theLArAutoCorrTotalCondAlg.LArAutoCorrTotalObjKey="LArPhysAutoCorr"  
    theLArAutoCorrTotalCondAlg.LArOnOffIdMappingObjKey=mapKey
    if flags.LArCalib.isSC:
       theLArAutoCorrTotalCondAlg.LArPedestalObjKey="Pedestal"
       theLArAutoCorrTotalCondAlg.LArShapeObjKey = "LArShapeSC"
       theLArAutoCorrTotalCondAlg.LArfSamplObjKey = "LArfSamplSC"
       theLArAutoCorrTotalCondAlg.LArMinBiasObjKey = "LArMinBiasSC"
    else:   
       theLArAutoCorrTotalCondAlg.LArPedestalObjKey="LArPedestal"

    result.addCondAlgo(theLArAutoCorrTotalCondAlg)
    
    result.addEventAlgo(CompFactory.LArAutoCorrAlgToDB(GroupingType=flags.LArCalib.GroupingType,
                                                       NMinbias=nColl,isSC=flags.LArCalib.isSC))

    
    #Ntuple writing
    rootfile=flags.LArCalib.Output.ROOTFile
    if rootfile != "":
        result.addEventAlgo(CompFactory.LArAutoCorr2Ntuple(ContainerKey="LArPhysAutoCorr",OffId=True,isSC=flags.LArCalib.isSC,
                                                           BadChanKey=bcKey))
        import os
        if os.path.exists(rootfile):
            os.remove(rootfile)
        result.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+rootfile+"' OPT='NEW'" ]))
        result.setAppProperty("HistogramPersistency","ROOT")
    pass # end if ROOT ntuple writing
    
    

    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    result.merge(OutputConditionsAlgCfg(flags,
                                        outputFile=flags.LArCalib.Output.POOLFile,
                                        ObjectList=["LArAutoCorrComplete#LArPhysAutoCorr#"+flags.LArCalib.PhysAutoCorr.Folder,],
                                        IOVTagList=[PhysAutoCorrTag,],
                                        Run1=flags.LArCalib.IOVStart,
                                        Run2=flags.LArCalib.IOVEnd
                                    ))


    #RegistrationSvc    
    result.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = False))
    result.getService("IOVDbSvc").DBInstance=""


    #MC Event selector since we have no input data file 
    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    result.merge(McEventSelectorCfg(flags,
                                    RunNumber = flags.LArCalib.Input.RunNumbers[0],
                                    EventsPerRun      = 1,
                                    FirstEvent	      = 1,
                                    InitialTimeStamp  = 0,
                                    TimeStampInterval = 1))


    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    result.merge(PerfMonMTSvcCfg(flags))

    return result




if __name__ == "__main__":

    import sys
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    ConfigFlags=initConfigFlags()
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    addLArCalibFlags(ConfigFlags)
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    
    ConfigFlags.Input.Files=[]
    ConfigFlags.LArCalib.Input.RunNumbers=[400939,]
    ConfigFlags.LArCalib.Input.Database="/home/wlampl/calibTest/00400939_00400943_00400945_Barrel-EMB-EMEC_HIGH_40_21.0.20_1/poolFiles/myDB200_00400939_00400943_00400945_EB-EMBA_one.db_Delay"
    ConfigFlags.LArCalib.Input.SubDet="EM"
    ConfigFlags.LArCalib.BadChannelDB="/home/wlampl/calibTest/00400939_00400943_00400945_Barrel-EMB-EMEC_HIGH_40_21.0.20_1/poolFiles/SnapshotBadChannel_00400939_00400943_00400945_EB-EMBA.db"
    ConfigFlags.LArCalib.BadChannelTag="-RUN2-UPD3-00"
    ConfigFlags.LArCalib.Output.ROOTFile="physAC.root"
    ConfigFlags.IOVDb.DatabaseInstance="CONDBR2"
    ConfigFlags.IOVDb.DBConnection="sqlite://;schema=output.sqlite;dbname=CONDBR2"
    ConfigFlags.IOVDb.GlobalTag="LARCALIB-RUN2-02"
    ConfigFlags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
    #ConfigFlags.Exec.OutputLevel=1
    ConfigFlags.LArCalib.OFC.Ncoll=20
    ConfigFlags.LArCalib.OFC.Nsamples=5

    ConfigFlags.fillFromArgs()
    ConfigFlags.lock()

    cfg=MainServicesCfg(ConfigFlags)
    cfg.merge(LArPileUpAutoCorrCfg(ConfigFlags))


    print("Folders:")
    print(cfg.getService("IOVDbSvc").Folders)
    print("ADC2MeV:")
    print(cfg.getCondAlgo("LArADC2MeVCondAlg"))
    print("AutoCorrTotal:")
    print(cfg.getCondAlgo("LArAutoCorrTotalCondAlg"))


    print("Start running...")
    sys.exit(cfg.run(1).isFailure())
