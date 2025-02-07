#!/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory


if __name__=='__main__':

  import os,sys
  import argparse

  # now process the CL options and assign defaults
  parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument('-r','--run', dest='run', default=0x7fffffff, help='Run number', type=int)
  parser.add_argument('--sqlite', dest='sqlite', default=None, help='sqlite file to read from (default: oracle)', type=str)
  parser.add_argument('-t','--tag',dest='dbtag',default=None,help="Global conditions tag", type=str)
  parser.add_argument('-f','--ftag',dest='ftag',default=None,help="folder tag suffig", type=str)
  parser.add_argument('-o','--out', dest='out', default="LArConditions.root", help='Output root file', type=str)
  parser.add_argument('--ofcfolder',dest='ofcfolder',default="", help="OFC flavor",type=str)
  parser.add_argument('-s','--isSC', dest='isSC', action='store_true', default=False, help='is SC?')
  parser.add_argument('-m','--isMC', dest='isMC', action='store_true', default=False, help='is MC?')

  parser.add_argument("--objects",dest="objects",default="PEDESTAL,RAMP",help="List of conditions types to be dumped",type=str)
  parser.add_argument("--folders",dest="folders",default="/LAR/ElecCalibFlat/Pedestal,/LAR/ElecCalibFlat/Ramp",help="List of folders to be taken from sqlite",type=str)
  parser.add_argument('--offline',dest="offline", action='store_true', default=False, help='is offline folder?')
  parser.add_argument('--poolcat',dest="poolcat", default="", type=str, help='is offline folder?')
  parser.add_argument('-n', '--ntuple', dest='ntname', default='', help='output ntuple name (if different from default)', type=str)
  parser.add_argument('--oLevel', dest='olevel', default=3, help='OutputLevel of the job', type=int)
   
  args = parser.parse_args()
  if help in args and args.help is not None and args.help:
    parser.print_help()
    sys.exit(0)


  #Translation table ... with a few potential variant spellings
  objTable={"RAMP":"Ramp",
            "DAC2UA":"DAC2uA",
            "DACUA":"DAC2uA",
            "PEDESTAL":"Pedestal",
            "PED":"Pedestal",
            "UA2MEV":"uA2MeV",
            "UAMEV":"uA2MeV",
            "MPHYSOVERMCAL":"MphysOverMcal",
            "MPHYSMCAL":"MphysOverMcal",
            "MPMC":"MphysOverMcal",
            "OFC":"OFC",
            "SHAPE":"Shape",
            "HVSCALECORR":"HVScaleCorr",
            "HVSCALE":"HVScaleCorr",
            "FSAMPL":"fSampl",
            "AUTOCORR":"AutoCorr",
            "AC":"AutoCorr",
            "CALIWAVE":"CaliWave",
            "PHYSWAVE":"PhysWave",
            "OFCCALI":"OFCCali",
            "ACORR":"AutoCorr",    
            "DSPTHR":"DSPThr",
            "MINBIAS":"MinBias",
            "PHYSAC":"PhysAutoCorr",    
          }

  objects=set()
  objectsOnl=set() 
  for obj in args.objects.split(","):
    objU=obj.upper()
    if objU not in objTable:
      print("ERROR: Unknown conditions type",obj)
      sys.exit(0)

    objects.add(objTable[objU])
    if "OFCCALI" not in obj.upper() and 'WAVE' not in obj.upper() and 'DSPTHR' not in obj.upper() and 'MINBIAS' not in obj.upper() and not args.offline:
       objectsOnl.add(objTable[objU])
    
  flds=set()
  for fld in args.folders.split(","):
     flds.add(fld)
 
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  flags=initConfigFlags()
  from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
  addLArCalibFlags(flags, args.isSC)
   
  flags.Input.RunNumbers=[args.run]
  from AthenaConfiguration.TestDefaults import defaultGeometryTags
  flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3

  flags.Input.Files=[]

  flags.Input.isMC=args.isMC
  flags.LArCalib.isSC=args.isSC

  flags.LAr.doAlign=False

  flags.LAr.OFCShapeFolder=args.ofcfolder

  from AthenaConfiguration.Enums import LHCPeriod
  if flags.Input.RunNumbers[0] < 222222:
    #Set to run1 for early run-numbers
    flags.GeoModel.Run=LHCPeriod.Run1 
    flags.IOVDb.DatabaseInstance="OFLP200" if flags.Input.isMC else "COMP200" 
  else:
    flags.GeoModel.Run=LHCPeriod.Run2
    flags.IOVDb.DatabaseInstance="OFLP200" if flags.Input.isMC else "CONDBR2"

  if args.dbtag:
    flags.IOVDb.GlobalTag=args.dbtag
  elif flags.Input.isMC:
    flags.IOVDb.GlobalTag="OFLCOND-MC16-SDR-20"
  elif flags.IOVDb.DatabaseInstance == "COMP200":
    flags.IOVDb.GlobalTag="COMCOND-BLKPA-RUN1-09"
  else: 
    flags.IOVDb.GlobalTag="CONDBR2-ES1PA-2024-01"

  flags.Exec.OutputLevel=args.olevel
  flags.Debug.DumpCondStore=True
  flags.Debug.DumpDetStore=True

  if (args.sqlite):
    flags.IOVDb.SqliteInput=args.sqlite
    flags.IOVDb.SqliteFolders=tuple(flds)
  if len(objects)!=len(objectsOnl):
    flags.IOVDb.DBConnection="COOLOFL_LAR/CONDBR2" 

  flags.lock()
  
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  cfg=MainServicesCfg(flags)

  #MC Event selector since we have no input data file
  from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
  cfg.merge(McEventSelectorCfg(flags,
                               EventsPerRun      = 1,
                               FirstEvent	      = 1,
                               InitialTimeStamp  = 0,
                               TimeStampInterval = 1))



  #Get LAr basic services and cond-algos
  from LArGeoAlgsNV.LArGMConfig import LArGMCfg
  cfg.merge(LArGMCfg(flags))

  if flags.LArCalib.isSC:
    #Setup SuperCell cabling
    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg, LArCalibIdMappingSCCfg, LArLATOMEMappingCfg
    cfg.merge(LArOnOffIdMappingSCCfg(flags))
    cfg.merge(LArCalibIdMappingSCCfg(flags))
    cfg.merge(LArLATOMEMappingCfg(flags))
    if not args.offline:
       from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBSCCfg
       cfg.merge(LArElecCalibDBSCCfg(flags,objectsOnl))
  else: 
    #Setup regular cabling
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg, LArCalibIdMappingCfg
    cfg.merge(LArOnOffIdMappingCfg(flags))
    cfg.merge(LArCalibIdMappingCfg(flags))
    if not args.offline:
       from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
       cfg.merge(LArElecCalibDBCfg(flags,objectsOnl))
  
  from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
  cfg.merge(LArBadChannelCfg(flags, isSC=flags.LArCalib.isSC))

  bcKey = "LArBadChannelSC" if flags.LArCalib.isSC else "LArBadChannel"

  if "Pedestal" in objects:
    ckey = "LArPedestalSC" if flags.LArCalib.isSC else "LArPedestal"
    cfg.addEventAlgo(CompFactory.LArPedestals2Ntuple(ContainerKey = ckey,
                                                        AddFEBTempInfo = False, 
                                                        AddCalib = True,
                                                        isSC = flags.LArCalib.isSC,
                                                        BadChanKey = bcKey
                                                      ))
                      
  if "AutoCorr" in objects:
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    if flags.LArCalib.isSC:
       cfg.merge(addFolders(flags,'/LAR/ElecCalibOflSC/AutoCorrs/AutoCorr',modifiers='<key>LArAutoCorrSC</key>',className='LArAutoCorrComplete'))
    else:   
       if args.ftag:
          cfg.merge(addFolders(flags,'/LAR/ElecCalibOfl/AutoCorrs/AutoCorr',tag="".join('/LAR/ElecCalibOfl/AutoCorrs/AutoCorr'.split('/')) + args.ftag))
       else:
          cfg.merge(addFolders(flags,'/LAR/ElecCalibOfl/AutoCorrs/AutoCorr',className='LArAutoCorrComplete'))
    ckey="LArAutoCorrSC" if flags.LArCalib.isSC else "LArAutoCorr"
    cfg.addEventAlgo(CompFactory.LArAutoCorr2Ntuple(ContainerKey = "LArAutoCorrSym" if flags.Input.isMC else ckey,
                                                    AddFEBTempInfo = False, 
                                                    AddCalib = True,
                                                    isSC = flags.LArCalib.isSC,
                                                    ApplyCorrection = True,
                                                    AddCorrUndo = True,
                                                    BadChanKey = bcKey,

                                                  ))

  if "PhysAutoCorr" in objects:
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    if flags.LArCalib.isSC:
       cfg.merge(addFolders(flags,'/LAR/ElecCalibOflSC/AutoCorrs/PhysicsAutoCorr',modifiers='<key>LArAutoCorrSC</key>',className='LArAutoCorrComplete'))
    else:   
       if args.ftag:
          cfg.merge(addFolders(flags,'/LAR/ElecCalibOfl/AutoCorrs/PhysicsAutoCorr',tag="".join('/LAR/ElecCalibOfl/AutoCorrs/PhysicsAutoCorr'.split('/')) + args.ftag))
       else:
          cfg.merge(addFolders(flags,'/LAR/ElecCalibOfl/AutoCorrs/PhysicsAutoCorr',className='LArAutoCorrComplete'))
    ckey="LArPhysAutoCorrSC" if flags.LArCalib.isSC else "LArPhysAutoCorr"
    cfg.addEventAlgo(CompFactory.LArAutoCorr2Ntuple(ContainerKey =  ckey,
                                                     AddFEBTempInfo = False, 
                                                     AddCalib = True,
                                                     isSC = flags.LArCalib.isSC,
                                                     ApplyCorrection = not flags.Input.isMC,
                                                     AddCorrUndo = not flags.Input.isMC,
                                                     BadChanKey = bcKey,
 
                                                   ))

  if "Ramp" in objects:
    ckey = "LArRampSC" if flags.LArCalib.isSC else "LArRamp"
    if args.offline:
       from IOVDbSvc.IOVDbSvcConfig import addFolders
       if flags.LArCalib.isSC:
          cfg.merge(addFolders(flags,'/LAR/ElecCalibOflSC/Ramps/RampLinea',modifiers='<key>LArRampSC</key>',className='LArRampComplete'))
       else:   
          cfg.merge(addFolders(flags,'/LAR/ElecCalibOfl/Ramps/RampLinea',className='LArRampComplete'))
    cfg.addEventAlgo(CompFactory.LArRamps2Ntuple(RampKey="LArRampSym" if flags.Input.isMC else ckey,
                                                 AddFEBTempInfo = False, 
                                                 AddCalib = True,
                                                 isSC = flags.LArCalib.isSC,
                                                 ApplyCorr = True,
                                                 AddCorrUndo = True,
                                                 BadChanKey = bcKey
                                               ))
  
  if "OFC" in objects:
    if args.offline: 
       from IOVDbSvc.IOVDbSvcConfig import addFolders
       for fld in flds:
          if 'OFC' in fld:
             if args.ftag:
                cfg.merge(addFolders(flags,fld,tag="".join(foldername.split('/')) + args.ftag))
             else:
                cfg.merge(addFolders(flags,fld))
             ckey= 'LArOFC' if '1phase' in fld else 'LArLArOFCPhys4samples'
             ntname= 'OFC' if '1phase' in fld  else 'OFC_1ns'
             break
    else:         
       ckey = "LArOFCSC" if flags.LArCalib.isSC else "LArOFC"
       ntname = 'OFC'
    cfg.addEventAlgo(CompFactory.LArOFC2Ntuple(AddFEBTempInfo   = False,   
                                               ContainerKey=ckey,
                                               NtupleName=ntname,
                                               isSC = flags.LArCalib.isSC,
                                               BadChanKey = bcKey
                                             ))

  if "OFCCali" in objects:
    if args.offline: 
       from IOVDbSvc.IOVDbSvcConfig import addFolders
       for fld in flds:
          if 'OFC' in fld:
             if args.ftag:
                cfg.merge(addFolders(flags,fld,tag="".join(foldername.split('/')) + args.ftag))
             else:
                cfg.merge(addFolders(flags,fld))
             ckey= 'LArOFC' if '1phase' in fld else 'LArOFC'
             break
    else:         
       ckey = "LArOFCSCCali" if flags.LArCalib.isSC else "LArOFCCali"
       fldr = "/LAR/ElecCalibFlatSC/OFCCali" if flags.LArCalib.isSC else "/LAR/ElecCalibFlat/OFCCali"
       dbString = "<db>sqlite://;schema="+args.sqlite+";dbname=CONDBR2" if args.sqlite else  "<db>COOLONL_LAR/CONDBR2</db>"
       from IOVDbSvc.IOVDbSvcConfig import addFolders
       cfg.merge(addFolders(flags,fldr,detDb=dbString,className="CondAttrListCollection"))
       LArOFCSCCondAlg  =  CompFactory.getComp("LArFlatConditionsAlg<LArOFCSC>")("LArOFCSCCaliCondAlg")
       LArOFCSCCondAlg.ReadKey=fldr
       LArOFCSCCondAlg. WriteKey=ckey
       cfg.addCondAlgo(LArOFCSCCondAlg)

    cfg.addEventAlgo(CompFactory.LArOFC2Ntuple("LArOFC2NtupleCali",
                                               AddFEBTempInfo   = False,   
                                               ContainerKey=ckey,
                                               NtupleName="OFCCali" if args.ntname=='' else args.ntname,
                                               isSC = flags.LArCalib.isSC,
                                               BadChanKey = bcKey
                                             ))


  if "Shape" in objects:
    ckey = "LArShapeSC" if flags.LArCalib.isSC else "LArShape"
    if flags.Input.isMC:
       ckey="LArShapeSym"
    cfg.addEventAlgo(CompFactory.LArShape2Ntuple(ContainerKey=ckey,
                                                 AddFEBTempInfo   = False,   
                                                 AddCalib = True,
                                                 isSC = flags.LArCalib.isSC,
                                                 BadChanKey = bcKey
               
                                               ))
  if "MphysOverMcal" in objects:
    cfg.addEventAlgo(CompFactory.LArMphysOverMcal2Ntuple(ContainerKey   = "LArMphysOverMcalSC" if flags.LArCalib.isSC else "LArMphysOverMcal",
                                                         AddFEBTempInfo   = False,
                                                         AddCalib = True,
                                                         isSC = flags.LArCalib.isSC,
                                                         BadChanKey = bcKey
                                                       ))

  #ADC2MeV and DACuA are handled by the same ntuple dumper
  if "DAC2uA" in objects or "uA2MeV" in objects:
    uackey = "LAruA2MeVSC" if flags.LArCalib.isSC else "LAruA2MeV"
    dackey = "LArDAC2uASC" if flags.LArCalib.isSC else "LArDAC2uA"
    ua2MeVKey="LAruA2MeVSym" if flags.Input.isMC else uackey
    dac2uAKey="LArDAC2uASym" if flags.Input.isMC else dackey

    cfg.addEventAlgo(CompFactory.LAruA2MeV2Ntuple(uA2MeVKey=ua2MeVKey if "uA2MeV" in objects else "",
                                                  DAC2uAKey=dac2uAKey if "DAC2uA" in objects else "",
                                                  isSC = flags.LArCalib.isSC,
                                                  BadChanKey = bcKey
                                                ))
    

  if "HVScaleCorr" in objects:
    # hack to read from sqlite created by HV computations
    iovDbSvc=cfg.getService("IOVDbSvc")
    for i in range(0,len(iovDbSvc.Folders)):
          if (iovDbSvc.Folders[i].find("HVScaleCorr")>=0):
              iovDbSvc.Folders[i]+="<key>/LAR/ElecCalibFlatSC/HVScaleCorr</key>"

    cfg.addEventAlgo(CompFactory.LArHVScaleCorr2Ntuple(ContainerKey= "LArHVScaleCorrSC" if flags.LArCalib.isSC else "LArHVScaleCorr",
                                                       AddFEBTempInfo = False,
                                                       isSC = flags.LArCalib.isSC,
                                                       BadChanKey = bcKey
                                                     ))

  if "fSampl" in objects:
    cfg.addEventAlgo(CompFactory.LArfSampl2Ntuple(ContainerKey="LArfSamplSC" if flags.LArCalib.isSC else "LArfSamplSym",
                                                  isSC=flags.LArCalib.isSC
                                                ))

  if "CaliWave" in objects:
    if flags.Input.isMC:
       print('No CaliWave in MC')
    else:   
       fld = "/LAR/ElecCalibOflSC/CaliWaves/CaliWave" if flags.LArCalib.isSC else "/LAR/ElecCalibOfl/CaliWaves/CaliWave"   
       from IOVDbSvc.IOVDbSvcConfig import addFolders
       cfg.merge(addFolders(flags,fld,modifiers='<key>LArCaliWave</key><typeName>LArCaliWaveContainer</typeName>'))
       cfg.addEventAlgo(CompFactory.LArCaliWaves2Ntuple(KeyList = ["LArCaliWave"],
                                                 NtupleName = "CALIWAVE",
                                                 AddFEBTempInfo = False,   
                                                 SaveDerivedInfo = True,
                                                 AddCalib = True,
                                                 SaveJitter = True,
                                                 isFlat = False,
                                                 isSC = flags.LArCalib.isSC,
                                                 BadChanKey = bcKey
                                               ))

  if "PhysWave" in objects:
    if flags.Input.isMC:
       print('No PhysWave in MC yet')
    else:   
       fld = "/LAR/ElecCalibOflSC/PhysWaves/RTM" if flags.LArCalib.isSC else "/LAR/ElecCalibOfl/PhysWaves/RTM"   
       from IOVDbSvc.IOVDbSvcConfig import addFolders
       cfg.merge(addFolders(flags,fld))
       cfg.addEventAlgo(CompFactory.LArPhysWaves2Ntuple(KeyList = ["LArPhysWave"],
                                                 NtupleName = "PHYSWAVE",
                                                 AddFEBTempInfo = False,   
                                                 SaveDerivedInfo = True,
                                                 AddCalib = True,
                                                 isFlat = False,
                                                 isSC = flags.LArCalib.isSC,
                                                 BadChanKey = bcKey
                                               ))

  if "DSPThr" in objects:
     from IOVDbSvc.IOVDbSvcConfig import addFolders
     cfg.merge(addFolders(flags,"/LAR/Configuration/DSPThresholdFlat/Thresholds",detDb="LAR_ONL"))  
     cfg.addEventAlgo(CompFactory.LArDSPThresholds2Ntuple(DumpFlat=True,FlatFolder="/LAR/Configuration/DSPThresholdFlat/Thresholds"))

  if "MinBias" in objects:
     #FIXME different for MC
     from IOVDbSvc.IOVDbSvcConfig import addFolders
     if args.offline:
        myfld="/LAR/ElecCalibOfl/LArPileupAverage"
        mydb="LAR_OFL"
     else:
        myfld="/LAR/LArPileup/LArPileupAverage"
        mydb="LAR_ONL"  
     if args.ftag:
        cfg.merge(addFolders(flags,myfld,detDb=mydb,className="LArMinBiasAverageMC",tag="".join(myfld.split('/')) + args.ftag))  
     else:
        cfg.merge(addFolders(flags,myfld,detDb=mydb,className="LArMinBiasAverageMC"))  
     LArMinBiasAverageSymAlg =  CompFactory.getComp("LArSymConditionsAlg<LArMinBiasAverageMC, LArMinBiasAverageSym>")    
     LArMCSymCondAlg=CompFactory.LArMCSymCondAlg
     cfg.addCondAlgo(LArMCSymCondAlg(ReadKey="LArOnOffIdMap"))
     cfg.addCondAlgo(LArMinBiasAverageSymAlg(ReadKey="LArPileupAverage",WriteKey="LArSymPileupAverage"))
     cfg.addEventAlgo(CompFactory.LArMinBias2Ntuple(ContainerKey="",ContainerKeyAv="LArSymPileupAverage"))
  
  rootfile=args.out
  if os.path.exists(rootfile):
    os.remove(rootfile)
  cfg.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+rootfile+"' OPT='NEW'" ]))
  cfg.setAppProperty("HistogramPersistency","ROOT")

  if args.dbtag and 'CALIB' in args.dbtag:
     cfg.getService("IOVDbSvc").DBInstance=""
    
  if args.poolcat:
     cfg.getService("PoolSvc").ReadCatalog+=["xmlcatalog_file:%s"%args.poolcat,]  

  cfg.run(1)
  sys.exit(0)
  
