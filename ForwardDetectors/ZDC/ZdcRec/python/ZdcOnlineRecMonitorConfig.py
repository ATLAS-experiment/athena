# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

## CA-based configuration for Running the ZDC Athena Online Reconstruction and Monitoring
## imports detailed configurations from ZdcRecConfig
##      offline testing: python RecExOnline/RecExOnline_Partition_Online_ZDC.py --filesInput path/to/raw.data --evtMax 10
##      for offline testing on testbed / p1, open a new login session + make sure $TDAQ_PARTITION is NOT set
## Author: Yuhan Guo


# -------------------------------- IMPORT LIBRARIES --------------------------------
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AllConfigFlags import initConfigFlags

from ZdcRecConfig import ZdcGenericFlagSetting, ZdcStreamDependentFlagSetting

from ZdcFCalRecConfig import FCalRecCfg, ZdcFCalAdditionalFlagSetting

import os
import ispy

from AthenaCommon.Logging import logging
log = logging.getLogger("ZdcOnlineRecMonitorConfig")


# -------------------------------- PARTITION & ENVIRONMENT --------------------------------
def PartitionAndEnvironmentConfig():

    '''Set up the partition (object still exists but invalid if offline)
    and finds the current environment (testbed / p1)
    Returns:
        partition: the partition object
        isTestbed - boolean that indicates whether it's testbed or p1'''

    # -------------------------------- PARTITION --------------------------------

    # if environment variable TDAQ_PARTITION doesn't exist --> partition object exists but partition.isValid() is false
    partition = ispy.IPCPartition("") if os.getenv("TDAQ_PARTITION") is None else ispy.IPCPartition(os.getenv("TDAQ_PARTITION"))

    # -------------------------------- Environment (Testbed / Point1) --------------------------------

    environmentString = os.getenv("ENVIORNMENT")  # ENVIORNMENT defined after one sources setup_partition.sh

    if partition.isValid() and environmentString is None:
         log.warning("Warning: Partition is valid but environmental variable ENVIORNMENT is not set!")
         log.warning("Warning: In this case by default assumes environment is p1!")
    
    isTestbed = (environmentString == "TB")

    if partition.isValid():
        log.info("Running Online with Partition: %s",partition.name())
    else:
        log.info("Partition %s not found. Running Offline - must provide input files!", partition.name())

    return partition, isTestbed

