#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import Format
from AthenaConfiguration.Enums import LHCPeriod

from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
from OutputStreamAthenaPool.OutputStreamConfig import addToESD

from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg

from ZdcRec.SetConfigTagFromFlags import SetConfigTag

from ZdcNtuple.ZdcNtupleConfig import ZdcNtupleCfg
    
# FIXME: removing for MC
from TrigConfigSvc.TriggerConfigAccess import getL1MenuAccess
# added getRun3NavigationContainerFromInput as per Tim Martin's suggestions
from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg, getRun3NavigationContainerFromInput
    

def ZdcRecOutputCfg(flags):

    acc = ComponentAccumulator()

    ZDC_ItemList=[]
    if flags.Input.Format is Format.BS:
        ZDC_ItemList.append("xAOD::ZdcModuleContainer#ZdcModules")
        ZDC_ItemList.append("xAOD::ZdcModuleAuxContainer#ZdcModulesAux.")
        ZDC_ItemList.append("xAOD::ZdcModuleContainer#ZdcSums")
        ZDC_ItemList.append("xAOD::ZdcModuleAuxContainer#ZdcSumsAux.")

    acc.merge(addToESD(flags,ZDC_ItemList))
    acc.merge(addToAOD(flags,ZDC_ItemList))

    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    acc.merge(SetupMetaDataForStreamCfg(flags,streamName="AOD"))
    

    return acc


def ZdcAnalysisToolCfg(flags, run, config="PbPb2023", DoCalib=False, DoTimeCalib=False, DoTrigEff=False, ForceCalibRun=-1, ForceCalibLB=814):
    acc = ComponentAccumulator()

    print('ZdcAnalysisToolCfg: setting up ZdcAnalysisTool with config='+config)

    acc.setPrivateTools(CompFactory.ZDC.ZdcAnalysisTool(
        name = 'ZdcAnalysisTool'+config, 
        Configuration = config,
        DoCalib = DoCalib,
        DoTimeCalib = DoTimeCalib,
        DoTrigEff = DoTrigEff,
        ForceCalibRun = ForceCalibRun,
        ForceCalibLB = ForceCalibLB, 
        LHCRun = run ))
    return acc

def ZdcLEDAnalysisToolCfg(flags, config = 'ppPbPb2023'):  
    acc = ComponentAccumulator()

    print('ZdcAnalysisToolCfg: setting up ZdcAnalysisTool with config='+config)
    acc.setPrivateTools(CompFactory.ZDC.ZdcLEDAnalysisTool(name = 'ZdcLEDAnalysisTool'+config, 
                                                           Configuration = config))
    return acc


def ZdcTrigValToolCfg(flags, config = 'PbPb2023'):
    acc = ComponentAccumulator()
    
    acc.merge(TrigDecisionToolCfg(flags))
    
    trigValTool = CompFactory.ZDC.ZdcTrigValidTool(
        name = 'ZdcTrigValTool',
        WriteAux = True,
        AuxSuffix = '',
        filepath_LUT = 'TrigT1ZDC/zdc_json_PbPb5.36TeV_2023.json') # changed on Oct 13 to accomodate Zdc 1n peak shift
        
    trigValTool.TrigDecisionTool = acc.getPublicTool('TrigDecisionTool')
    
    trigValTool.triggerList = [c for c in getL1MenuAccess(flags) if 'L1_ZDC_BIT' in c]
    
    acc.setPrivateTools(trigValTool)
      
    return acc

def RPDAnalysisToolCfg(flags):
    acc = ComponentAccumulator()
    acc.setPrivateTools(
        CompFactory.ZDC.RPDAnalysisTool(name="RPDAnalysisTool")
    )
    return acc

def RpdSubtractCentroidToolCfg(flags):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ZDC.RpdSubtractCentroidTool(name = 'RpdSubtractCentroidTool'))
    return acc

