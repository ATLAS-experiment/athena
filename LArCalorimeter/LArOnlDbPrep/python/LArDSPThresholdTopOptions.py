#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

tagList = [
           'LARConfigurationDSPThresholdFlatTemplates-Qt1sigma-samp1sigma',
           'LARConfigurationDSPThresholdFlatTemplates-Qt1.5sigma-samp1.5sigma',
           'LARConfigurationDSPThresholdFlatTemplates-Qt2sigma-samp2sigma',
           'LARConfigurationDSPThresholdFlatTemplates-Qt3sigma-samp3sigma',
           'LARConfigurationDSPThresholdFlatTemplates-Qt5sigma-samp5sigma',
           'LARConfigurationDSPThresholdFlatTemplates-Qt3sigmamuPileup-samp3sigmamuPileup',
           'LARConfigurationDSPThresholdFlatTemplates-Qt4sigmamuPileup-samp4sigmamuPileup',
           'LARConfigurationDSPThresholdFlatTemplates-Qt5sigmamuPileup-samp5sigmamuPileup'
           ]


def LArDSPThresholdCfg(flags,tag="",ModeType="noise",RunSince=0,fill=True,
                       Sampval=0, Qtval=0, Samppileup=False, Qtpileup=False):

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg=LArGMCfg(flags)

    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    cfg.merge(LArOnOffIdMappingCfg(flags))
    # setup bad chan and missing febs
    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg,LArBadFebCfg
    cfg.merge(LArBadChannelCfg(flags))
    cfg.merge(LArBadFebCfg(flags))

    folder="/LAR/Configuration/DSPThresholdFlat/Templates"

    fileName=ModeType+tag

    setName="-".join(tag.split("-")[1:])

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    cfg.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))
    cfg.merge(CaloNoiseCondAlgCfg(flags,"electronicNoise"))
    cfg.merge(CaloNoiseCondAlgCfg(flags,"pileupNoise"))

    from AthenaConfiguration.ComponentFactory import CompFactory
    theLArDSPThresholdFillAlg=CompFactory.LArDSPThresholdFillInline("LArDSPThresholdFillInline")
    from AthenaCommon.Constants import INFO
    theLArDSPThresholdFillAlg.OutputLevel=INFO
    theLArDSPThresholdFillAlg.Key=folder
    theLArDSPThresholdFillAlg.OutFile=fileName+".txt"
    theLArDSPThresholdFillAlg.mode=ModeType
    theLArDSPThresholdFillAlg.MaskBadChannels=True
    theLArDSPThresholdFillAlg.ProblemsToMask=[ "highNoiseHG","highNoiseMG","highNoiseLG" ]
    theLArDSPThresholdFillAlg.NameOfSet=setName

    if ModeType=="fixed":
       theLArDSPThresholdFillAlg.tQThreshold=Qtval
       theLArDSPThresholdFillAlg.samplesThreshold=Sampval

    if ModeType=="noise":
       theLArDSPThresholdFillAlg.sigmaNoiseSamples=Sampval
       theLArDSPThresholdFillAlg.sigmaNoiseQt=Qtval
       theLArDSPThresholdFillAlg.usePileupNoiseSamples=Samppileup
       theLArDSPThresholdFillAlg.usePileupNoiseQt=Qtpileup

    if fill:
       theLArDSPThresholdFillAlg.Fill=True
       theLArDSPThresholdFillAlg.Dump=True
    else:
       theLArDSPThresholdFillAlg.Fill=False
       theLArDSPThresholdFillAlg.Dump=True

    cfg.addEventAlgo(theLArDSPThresholdFillAlg)

    if fill:
       OutputList=[ "AthenaAttributeList#"+folder ]
       OutputTagList=[tag]

       from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
       cfg.merge(OutputConditionsAlgCfg(flags,outputFile="LArDSPthresholdTemplates.pool.root",
                                               ObjectList=OutputList,IOVTagList=OutputTagList,WriteIOV=True, Run1=RunSince))

       cfg.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = False,
                                    OverrideNames = ["tQThr","samplesThr","trigSumThr",], 
                                    OverrideTypes = ["Blob16M","Blob16M","Blob16M",] ))


    else:
       from IOVDbSvc.IOVDbSvcConfig import addFolders
       cfg.merge(addFolders(flags,folder+"<tag>"+tag+"</tag>"))

    cfg.getService("PoolSvc").WriteCatalog="xmlcatalog_file:PoolFileCatalog_LARConfigurationDSPThresholdTemplates.xml"

    #MC Event selector since we have no input data file
    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags,
                                     RunNumber         = flags.Input.RunNumbers[0],
                                     EventsPerRun      = 1,
                                     FirstEvent        = 1,
                                     InitialTimeStamp  = 0,
                                     TimeStampInterval = 1))

    return cfg