# -------------------------------- CONFIGURATION FLAGS SETTING --------------------------------
def ZdcOnlineConfigFlagsSetting(flags, partition):
    '''Set additional configuration flags for online environment'''
    
    log.debug ('Setting additional flags for online environment')

    flags.Concurrency.NumThreads = 1 
    flags.Common.isOnline = True
    flags.DQ.Environment = 'online'
    flags.DQ.enableLumiAccess = False
    flags.Common.useOnlineLumi = True
    flags.DQ.doStreamAwareMon = False
    flags.DQ.FileKey = ""
    flags.LAr.doHVCorr = False
    flags.Trigger.triggerConfig = 'DB'

    flags.Output.doWriteESD = False
    flags.Output.doWriteAOD = False

    if (partition.isValid() and partition.name() != 'ATLAS'): # running on testbed or at p1 in standalone partition: no trigger info
        flags.DQ.useTrigger = False
        flags.DQ.triggerDataAvailable = False  

    # ------------------------------- turn off steering flags for online environment -------------------------------
    _steeringFlags = ['HLT.doBjet', 'HLT.doBphys', 'HLT.doCalo', 'HLT.doEgamma', 'HLT.doGeneral', 'HLT.doInDet', 'HLT.doJet', 'HLT.doMET', 'HLT.doMinBias', 'HLT.doMuon', 'HLT.doTau', 'InDet.doAlignMon', 'InDet.doGlobalMon', 'InDet.doPerfMon', 'LVL1Calo.doValidatio', 'Muon.doAlignMon', 'Muon.doCombinedMon', 'Muon.doPhysicsMon', 'Muon.doRawMon', 'Muon.doSegmentMon', 'Muon.doTrackMon', 'Muon.doTrkPhysMon', 'doAFPMon', 'doCTPMon', 'doCaloGlobalMon', 'doDataFlowMon', 'doEgammaMon', 'doGlobalMon', 'doHIMon', 'doHLTMon', 'doInDetMon', 'doJetInputsMon', 'doJetMon', 'doJetTagMon', 'doLArMon', 'doLVL1CaloMon', 'doLVL1InterfacesMon', 'doLucidMon', 'doMissingEtMon', 'doMuonMon', 'doPixelMon', 'doSCTMon', 'doTRTMon', 'doTauMon', 'doTileMon']

    for flag in _steeringFlags:
        if flags.hasFlag('DQ.Steering.' + flag):
            flags._set('DQ.Steering.' + flag, False)
        else:
            flags.addFlag('DQ.Steering.' + flag, False)

    # ------------------------------- turn off trigger flags for online environment -------------------------------
    _triggerFlags = ['CostMonitoring.doCostMonitoring', 'CostMonitoring.monitorROBs', 'DecisionMakerValidation.Execute', 'Jet.fastbtagPFlow', 'Jet.fastbtagVertex', 'decodeHLT', 'enableL1CaloPhase1', 'enableL1MuonPhase1', 'L1.doMuon', 'L1.doCalo', 'L1.doTopo', 'L1.doCTP', 'L1MuonSim.NSWVetoMode', 'L1MuonSim.doBIS78', 'L1MuonSim.doMMTrigger', 'L1MuonSim.doPadTrigger', 'doLVL1', 'doHLT', 'doCalo', 'doID', 'doMuon', 'doNavigationSlimming', 'enableL1CaloLegacy', 'endOfEventProcessing.Enabled', 'fastMenuGeneration', 'Online.BFieldAutoConfig']

    for flag in _triggerFlags:
        if flags.hasFlag('Trigger.' + flag):
            flags._set('Trigger.' + flag, False)
        else:
            flags.addFlag('Trigger.' + flag, False)


# -------------------------------- Online project name manual setting for testbed --------------------------------
def ZdcOnlineProjectNameManualSetting(flags, isTestbed):
    '''If running on testbed, manually set Input.ProjectName flag from the OKS variable ZDC_PROJECT_NAME processed as an environmental variable
    If running at P1, check for project name and set to default if not properly set
    Necessary since ZdcStreamDependentFlagSetting will throw ValueError is ProjectName is not set'''
    
    if isTestbed: # testbed: set project name from OKS variable
        if os.getenv("ZDC_PROJECT_NAME") is None:
            log.warning("Running on testbed, yet ZDC_PROJECT_NAME is NOT set!")
            log.warning("Setting to be data_test by default.")
            flags.Input.ProjectName = 'data_test'
        else:
            flags.Input.ProjectName = os.getenv("ZDC_PROJECT_NAME")
    elif partition.isValid and not flags.Input.ProjectName: # project name is None or empty string
        if partition.name() == 'ATLAS':
            log.warning("Running in ATLAS partition, but ProjectName is NOT correctly set!")
            log.warning("Setting to be data24_hi by default. Could cause issues.")
            flags.Input.ProjectName = 'data24_hi'
        else: # running at P1 in standalone partition
            flags.Input.ProjectName = 'data_test'