def ZdcRecRun2Cfg(flags):        
    acc = ComponentAccumulator()
    config = SetConfigTag(flags)
    print ('ZdcRecConfig.py: Running with config tag ', config)

    doCalib = False
    doTimeCalib = False
    doTrigEff = False

    if flags.Input.ProjectName == "data15_hi":
        doCalib = True
        doTimeCalib = True
        doTrigEff = True
    elif flags.Input.ProjectName == "data17_13TeV":
        doCalib = False
        doTimeCalib = False
        doTrigEff = False
    elif flags.Input.ProjectName == "data16_hip":
        doCalib = True
        doTimeCalib = False
        doTrigEff = False
    elif flags.Input.ProjectName == "data18_hi":
        doCalib = True
        doTimeCalib = False
        doTrigEff = False

    acc.merge(ByteStreamReadCfg(flags, type_names=['xAOD::TriggerTowerContainer/ZdcTriggerTowers',
                                         'xAOD::TriggerTowerAuxContainer/ZdcTriggerTowersAux.']))

    acc.addEventAlgo(CompFactory.ZdcByteStreamRawDataV2())
    acc.addEventAlgo(CompFactory.ZdcRecV3Decode())

    anaTool = acc.popToolsAndMerge(ZdcAnalysisToolCfg(flags,2,config,doCalib,doTimeCalib,doTrigEff))

    acc.addEventAlgo(CompFactory.ZdcRecV3("ZdcRecV3",ZdcAnalysisTool=anaTool))

    return acc

def ZdcRecRun3Cfg(flags):

    acc = ComponentAccumulator()
    config = SetConfigTag(flags)
    print ('ZdcRecConfig.py: Running with config tag ', config)

    doCalib = False
    doTimeCalib = False
    doTrigEff = False
    ForceCalibRun = -1
    ForceCalibLB = 814
    
    if flags.Input.TriggerStream != "calibration_ZDCInjCalib" and flags.Input.TriggerStream != "calibration_DcmDummyProcessor":
        if flags.Input.ProjectName == "data23_comm":
            doCalib = True
        elif flags.Input.ProjectName == "data23_hi": # for "data24_hi" or "data24_5p36TeV," need to also check flags.Input.TriggerStream != "calibration_ZDCInjCalib"
            doCalib = True
            doTimeCalib = True
        elif flags.Input.ProjectName == "data24_hi": # for "data24_hi" or "data24_5p36TeV," need to also check flags.Input.TriggerStream != "calibration_ZDCInjCalib"
            doCalib = True
            doTimeCalib = True
            ForceCalibRun = 463427
            ForceCalibLB = 500
        elif flags.Common.isOnline:
            doCalib = True
            doTimeCalib = True
            ForceCalibRun = 463427
            ForceCalibLB = 500

    doRPD = flags.Detector.EnableZDC_RPD

    print('ZdcRecRun3Cfg: doCalib = '+str(doCalib)+' for project '+flags.Input.ProjectName)
    print('RPD enable flag is '+str(doRPD))
    
    anaTool = acc.popToolsAndMerge(ZdcAnalysisToolCfg(flags,3,config,doCalib,doTimeCalib,doTrigEff,ForceCalibRun,ForceCalibLB))

    if (doRPD):
        rpdAnaTool = acc.popToolsAndMerge(RPDAnalysisToolCfg(flags))
        centroidTool = acc.popToolsAndMerge(RpdSubtractCentroidToolCfg(flags))

    if ( flags.Input.isMC ):
        zdcTools = [anaTool] # expand list as needed
        if doRPD:
            zdcTools += [rpdAnaTool,centroidTool]
    elif ( flags.Trigger.doZDC ): # if doZDC flag is true we are in a trigger reprocessing -> no TrigValidTool
        zdcTools = [anaTool] # expand list as needed
    elif flags.Input.TriggerStream == "calibration_ZDCInjCalib" or flags.Input.TriggerStream == "calibration_DcmDummyProcessor":
        zdcTools = [anaTool] # no trigger / RPD / centroid - either online or offline
    elif (flags.Common.isOnline): # running online + NOT injector pulse: with RPD + centroid but no trigger validation for now; may add later
        zdcTools = [anaTool] # expand list as needed
        if doRPD:
            zdcTools += [rpdAnaTool,centroidTool]
    else: # default (not MC, not trigger repoc, not injector pulse, not online)
        trigTool = acc.popToolsAndMerge(ZdcTrigValToolCfg(flags,config))  
        zdcTools = [anaTool,trigTool] # expand list as needed
        if doRPD:
            zdcTools += [rpdAnaTool,centroidTool]
    
    if flags.Input.Format is Format.BS:
        acc.addEventAlgo(CompFactory.ZdcByteStreamLucrodData())
        acc.addEventAlgo(CompFactory.ZdcRecRun3Decode())
    if flags.Input.isMC:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        acc.merge(PoolReadCfg(flags))
        acc.addEventAlgo(CompFactory.ZdcMCTruthAlg())

    zdcAlg = CompFactory.ZdcRecRun3("ZdcRecRun3",ZdcAnalysisTools=zdcTools)
    acc.addEventAlgo(zdcAlg, primary=True)

    return acc