if __name__ == "__main__":

    import argparse
    import sys

    # now process the CL options and assign defaults
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('-r','--run', dest='run', default=500000, help='Run to read the noise')
    parser.add_argument('-s','--since', dest='since', default=480000, help='Run to start IOV')
    parser.add_argument('-t','--tagnum', dest='tagnum', default=0, help='Which tag to compute')
    parser.add_argument('-x','--tagstr', dest='tagstr', default='', help='Tag string to compute')
    parser.add_argument('-n','--noisetag', dest='noisetag', default="", help='Which noise tag to use')
    parser.add_argument('--sqlite', dest='sql', default="", help='Sqlite file with noise folder')
    parser.add_argument('-a','--localnoise', dest='localnoise', default=False, action="store_true", help='read the noise from local sqlite')
    parser.add_argument('-p','--localpileup', dest='localpileup', default=False, action="store_true", help='read the pileup from local sqlite')

    args = parser.parse_args()
    if help in args and args.help is not None and args.help:
       parser.print_help()
       sys.exit(0)

    for _, value in args._get_kwargs():
     if value is not None:
         print(value)

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    flags=initConfigFlags()
    addLArCalibFlags(flags)

    if args.tagstr != '':
       if 'LARConfigurationDSPThresholdFlatTemplates-Qt' not in args.tagstr or '-samp' not in args.tagstr:
          tagstr = tagList[0]
       else:
          tagstr = args.tagstr   
    else:
       tagstr = tagList[int(args.tagnum)] if int(args.tagnum)<len(tagList) else tagList[0]

    print(tagstr)

    Qtstr=tagstr.split("Qt")[1]
    Sampstr=tagstr.split("samp")[1]

    if "sigma" in tagstr:
       ModeType = "noise"

       QtPileup = False
       SampPileup = False
        
       if len(Qtstr.split("sigma")) > 1:
          print(Qtstr.split("sigma")[1])
          if Qtstr.split("sigma")[1].find("Pileup") > -1:
             QtPileup = True

       if len(Sampstr.split("sigma")) > 1:
          print(Sampstr.split("sigma")[1])
          if Sampstr.split("sigma")[1].find("Pileup") > -1:
             SampPileup = True

       print("Setting pileup noise to (Qt,Samp) = ",QtPileup,SampPileup)

       Qtstr=Qtstr.split("sigma")[0]
       Sampstr=Sampstr.split("sigma")[0]
    
    elif "HECFCAL" in tagstr:
       ModeType = "group"
       Qtstr="0" ## done manually
       Sampstr="0" ## done manually

    else:
       ModeType = "fixed"
       Qtstr=Qtstr.split("-")[0]

    QtVal=float(Qtstr)
    SampVal=float(Sampstr)
    print(QtVal,SampVal)

    flags.Input.Files=[]
    flags.Input.RunNumbers = [int(args.run),]

    flags.IOVDb.DBConnection  = "sqlite://;schema=DSPThresholdTemplates.db;dbname=CONDBR2"

    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

    flags.Detector.EnableID=False
    flags.Detector.EnableMuon=False
    flags.Detector.EnableForward=False

    flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2024-03'

    flags.Debug.DumpDetStore=True
    flags.Debug.DumpEvtStore=True
    flags.Debug.DumpCondStore=True

    if args.sql != "":
       flags.IOVDb.SqliteInput=args.sql
       fldrs=[]
       if args.localpileup:
          fldrs += ["/CALO/Ofl/Noise/PileUpNoiseLumi"]
       if args.localpileup:
          fldrs += ["/LAR/NoiseOfl/CellNoise"]
       flags.IOVDb.SqliteFolders=fldrs

    flags.LAr.doHVCorr=False

    flags.lock()
    flags.dump()

    cfg=MainServicesCfg(flags)

    cfg.merge(LArDSPThresholdCfg(flags,tag=tagstr,ModeType="noise",RunSince=0,fill=True,
                Sampval=SampVal, Qtval=QtVal, Samppileup=SampPileup, Qtpileup=QtPileup))

    from IOVDbSvc.IOVDbSvcConfig import addOverride        
    cfg.merge(addOverride(flags,"/CALO/Ofl/Noise/PileUpNoiseLumi","CALOOflNoisePileUpNoiseLumi-RUN2-UPD1-00"))
    if args.noisetag!="":
       cfg.merge(addOverride(flags,"/LAR/NoiseOfl/CellNoise",args.noisetag))


    cfg.run(2)