# -------------------------------- Online trigger-stream flag manual setting --------------------------------
def ZdcOnlineTriggerStreamManualSetting(flags, partition, isTestbed):
    '''manually set Input.TriggerStream flag from the OKS variable ZDC_STREAM_NAME processed as an environmental variable
    Should only be called in the online environment (do NOT overwrite the TriggerStream info from offline metadata)'''

    # Remark: autoConfigOnlineRecoFlags only fills run-parameter-dependent flags, not stream-specific ones --> manual setting needed
    # Remark: stream and projectname-dependent flag settings, such as Detector.EnableZDC_RPD, are set in the function ZdcStreamDependentFlagSetting
    # This function and ZdcOnlineProjectNameManualSetting make sure projectname and triggerstream are correctly set
    # And must be called prior to calling ZdcStreamDependentFlagSetting

    # P1 and standalone partition
    if (not isTestbed and partition.name() != 'ATLAS'):
        flags.Input.TriggerStream = "calibration_DcmDummyProcessor"
    else: 
        # either running in ATLAS - triggerstream tag not automatically set, but trigger info is available --> set tag manually
        # or running on testbed - trigger info is NOT available, but triggerstream is needed to correctly configure reconstruction
        if os.getenv("ZDC_STREAM_NAME") is None:
            log.warning("Running on testbed or on p1 in ATLAS partition, yet ZDC_STREAM_NAME is NOT set!")
            log.warning("Assuming stream to be ZdcCalib by default! Likely to cause issues.")
            flags.Input.TriggerStream = "calibration_ZDCCalib"
        elif os.getenv("ZDC_STREAM_NAME") == "ZDCCalib":
            flags.Input.TriggerStream = "calibration_ZDCCalib"
        elif os.getenv("ZDC_STREAM_NAME") == "ZDCLEDCalib":
            flags.Input.TriggerStream = "calibration_ZDCLEDCalib"
        elif os.getenv("ZDC_STREAM_NAME") == "ZDCInjCalib":
            flags.Input.TriggerStream = "calibration_ZDCInjCalib"
        elif os.getenv("ZDC_STREAM_NAME") == "MinBias":
            flags.Input.TriggerStream = "physics_MinBias"
        elif os.getenv("ZDC_STREAM_NAME") == "UCC":
            flags.Input.TriggerStream = "physics_UCC"
        elif os.getenv("ZDC_STREAM_NAME") == "express":
            flags.Input.TriggerStream = "express_express"

    
# -------------------------------- OUTPUTTING DEGUG MESSAGES --------------------------------
def ZdcOnlinePrintDebugMsgs():
    '''Prints debug messages (for now, always on)'''

    # Check environmental variables
    log.debug ('check if the os environment configs are correctly set')
    log.debug ('ZDC_RELEASE_NAME %s', os.getenv("ZDC_RELEASE"))
    log.debug ('ENVIORNMENT %s', os.getenv("ENVIORNMENT"))
    log.debug ('ZDC_KEY_COUNT %s', os.getenv("ZDC_KEY_COUNT"))
    log.debug ('ZDC_KEY %s', os.getenv("ZDC_KEY"))
    log.debug ('ZDC_ATHENA_JOB_NAME %s', os.getenv("ZDC_ATHENA_JOB_NAME"))
    log.debug ('ZDC_STREAM_NAME %s', os.getenv("ZDC_STREAM_NAME"))
    log.debug ('ZDC_STREAM_TYPE %s', os.getenv("ZDC_STREAM_TYPE"))


# -------------------------------- BYTE STREAM EMON INPUT SERVICE --------------------------------

def ZdcOnlineByteStreamCfg(flags, partition, isTestbed):
    '''Configure byte-stream input service using environmental (OKS) variables'''

    acc = ComponentAccumulator()

    bytestreamConversion = CompFactory.ByteStreamCnvSvc()
    acc.addService(bytestreamConversion, primary=True)

    from ByteStreamEmonSvc.EmonByteStreamConfig import EmonByteStreamCfg
    acc.merge(EmonByteStreamCfg(flags)) # setup EmonSvc

    bsSvc = acc.getService("ByteStreamInputSvc")
    bsSvc.Partition = partition.name()

    if isTestbed:
        bsSvc.Key = "ReadoutApplication"
    else:
        bsSvc.Key = os.environ.get("ZDC_KEY", "dcm")
        log.debug('the value being assigned to bssvc key is %s', os.environ.get("ZDC_KEY", "dcm"))
    
    log.info('final bssvc key: %s', bsSvc.Key)
    bsSvc.KeyCount = int(os.environ.get("ZDC_KEY_COUNT","250"))
    log.info('final bssvc keycount: %s', bsSvc.KeyCount)
    bsSvc.ISServer = "Histogramming" # IS server on which to create this provider
    bsSvc.BufferSize = 10 # event buffer size for each sampler
    bsSvc.UpdatePeriod = 30 # time in seconds between updating plots
    bsSvc.Timeout = 240000 # timeout (not sure what this does)
    bsSvc.PublishName = os.getenv("ZDC_ATHENA_JOB_NAME","ZDC_Athena_monitor_test") # set name of this publisher as it will appear in IS
    bsSvc.ExitOnPartitionShutdown = False
    bsSvc.ClearHistograms = True # clear hists at start of new run
    bsSvc.GroupName = "RecExOnline"
    # stream specifies
    bsSvc.StreamType = os.getenv("ZDC_STREAM_TYPE","physics") if isTestbed or partition.name() == "ATLAS" else "calibration" # if on testbed or at p1 in ATLAS partition: set the stream type from environmental (OKS) variables
    bsSvc.StreamNames = os.getenv("ZDC_STREAM_NAME","ZDCCalib:ZDCLEDCalib:MinBias").split(":") if isTestbed or partition.name() == "ATLAS" else "ZDCLEDCalib".split(":") # name of the stream (Egamma,JetTauEtmiss,MinBias,Standby, etc.), this can be a colon(:) separated list of streams that use the 'streamLogic' to combine stream for 2016 HI run
    bsSvc.StreamLogic = os.getenv("ZDC_STREAM_LOGIC","Or") if partition.name() == "ATLAS" else "Ignore"

    log.debug('Printing out for debugging at testing/developing stage')
    log.debug('Testing if settings of these variables in ZDC athena segment OKS are correctly picked up by the python code')
    log.debug('the stream type is: %s', bsSvc.StreamType)
    log.debug('the stream names are: %s', bsSvc.StreamNames)
    log.debug('the stream logic is: %s', bsSvc.StreamLogic)

    return acc