def ZdcNtupleLocalCfg(flags):
    
    acc = ComponentAccumulator()
    run = flags.GeoModel.Run

    if (run == LHCPeriod.Run2):
        print ('ZdcRecConfig.py: setting up Run 2 ntuple!')
        acc.merge(ZdcNtupleRun2Cfg(flags))
    elif (run == LHCPeriod.Run3):
        print ('ZdcRecConfig.py: setting up Run 3 ntuples!')
        acc.merge(ZdcNtupleRun3Cfg(flags))
    else:
        print ('ZdcRecConfig.py: setting up no ntuple!')

    return acc

def ZdcNtupleRun2Cfg(flags,**kwargs):

    acc = ComponentAccumulator()
    acc.merge(ZdcNtupleCfg(flags,
                           useGRL = False,
                           zdcOnly = True,
                           enableTrigger = False,
                           enableOutputSamples = True,
                           enableOutputTree = True,
                           writeOnlyTriggers = False,
                           nsamplesZdc = 7,
                           **kwargs))

    acc.addService(CompFactory.THistSvc(Output = ["ANALYSIS DATAFILE='zdctree.root' OPT='RECREATE'"]))
#    acc.setAppProperty("HistogramPersistency","ROOT")
    return acc

def ZdcNtupleRun3Cfg(flags,**kwargs):
    
    acc = ComponentAccumulator()
    acc.merge(ZdcNtupleCfg(flags,
                           useGRL = False,
                           zdcOnly = True,
                           lhcf2022 = False,
                           lhcf2022zdc = False,
                           lhcf2022afp = False,
                           enableTrigger = not flags.Input.isMC,
                           enableOutputSamples = True,
                           enableOutputTree = True,
                           writeOnlyTriggers = False,
                           enableRPD = flags.Input.TriggerStream != "calibration_ZDCInjCalib" and flags.Input.TriggerStream != "calibration_DcmDummyProcessor",
                           enableCentroid = flags.Input.TriggerStream != "calibration_ZDCInjCalib" and flags.Input.TriggerStream != "calibration_DcmDummyProcessor",
                           reprocZdc = False,
                           **kwargs))

    acc.addService(CompFactory.THistSvc(Output = ["ANALYSIS DATAFILE='NTUP.root' OPT='RECREATE'"]))
    #acc.setAppProperty("HistogramPersistency","ROOT")
    return acc

def ZdcLEDNtupleCfg(flags):
    acc = ComponentAccumulator()
    zdcLEDNtuple = CompFactory.ZdcLEDNtuple("ZdcLEDNtuple")
    zdcLEDNtuple.enableOutputTree = True
    acc.addEventAlgo(zdcLEDNtuple)
    acc.addService(CompFactory.THistSvc(Output = ["ANALYSIS DATAFILE='NTUP.root' OPT='RECREATE'"]))    
    return acc

