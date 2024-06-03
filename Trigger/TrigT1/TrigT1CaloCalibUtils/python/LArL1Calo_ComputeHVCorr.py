#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory


if __name__=="__main__":

    import sys
    import argparse
    from time import strptime
    from calendar import timegm

    # now process the CL options and assign defaults
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-d','--date', dest='date', default='2024-04-30:00:00:00', help='date', type=str)
    parser.add_argument('-o','--outsqlite', dest='outsql', default="hvcorrections.sqlite", help='Output sqlite file', type=str)
    parser.add_argument('-t','--tag', dest='glbtag', default='CONDBR2-BLKPA-2024-02', help='Global tag to use', type=str)

    args = parser.parse_args()
    if help in args and args.help is not None and args.help:
        parser.print_help()
        sys.exit(0)

    try:
      ts=strptime(args.date+'/UTC','%Y-%m-%d:%H:%M:%S/%Z')
      TimeStamp=int(timegm(ts))*1000000000
    except ValueError:
      print("ERROR in time specification,",args.date,"  use e.g. 2024-05-25:14:01:00")
      

    from LArCalibProcessing.TimeStampToRunLumi import TimeStampToRunLumi

    rlb=TimeStampToRunLumi(TimeStamp)
    if rlb is None:
       print("WARNING: Failed to convert time",TimeStamp,"into a run/lumi number")
       RunNumber=999999
       LumiBlock=0
    else:
       RunNumber=rlb[0]
       LumiBlock=rlb[1]

    print("Working on run",RunNumber,"LB",LumiBlock,"Timestamp:",TimeStamp)
    
    #Import the MainServices (boilerplate)
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   
    #Import the flag-container that is the arguemnt to the configuration methods
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    flags=initConfigFlags()
    addLArCalibFlags(flags)

    # database folder
    LArHVScaleCorrFolder = "/LAR/ElecCalibOfl/HVScaleCorr"

    # output key
    keyOutput = "LArHVScaleCorr"

    # write IOV
    WriteIOV      = True

    # flags settings
    flags.IOVDb.DBConnection="sqlite://;schema="+args.outsql +";dbname=L1CALO"
    flags.IOVDb.GlobalTag = args.glbtag
    # geometry
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

    flags.Input.Files=[]
    flags.Input.RunNumbers=[RunNumber,]

    flags.Debug.DumpDetStore=True
    flags.Debug.DumpCondStore=True
    flags.Debug.DumpEvtStore=True

    flags.dump()
    flags.lock()

    cfg=MainServicesCfg(flags)
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg 
    cfg.merge(LArOnOffIdMappingCfg(flags))

    #MC Event selector since we have no input data file 
    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags,
                                    RunNumber         = flags.Input.RunNumbers[0],
                                    EventsPerRun      = 1,
                                    FirstEvent	      = 1,
                                    InitialTimeStamp  = 0,
                                    TimeStampInterval = 1))


    from LArCalibUtils.LArHVScaleConfig import LArHVScaleCfg
    cfg.merge(LArHVScaleCfg(flags))
 
    cfg.addEventAlgo(CompFactory.L1CaloHVDummyContainers())
 
    # setup l1calo database
    # FIXME:
    #include('TrigT1CaloCalibConditions/L1CaloCalibConditionsRun2_jobOptions.py')
    # hack because of deleted files from trigger:
    cfg.addService(CompFactory.L1CaloCondSvc())
    L1CaloFolderList = []
    L1CaloFolderList += ["/TRIGGER/L1Calo/V1/Calibration/PpmDeadChannels"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V1/Conditions/DisabledTowers"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V1/Conditions/RunParameters"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V1/Configuration/PprChanDefaults"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Configuration/PprChanDefaults"]
    
    L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Configuration/PprChanStrategy"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V1/Calibration/Physics/PprChanCalib"]
    
    L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Physics/PprChanCalib"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Physics/PprChanCommon"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Physics/PprChanLowMu"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Physics/PprChanHighMu"]
    
    L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib1/PprChanCalib"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib1/PprChanCommon"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib1/PprChanLowMu"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib1/PprChanHighMu"]
    
    L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib2/PprChanCalib"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib2/PprChanCommon"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib2/PprChanLowMu"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib2/PprChanHighMu"]
    
    L1CaloFolderList += ["/TRIGGER/L1Calo/V1/Conditions/RunParameters"]
    L1CaloFolderList += ["/TRIGGER/L1Calo/V1/Conditions/DerivedRunPars"]
    L1CaloFolderList += ["/TRIGGER/Receivers/Conditions/VgaDac"]
    L1CaloFolderList += ["/TRIGGER/Receivers/Conditions/Strategy"]
 
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    for l1calofolder in L1CaloFolderList:
       cfg.merge(addFolders(flags,l1calofolder, db="TRIGGER", tag="HEAD", className="CondAttrListCollection"))
    cfg.addCondAlgo(CompFactory.L1CaloCondAlg(UsePhysicsRegime = True, UseCalib1Regime = False, UseCalib2Regime = False))   
 
    from CaloConditions.CaloConditionsConfig import CaloTTIdMapCfg
    cfg.merge(CaloTTIdMapCfg(flags))   
 
    l1hvcorr = CompFactory.L1CaloHVCorrectionsForDB()
 
    from TrigT1CaloCalibTools.L1CaloCalibToolsConfig import L1CaloCells2TriggerTowersCfg
    cfg.merge(L1CaloCells2TriggerTowersCfg(flags,'L1CaloCells2TriggerTowers'))
 
    # set up tools
    #from TrigT1CaloTools.TrigT1CaloToolsConf import LVL1__L1TriggerTowerTool
    #ToolSvc += LVL1__L1TriggerTowerTool("L1TriggerTowerTool")
    #from TrigT1CaloCalibTools.TrigT1CaloCalibToolsConf import LVL1__L1CaloLArTowerEnergy
    #ToolSvc += LVL1__L1CaloLArTowerEnergy("L1CaloLArTowerEnergy")
    #from TrigT1CaloCalibTools.TrigT1CaloCalibToolsConf import LVL1__L1CaloOfflineTriggerTowerTools
    #ToolSvc += LVL1__L1CaloOfflineTriggerTowerTools("L1CaloOfflineTriggerTowerTools")
 
    cfg.addEventAlgo(l1hvcorr)
 
    # configure writing of calib database
    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    cfg.merge(OutputConditionsAlgCfg(flags, "HVCorrectionsOutput", outputFile="dummy.root",
                                     ObjectList = [ "CondAttrListCollection#/TRIGGER/L1Calo/V1/Results/RxLayers",
                                                    "CondAttrListCollection#/TRIGGER/L1Calo/V1/Results/HVCorrections"],
                                     WriteIOV = WriteIOV,
                                     RunNumber=RunNumber
                                    )
             )
    #RegistrationSvc    
    cfg.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = True,
                                                     SVFolder=True,
                                                 ))
 
    cfg.getService("IOVDbSvc").DBInstance=""
 
    print("Start running...")
    cfg.run(1)