def ZdcOnlineRecoFlagSettings(flags):
    ZdcGenericFlagSetting(flags)

    # exit if running in offline mode and no input file is provided
    if not partition.isValid() and len(flags.Input.Files)==0:
        log.fatal("FATAL: Running in offline mode but no input files provided")
        import sys
        sys.exit(1)

    if partition.isValid(): # online-specific config flag settings
        # online auto config flag settings
        from AthenaConfiguration.AutoConfigOnlineRecoFlags import autoConfigOnlineRecoFlags
        autoConfigOnlineRecoFlags(flags, partition.name()) # sets things like projectName etc which would otherwise be inferred from input file
        log.info('the auto-configured globaltag is: %s', flags.IOVDb.GlobalTag)

        # additional online config flag settings
        ZdcOnlineConfigFlagsSetting(flags, partition)
    else: # offline
        flags.Output.AODFileName="AOD.pool.root"
        flags.Output.HISTFileName="HIST.root"
        flags.Output.doWriteAOD=True

    # Manually set the Input.TriggerStream flag based on the environmental variable ZDC_STREAM_NAME
    # Must preceed calling ZdcFCalAdditionalFlagSetting and ZdcStreamDependentFlagSetting
    if partition.isValid():
        ZdcOnlineProjectNameManualSetting(flags, isTestbed)
        ZdcOnlineTriggerStreamManualSetting(flags, partition, isTestbed)
    
    ZdcFCalAdditionalFlagSetting(flags)


    # stream-dependent flag setting
    isLED, isInj, isCalib, pn = ZdcStreamDependentFlagSetting(flags)

    return isLED, isInj, isCalib, pn


def RunZdcOnlineRecoCfg(flags, isLED, isInj, isCalib):
    acc = ComponentAccumulator()

    if isLED:
        from ZdcRec.ZdcRecConfig import ZdcLEDRecCfg
        ZdcLEDRecAcc = ZdcLEDRecCfg(flags)
        acc.merge(ZdcLEDRecAcc)
        daqMode = 1 if partition.name() == 'zdcStandalone' else 2
        ZdcLEDRecAcc.getEventAlgo('ZdcRecRun3').DAQMode = daqMode
        log.info ('CHECK: The DAQ mode for the LED reconstruction is %s', ZdcLEDRecAcc.getEventAlgo('ZdcRecRun3').DAQMode)
    if isCalib or isInj: # should be able to run both if in standalone data
        from ZdcRec.ZdcRecConfig import ZdcRecCfg
        ZdcRecAcc = ZdcRecCfg(flags)
        acc.merge(ZdcRecAcc)

    return acc