def ZdcLEDRecCfg(flags):

    acc = ComponentAccumulator()
    
    if flags.Input.Format is Format.BS:
        run = flags.GeoModel.Run
        
        # debugging message since the metadata isn't working for calibration files yet
        print ("ZdcRecConfig.py: run = "+run.name)
        
        config = 'ppPbPb2023'
        #config = 'ppALFA2023'

        acc.addEventAlgo(CompFactory.ZdcByteStreamLucrodData())
        acc.addEventAlgo(CompFactory.ZdcRecRun3Decode())

        anaTool = acc.popToolsAndMerge(ZdcLEDAnalysisToolCfg(flags, config)) #anatool for zdcLED calibration  
    
        zdcTools = []
        zdcTools += [anaTool] # add trigTool after deocration migration
    
        # FIXME these are dependent on !65768
        zdcAlg = CompFactory.ZdcRecRun3("ZdcRecRun3",DAQMode=2, ForcedEventType=2, ZdcAnalysisTools=zdcTools) # DAQMode set to PhysicsPEB, event type set to ZdcEventLED
        acc.addEventAlgo(zdcAlg, primary=True)

    if flags.Output.doWriteESD or flags.Output.doWriteAOD:
        acc.merge(ZdcRecOutputCfg(flags))
        
    return acc

def ZdcLEDTrigCfg(flags):

    acc = ComponentAccumulator()

    # suggested by Tim Martin
    tdmv = CompFactory.TrigDec.TrigDecisionMakerValidator()			 
    tdmv.errorOnFailure = True
    tdmv.TrigDecisionTool = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    tdmv.NavigationKey = getRun3NavigationContainerFromInput(flags)
    acc.addEventAlgo( tdmv )
    # end of Tim's suggestions
    return acc

def ZdcRecCfg(flags):    
    """Configure Zdc analysis alg
    Additional arguments are useful in calibration runs
    """

    acc = ComponentAccumulator()
 
    run = flags.GeoModel.Run

    # debugging message since the metadata isn't working for calibration files yet
    print ("ZdcRecConfig.py: run = "+run.name)

    if (run == LHCPeriod.Run2):
        print ('ZdcRecConfig.py: setting up Run 2!')
        acc.merge(ZdcRecRun2Cfg(flags))
    elif (run == LHCPeriod.Run3):
        print ('ZdcRecConfig.py: setting up Run 3!')
        acc.merge(ZdcRecRun3Cfg(flags))
    else:
        print ('ZdcRecConfig.py: setting up nothing (problem)!')

    if flags.Output.doWriteESD or flags.Output.doWriteAOD:
        acc.merge(ZdcRecOutputCfg(flags))

    return acc

if __name__ == '__main__':

    """ This is selftest & ZDC calibration transform at the same time"""
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.TestDefaults import defaultGeometryTags

    flags = initConfigFlags()

    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.EnableVerboseViews = True

    flags.Detector.GeometryZDC=True
    flags.Detector.GeometryAFP=False
    flags.Detector.GeometryALFA=False
    flags.Detector.GeometryLucid=False
    flags.Detector.GeometryMDT=False
    flags.Detector.GeometryMM=False
    flags.Detector.GeometryMuon=False
    flags.Trigger.decodeHLT=False
    flags.Trigger.enableL1MuonPhase1=False
    flags.Trigger.L1.doMuon=False
    flags.Trigger.L1.doCalo=False
    flags.Trigger.L1.doTopo=False
    #flags.Reco.EnableTrigger = False

    # This does not work in this context
    # run = flags.GeoModel.Run
    # The EDM Version should be auto configured, but is not working at the moment, so is set by hand

    flags.Output.AODFileName="AOD.pool.root"
    flags.Output.HISTFileName="HIST.root"
    flags.Output.doWriteAOD=True

    parser = flags.getArgumentParser()
    parser.add_argument('--runCalibForStandaloneData',default="Calib",help="indicate if we run calib/LED reconstruction for standalone data: Calib (default) --> run calib reconstruction for injector-pulse events; LED --> run LED reconstruction for LED events")
    args = flags.fillFromArgs(parser=parser)

    # check for LED / calibration data running, and configure appropriately
    isLED = (flags.Input.TriggerStream == "calibration_ZDCLEDCalib")
    isCalib = (flags.Input.TriggerStream == "calibration_ZDCCalib" or flags.Input.TriggerStream == "calibration_ZDCInjCalib" or flags.Input.TriggerStream == "physics_MinBias" or flags.Input.TriggerStream == "express_express")
    if flags.Input.TriggerStream == "calibration_DcmDummyProcessor": # standalone data: do we want to run calibration or LED?
        if args.runCalibForStandaloneData == "Calib" or args.runCalibForStandaloneData == "calib":
            isCalib = True
        elif args.runCalibForStandaloneData == "LED" or args.runCalibForStandaloneData == "led":
            isLED = True
        else:
            print('WARNING: The value for the argument runCalibForStandaloneData is invalid')
            print('Running nominal reconstruction (injector-pulse) by default')
            isCalib = True

    if (isLED):
       print('ZdcRecConfig: Running LED data!')
    if (isCalib):
       print('ZdcRecConfig: Running ZDC calibration data!')
    if (flags.Input.isMC):
       print('ZdcRecConfig: Running over MC Samples')
       flags.Input.ProjectName = "data23_hi"
 
    # supply missing metadata based on project name
    pn = flags.Input.ProjectName
    if not pn:
        raise ValueError('Unknown project name')
    

    if isLED or pn == 'data_test':
        flags.Trigger.EDMVersion=3
        flags.GeoModel.Run = LHCPeriod.Run3
        flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
    else:
        year = int(pn.split('_')[0].split('data')[1])
        if (year < 20):
            flags.Trigger.EDMVersion=2
            flags.GeoModel.Run = LHCPeriod.Run2
        elif (year > 20):
            flags.Trigger.EDMVersion=3
            flags.GeoModel.Run = LHCPeriod.Run3
            flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3

    if (flags.Input.isMC):
        print('ZdcRecConfig: Overriding MC run to be Run 3!')
        flags.GeoModel.Run = LHCPeriod.Run3

    if flags.Input.TriggerStream == "calibration_ZDCInjCalib" or flags.Input.TriggerStream == "calibration_DcmDummyProcessor":
        flags.Detector.EnableZDC_RPD = False # disable RPD for injector
    elif pn == "data22_13p6TeV":
        flags.Detector.EnableZDC_RPD = False # disable RPD for LHCf


    if flags.Input.TriggerStream == "calibration_DcmDummyProcessor": # standalone data: no trigger info available
        flags.DQ.useTrigger = False
        flags.DQ.triggerDataAvailable = False 

    flags.lock()
    # flags.dump(evaluate=True) # uncomment this line if needed for testing

    acc=MainServicesCfg(flags)

    from AtlasGeoModel.ForDetGeoModelConfig import ForDetGeometryCfg
    acc.merge(ForDetGeometryCfg(flags))

    if not flags.Input.isMC and pn != 'data_test': # trigger reco config not existing for MC or standalone
        from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfgData
        acc.merge(TriggerRecoCfgData(flags))

    if isLED:
        #acc.merge(ZdcLEDTrigCfg(flags))
        acc.merge(ZdcLEDRecCfg(flags))
    if isCalib: # should be able to run both if in standalone data
        acc.merge(ZdcRecCfg(flags))


    if not flags.Input.isMC:
        if (isLED):
            from ZdcMonitoring.ZdcLEDMonitorAlgorithm import ZdcLEDMonitoringConfig
            acc.merge(ZdcLEDMonitoringConfig(flags,'ppPbPb2023'))
            acc.merge(ZdcLEDNtupleCfg(flags))
        if (isCalib):
            from ZdcMonitoring.ZdcMonitorAlgorithm import ZdcMonitoringConfig
            
            zdcMonitorAcc = ZdcMonitoringConfig(flags)
            acc.merge(zdcMonitorAcc)
            # zdcMonitorAcc.getEventAlgo('ZdcMonAlg').OutputLevel = 2 # turn on DEBUG messages
            if (flags.Input.TriggerStream != "calibration_DcmDummyProcessor"): #after ntuple works for standalone data, take this line out
                acc.merge(ZdcNtupleLocalCfg(flags))
    else:
        acc.merge(ZdcNtupleLocalCfg(flags))

    acc.printConfig(withDetails=True)

    with open("config.pkl", "wb") as f:
        acc.store(f)
    status = acc.run()
    if status.isFailure():
        import sys
        sys.exit(-1)