def RunZdcOnlineMonitorCfg(flags, isLED, isInj, isCalib):
    acc = ComponentAccumulator()

    if not flags.Input.isMC:
        if (isLED):
            from ZdcMonitoring.ZdcLEDMonitorAlgorithm import ZdcLEDMonitoringConfig
            zdcLEDMonitorAcc = ZdcLEDMonitoringConfig(flags,'ppPbPb2023')
            acc.merge(zdcLEDMonitorAcc)
            
        if (isCalib or isInj):
            from ZdcMonitoring.ZdcMonitorAlgorithm import ZdcMonitoringConfig
            zdcMonitorAcc = ZdcMonitoringConfig(flags)
            acc.merge(zdcMonitorAcc)

    return acc

if __name__ == '__main__':

    # boolean that indicates if debug mode is on (False if environmental variable BOOL_DEBUG_MODE not set)
    debugModeOn = (os.getenv("BOOL_DEBUG_MODE") == "True") 
    from AthenaCommon.Constants import DEBUG, INFO
    if debugModeOn:
        log.setLevel(DEBUG)  # Set to DEBUG to see all messages
    else:
        log.setLevel(INFO)  # Only print level info/above messages

    partition, isTestbed = PartitionAndEnvironmentConfig()

    # if not partition.isValid(): #uncomment the followig lines to turn on DEBUG messages for offline environment
    #     log.setLevel(DEBUG)
    #     debugModeOn = True

    if partition.isValid():
        ZdcOnlinePrintDebugMsgs()

    # -------------------------------- Configuration flag settings --------------------------------

    flags = initConfigFlags()

    isLED, isInj, isCalib, pn = ZdcOnlineRecoFlagSettings(flags)

    flags.lock()
    flags.dump(evaluate=True) # testing stage - always dump: make sure settings are correct + geometry/steering/triggers/reconstruction/... of all other sub-detectors are turned off

    # -------------------------------- Configuring & Merging Byte Stream emon service --------------------------------

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    # configuration byte stream emon service
    if partition.isValid():
        acc.merge(ZdcOnlineByteStreamCfg(flags, partition, isTestbed))
    else:
        log.info("Running Offline on %d files", len(flags.Input.Files))


    # -------------------------------- Adding Decoding, Reconstruction & Monitoring algorithms --------------------------------
    # assuming isLED & isCalib are set correctly for both online & offline environment
    # (online via configuration settings of isTestbed & stream type/name in OKS that this python scripts reads as environment input)
    # add the algorithms as in ZdcRecConfig

    from GaudiSvc.GaudiSvcConf import THistSvc
    THistSvc.OutputLevel = 5 #ERROR (we get a list of unnecessary warnings - turn the outputs off)

    from AtlasGeoModel.ForDetGeoModelConfig import ForDetGeometryCfg
    acc.merge(ForDetGeometryCfg(flags))

    if flags.DQ.useTrigger: # unlike in offline config files, for turning on trigger reco, we rely on DQ.useTrigger to be properly set based on partition + environment
                            # to avoid (error-prone) duplication of decision
        from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfgData
        acc.merge(TriggerRecoCfgData(flags))

    acc.merge(RunZdcOnlineRecoCfg(flags, isLED, isInj, isCalib))

    if (flags.Input.TriggerStream == "physics_MinBias" or flags.Input.TriggerStream == "express_express" or flags.Input.TriggerStream == "physics_UCC"):
        acc.merge(FCalRecCfg(flags))

    acc.merge(RunZdcOnlineMonitorCfg(flags, isLED, isInj, isCalib))

    acc.printConfig(withDetails=True)

    if debugModeOn:
        acc.foreach_component("*Zdc*").OutputLevel=DEBUG
        acc.foreach_component("*ZDC*").OutputLevel=DEBUG

    log.info("Configured Services: %s", ", ".join(svc.name for svc in acc.getServices()))
    log.info("Configured EventAlgos: %s", ", ".join(alg.name for alg in acc.getEventAlgos()))
    log.info("Configured CondAlgos: %s", ", ".join(alg.name for alg in acc.getCondAlgos()))

    status = acc.run()
    if status.isFailure():
        import sys
        sys.exit(-1)

