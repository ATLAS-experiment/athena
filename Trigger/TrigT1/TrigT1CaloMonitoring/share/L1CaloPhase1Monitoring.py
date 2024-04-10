# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## Script for Running the L1Calo Athena Simulation and/or Monitoring for Phase1
## can be run offline on raw or POOL files (for rerunning simulation)
## run through athena
##    offline: athena --CA TrigT1CaloPhase1Monitoring/L1CaloPhase1Monitoring.py --filesInput path/to/raw.data --evtMax 10
##    online:  athena --CA TrigT1CaloPhase1Monitoring/L1CaloPhase1Monitoring.py
## Author: Will Buttinger

from AthenaCommon.Configurable import ConfigurableCABehavior
from AthenaConfiguration.ComponentAccumulator import appendCAtoAthena
from AthenaConfiguration.ComponentFactory import CompFactory,isComponentAccumulatorCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.Enums import LHCPeriod,Format
from AthenaCommon import Constants
import os,sys
import ispy
partition = ispy.IPCPartition(os.getenv("TDAQ_PARTITION","ATLAS"))

flags = initConfigFlags()
flags.Input.Files = [] # so that when no files given we can detect that

# Note: The order in which all these flag defaults get set is very fragile
# so don't reorder the setup of this flags stuff


flags.Exec.OutputLevel = Constants.WARNING # by default make everything output at WARNING level
flags.Exec.InfoMessageComponents = ["AthenaEventLoopMgr","THistSvc","PerfMonMTSvc","ApplicationMgr"] # Re-enable some info messaging though
flags.Exec.PrintAlgsSequence = True # print the alg sequence at the start of the job (helpful to see what is scheduled)
# flags.Exec.FPE = -2 # disable FPE auditing ... set to 0 to re-enable

## Modify default flags
flags.GeoModel.Run = LHCPeriod.Run3 # needed for LArGMConfig - or can infer from above
flags.Common.useOnlineLumi = True # needed for lumi-scaled monitoring, only have lumi in online DB at this time
flags.DQ.doMonitoring = True      # use this flag to turn on/off monitoring in this application
flags.DQ.enableLumiAccess = False # in fact, we don't need lumi access for now ... this turns it all off
flags.DQ.FileKey = "" if partition.isValid() else "EXPERT" # histsvc file "name" to record to - Rafal asked it to be blank @ P1 ... means monitoring.root will be empty
flags.Output.HISTFileName = os.getenv("L1CALO_ATHENA_JOB_NAME","") + "monitoring.root" # control names of monitoring root file - ensure each online monitoring job gets a different filename to avoid collision between processes
flags.DQ.useTrigger = False # don't do TrigDecisionTool in MonitorCfg helper methods
# flag for saying if inputs should be decoded or not
flags.Trigger.L1.doCaloInputs = True
# flags for rerunning simulation
flags.Trigger.L1.doeFex = True
flags.Trigger.L1.dojFex = True
flags.Trigger.L1.dogFex = True
# if running online, override these with autoconfig values
# will set things like the GlobalTag automatically
if partition.isValid():
  # must ensure doLVL1 and doHLT are False, otherwise will get ByteStreamCnvSvc conflicts (TrigByteStreamCnvSvc is setup, but EMon setup provides ByteStreamCnvSvc)
  # see TriggerByteStreamConfig.py
  flags.Trigger.doLVL1 = False
  flags.Trigger.doHLT = False
  from AthenaConfiguration.AutoConfigOnlineRecoFlags import autoConfigOnlineRecoFlags
  autoConfigOnlineRecoFlags(flags, partition.name()) # sets things like projectName etc which would otherwise be inferred from input file
else:
  flags.Trigger.doLVL1 = True # set this just so that IOBDb.GlobalTag is autoconfigured based on release setup if running on RAW (autoconfig will take it from POOL file if running on that)
#flags.IOVDb.GlobalTag = lambda s: "OFLCOND-MC23-SDR-RUN3-02" if s.Input.isMC else "CONDBR2-ES1PA-2022-07" #"CONDBR2-HLTP-2022-02"

# now parse

parser = flags.getArgumentParser()
parser.add_argument('--runNumber',default=None,help="specify to select a run number")
parser.add_argument('--lumiBlock',default=None,help="specify to select a lumiBlock")
parser.add_argument('--evtNumber',default=None,nargs="+",type=int,help="specify to select an evtNumber")
parser.add_argument('--stream',default="physics_L1Calo",help="stream to lookup files in")
parser.add_argument('--fexReadoutFilter',action='store_true',help="If specified, will skip events without fexReadout")
parser.add_argument('--dbOverrides',default=None,nargs="+",type=str,help="specify overrides of COOL database folders in form <folder>=<dbPath>, example: /TRIGGER/L1Calo/V1/Calibration/EfexEnergyCalib=mytest.db ")
parser.add_argument('--postConfig',default=[],nargs="+",type=str,help="specify component properties to apply at the end of the config")
args = flags.fillFromArgs(parser=parser)
if args.runNumber is not None:
  from glob import glob
  if args.lumiBlock is None: args.lumiBlock="*"
  print("Looking up files in atlastier0 for run",args.runNumber,"lb =",args.lumiBlock)
  flags.Input.Files = []
  for lb in args.lumiBlock.split(","):
    if lb=="*":
      tryStr = f"/eos/atlas/atlastier0/rucio/data*/{args.stream}/*{args.runNumber}/*RAW/*lb*.*"
    else:
      tryStr = f"/eos/atlas/atlastier0/rucio/data*/{args.stream}/*{args.runNumber}/*RAW/*lb{int(lb):04}.*"
    print("Trying",tryStr)
    flags.Input.Files += glob(tryStr)
  print("Found",len(flags.Input.Files),"files")

# require at least 1 input file if running offline
if not partition.isValid() and len(flags.Input.Files)==0:
  print("FATAL: Running in offline mode but no input files provided")
  sys.exit(1)
elif partition.isValid():
  print("Running Online with Partition:",partition.name())

# if running on an input file, change the DQ environment, which will allow debug tree creation from monitoring algs
if len(flags.Input.Files)>0:
  flags.DQ.Environment = "user"
  # triggerConfig should default to DB which is appropriate if running on data
  if flags.Input.isMC: flags.Trigger.triggerConfig='FILE' # uses the generated L1Menu (see below)

# due to https://gitlab.cern.ch/atlas/athena/-/merge_requests/65253 must now specify geomodel explicitly if cant take from input file, but can autoconfigure it based on LHCPeriod set above
if flags.GeoModel.AtlasVersion is None:
  from AthenaConfiguration.TestDefaults import defaultGeometryTags
  flags.GeoModel.AtlasVersion = defaultGeometryTags.autoconfigure(flags)

# add detector conditions flags required for rerunning simulation
# needs input files declared if offline, hence doing after parsing
from AthenaConfiguration.DetectorConfigFlags import setupDetectorsFromList
setupDetectorsFromList(flags,['LAr','Tile','MBTS'],True)

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
cfg = MainServicesCfg(flags)

with ConfigurableCABehavior(): # need to temporarily activate run3 behaviour to use this configuration method

  flags.lock()
  if flags.Exec.MaxEvents == 0: flags.dump(evaluate=True)

  if partition.isValid() and len(flags.Input.Files)==0:
    from ByteStreamEmonSvc.EmonByteStreamConfig import EmonByteStreamCfg
    cfg.merge(EmonByteStreamCfg(flags)) # setup EmonSvc
    bsSvc = cfg.getService("ByteStreamInputSvc")
    bsSvc.Partition = partition.name()
    bsSvc.Key = os.environ.get("L1CALO_PTIO_KEY", "REB" if partition.name()=="L1CaloStandalone" else "dcm") # set the Sampler Key Type name (default is SFI)
    if partition.name()=="L1CaloSTF": bsSvc.Key = "SWROD"
    bsSvc.KeyCount = int(os.environ.get("L1CALO_PTIO_KEY_COUNT","25"))
    bsSvc.ISServer = "Histogramming" # IS server on which to create this provider
    bsSvc.BufferSize = 10 # event buffer size for each sampler
    bsSvc.UpdatePeriod = 30 # time in seconds between updating plots
    bsSvc.Timeout = 240000 # timeout (not sure what this does)
    bsSvc.PublishName = os.getenv("L1CALO_ATHENA_JOB_NAME","testing") # set name of this publisher as it will appear in IS (default is "l1calo-athenaHLT"; change to something sensible for testing)
    bsSvc.StreamType = os.getenv("L1CALO_PTIO_STREAM_TYPE","physics") # name of the stream type (physics,express, etc.)
    bsSvc.ExitOnPartitionShutdown = False
    bsSvc.ClearHistograms = True # clear hists at start of new run
    bsSvc.GroupName = "RecExOnline"
    # name of the stream (Egamma,JetTauEtmiss,MinBias,Standby, etc.), this can be a colon(:) separated list of streams that use the 'streamLogic' to combine stream for 2016 HI run
    bsSvc.StreamNames = os.getenv("L1CALO_PTIO_STREAM_NAME","L1Calo:Main:MinBias:MinBiasOverlay:UPC:EnhancedBias:ZeroBias:HardProbes:Standby:ALFACalib").split(":")
    bsSvc.StreamLogic = os.getenv("L1CALO_PTIO_STREAM_LOGIC","Or") if partition.name() != "L1CaloStandalone" else "Ignore"
    bsSvc.LVL1Names = [] # name of L1 items to select
    bsSvc.LVL1Logic = "Ignore" # one of: Ignore, Or, And
  elif flags.Input.Format == Format.POOL:
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))
  else:
    print("Running Offline on", len(flags.Input.Files)," bytestream files")
    #from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    #TODO: Figure out why the above line causes CA conflict @ P1 if try to run on a RAW file there
    from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
    cfg.merge(ByteStreamReadCfg(flags)) # configure reading bytestream

  # ensure histsvc is set up
  from AthenaMonitoring.AthMonitorCfgHelper import getDQTHistSvc
  cfg.merge(getDQTHistSvc(flags))

  # Create run3 L1 menu (needed for L1Calo EDMs)
  from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg,generateL1Menu, createL1PrescalesFileFromMenu,getL1MenuFileName
  if flags.Input.isMC:
    # for MC we set the TriggerConfig to "FILE" above, so must generate a menu for it to load (will be the release's menu)
    generateL1Menu(flags)
    createL1PrescalesFileFromMenu(flags)
  if flags.Trigger.triggerConfig=="FILE":
    menuFilename = getL1MenuFileName(flags)
    if os.path.exists(menuFilename):
      print("Using L1Menu:",menuFilename)
    else:
      print("L1Menu file does not exist:",menuFilename)
      sys.exit(1)
  cfg.merge(L1ConfigSvcCfg(flags))

  # -------- CHANGES GO BELOW ------------
  # setup the L1Calo software we want to monitor

  decoderTools = []

  if partition.isValid() or (flags.Input.Format != Format.POOL and not flags.Input.isMC):
    from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import eFexByteStreamToolCfg, jFexRoiByteStreamToolCfg, jFexInputByteStreamToolCfg, gFexByteStreamToolCfg, gFexInputByteStreamToolCfg
    if flags.Trigger.L1.doeFex: decoderTools += [cfg.popToolsAndMerge(eFexByteStreamToolCfg(flags=flags,name='eFexBSDecoderTool',TOBs=flags.Trigger.L1.doeFex,xTOBs=flags.Trigger.L1.doeFex,decodeInputs=flags.Trigger.L1.doCaloInputs,multiSlice=True))]
    if flags.Trigger.L1.dojFex: decoderTools += [cfg.popToolsAndMerge(jFexRoiByteStreamToolCfg(flags=flags,name="jFexBSDecoderTool",writeBS=False))]
    if flags.Trigger.L1.dogFex: decoderTools += [cfg.popToolsAndMerge(gFexByteStreamToolCfg(flags=flags,name="gFexBSDecoderTool",writeBS=False))]



    if flags.Trigger.L1.doCaloInputs:
      if flags.Trigger.L1.dojFex: decoderTools += [cfg.popToolsAndMerge(jFexInputByteStreamToolCfg(flags=flags,name='jFexInputBSDecoderTool',writeBS=False))]
      if flags.Trigger.L1.dogFex: decoderTools += [cfg.popToolsAndMerge(gFexInputByteStreamToolCfg(flags=flags,name='gFexInputBSDecoderTool',writeBS=False))]

    if len(decoderTools) > 0:
      from TrigT1ResultByteStream.TrigT1ResultByteStreamMonitoringConfig import L1TriggerByteStreamDecoderMonitoringCfg
      cfg.addEventAlgo(CompFactory.L1TriggerByteStreamDecoderAlg(
          name="L1TriggerByteStreamDecoder",
          OutputLevel=Constants.ERROR, # hides warnings about non-zero status codes in fragments ... will show up in hists
          DecoderTools=decoderTools,
          ByteStreamMetadataRHKey = '', # seems necessary @ P1 if trying to run on a raw file
          MaybeMissingROBs= [id for tool in decoderTools for id in tool.ROBIDs ] if partition.name()!="ATLAS" or not partition.isValid() else [], # allow missing ROBs away from online ATLAS partition
          MonTool= cfg.popToolsAndMerge(L1TriggerByteStreamDecoderMonitoringCfg(flags,"L1TriggerByteStreamDecoder", decoderTools))
        ),sequenceName='AthAlgSeq'
      )

  #from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
  #cfg.merge( LArOnOffIdMappingSCCfg(flags) )
  # from LArGeoAlgsNV.LArGMConfig import LArGMCfg
  # cfg.merge(LArGMCfg(flags))

  # rerun sim if required
  if flags.Trigger.L1.doeFex or flags.Trigger.L1.dojFex or flags.Trigger.L1.dogFex or flags.Trigger.L1.doCaloInputs:
    from L1CaloFEXSim.L1CaloFEXSimCfg import L1CaloFEXSimCfg
    cfg.merge(L1CaloFEXSimCfg(flags,outputSuffix="DAODSim" if flags.Input.Format == Format.POOL else ""))

    # do otf masking:
    # from IOVDbSvc.IOVDbSvcConfig import addFolders,addOverride
    # #cfg.merge(addFolders(flags,"<db>sqlite://;schema=/afs/cern.ch/user/w/will/new_maskedSCs_run457976.db;dbname=CONDBR2</db> /LAR/BadChannels/NoisyChannelsSC",className="CondAttrListCollection")) # dmCorr from DB!
    # cfg.merge(addFolders(flags,"/LAR/BadChannels/MaskedSC","LAR_ONL",tag="LARBadChannelsMaskedSC-RUN3-UPD1-00",className="CondAttrListCollection",extensible=False)) # when run online, need folder to be extensible to force reload each event
    # cfg.addCondAlgo(CompFactory.LArBadChannelCondAlg(name="MaskedSCCondAlg",ReadKey="/LAR/BadChannels/MaskedSC",isSC=True,CablingKey="LArOnOffIdMapSC",WriteKey="LArMaskedSC"))
    # # note to self, if need to flag extensible after loaded elsewhere, look at property: cfg.getService("IOVDbSvc").Folders ... extend relevant entry with "<extensible/>"
    # print(cfg.getService("MessageSvc"))
    # cfg.getService("MessageSvc").errorLimit = 0
    #
    # cfg.getEventAlgo("L1_eFexEmulatedTowers").LArBadChannelKey = "LArMaskedSC"



  if flags.DQ.doMonitoring:
    if flags.Trigger.L1.doeFex:
      from TrigT1CaloMonitoring.EfexMonitorAlgorithm import EfexMonitoringConfig
      cfg.merge(EfexMonitoringConfig(flags))
      EfexMonAlg = cfg.getEventAlgo('EfexMonAlg')
      # do we need next lines??
      EfexMonAlg.eFexEMTobKeyList = ['L1_eEMRoI', 'L1_eEMxRoI'] # default is just L1_eEMRoI
      EfexMonAlg.eFexTauTobKeyList = ['L1_eTauRoI', 'L1_eTauxRoI']
      #  Adjust eFEX containers to be monitored to also monitor the sim RoI
      for l in [EfexMonAlg.eFexEMTobKeyList,EfexMonAlg.eFexTauTobKeyList]: l += [x + "Sim" for x in l ]
      # monitoring of simulation vs hardware
      if not flags.Input.isMC:
        from TrigT1CaloMonitoring.EfexSimMonitorAlgorithm import EfexSimMonitoringConfig
        cfg.merge(EfexSimMonitoringConfig(flags))
      # EfexSimMonitorAlgorithm = cfg.getEventAlgo('EfexSimMonAlg')
      # and now book the histograms that depend on the containers
      from TrigT1CaloMonitoring.EfexMonitorAlgorithm import EfexMonitoringHistConfig
      cfg.merge(EfexMonitoringHistConfig(flags,EfexMonAlg))

    if flags.Trigger.L1.dojFex:
      from TrigT1CaloMonitoring.JfexMonitorAlgorithm import JfexMonitoringConfig
      cfg.merge(JfexMonitoringConfig(flags))
      from TrigT1CaloMonitoring.JfexSimMonitorAlgorithm import JfexSimMonitoringConfig
      cfg.merge(JfexSimMonitoringConfig(flags))
    if flags.Trigger.L1.dogFex:
      from TrigT1CaloMonitoring.GfexMonitorAlgorithm import GfexMonitoringConfig
      cfg.merge(GfexMonitoringConfig(flags))
      from TrigT1CaloMonitoring.GfexSimMonitorAlgorithm import GfexSimMonitoringConfig
      cfg.merge(GfexSimMonitoringConfig(flags))
      # can't include efficiency monitoring because requires too many things we don't have
      # from TrigT1CaloMonitoring.JetEfficiencyMonitorAlgorithm import JetEfficiencyMonitoringConfig
      # cfg.merge(JetEfficiencyMonitoringConfig(flags))

    # input data monitoring
    if flags.Trigger.L1.doCaloInputs and not flags.Input.isMC:
      from TrigT1CaloMonitoring.EfexInputMonitorAlgorithm import EfexInputMonitoringConfig
      if flags.Trigger.L1.doeFex: cfg.merge(EfexInputMonitoringConfig(flags))
      from TrigT1CaloMonitoring.JfexInputMonitorAlgorithm import JfexInputMonitoringConfig
      if flags.Trigger.L1.dojFex: cfg.merge(JfexInputMonitoringConfig(flags))
      from TrigT1CaloMonitoring.GfexInputMonitorAlgorithm import GfexInputMonitoringConfig
      if flags.Trigger.L1.dogFex: cfg.merge(GfexInputMonitoringConfig(flags))

  mainSeq = "AthAllAlgSeq"
  if args.fexReadoutFilter:
    # want to take existing AthAllSeqSeq and move it inside a new sequence
    topSeq = cfg.getSequence("AthAlgEvtSeq")
    algSeq = cfg.getSequence(mainSeq)
    mainSeq = "New" + mainSeq
    # topSeq has three sub-sequencers ... preserve first and last
    topSeq.Members = [topSeq.Members[0],CompFactory.AthSequencer(mainSeq),topSeq.Members[-1]]
    cfg.addEventAlgo(CompFactory.L1IDFilterAlgorithm(),sequenceName=mainSeq)
    cfg.getSequence(mainSeq).Members += [algSeq]

  if args.evtNumber is not None:
    print("filtering events",args.evtNumber)
    # similar adjustment with an event filter
    topSeq = cfg.getSequence("AthAlgEvtSeq")
    algSeq = cfg.getSequence(mainSeq)
    mainSeq = "New" + mainSeq
    # topSeq has three sub-sequencers ... preserve first and last
    topSeq.Members = [topSeq.Members[0],CompFactory.AthSequencer(mainSeq),topSeq.Members[-1]]
    cfg.addEventAlgo(CompFactory.EventNumberFilterAlgorithm("EvtNumberFilter",EventNumbers=args.evtNumber),sequenceName=mainSeq)
    cfg.getSequence(mainSeq).Members += [algSeq]
    # cfg.addEventAlgo(CompFactory.LVL1.eFexEventDumper("Dumper",TowersKey="L1_eFexEmulatedTowers"))

  from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
  cfg.merge( PerfMonMTSvcCfg(flags) )

  from AthenaConfiguration.Utils import setupLoggingLevels
  setupLoggingLevels(flags,cfg)

  if cfg.getService("AthenaEventLoopMgr"): cfg.getService("AthenaEventLoopMgr").IntervalInSeconds = 30

  if type(args.dbOverrides)==list:
    from IOVDbSvc.IOVDbSvcConfig import addOverride
    #examples:
    #cfg.merge( addOverride(flags, folder="/TRIGGER/L1Calo/V1/Calibration/EfexEnergyCalib", db="sqlite://;schema=mytest.db;dbname=CONDBR2",tag="" ) )
    #cfg.merge( addOverride(flags, folder="/TRIGGER/L1Calo/V1/Calibration/EfexNoiseCuts", db="sqlite://;schema=/afs/cern.ch/user/w/will/calib.sqlite;dbname=L1CALO",tag="" ) )
    for override in args.dbOverrides:
      print(override)
      folderName,dbPath = override.split("=",1)
      if folderName == "": raise ValueError("Cannot parse dbOverride: " + override)
      if ";dbname=" not in dbPath: dbPath += ";dbname=CONDBR2"
      dbPath,dbInst = dbPath.split(";dbname=")
      if not os.path.exists(dbPath): raise ValueError("dbOverride file doesn't exist: " + dbPath)
      if folderName[0] != "/": folderName = "/TRIGGER/L1Calo/V1/Calibration/" + folderName
      print("Overriding COOL folder:",folderName,dbPath,dbInst)
      cfg.merge( addOverride(flags,folder=folderName,db=f"sqlite://;schema={dbPath};dbname={dbInst}",tag=""))


  # configure output AOD if requested
  if flags.Output.AODFileName != "":
    def addEDM(edmType, edmName):
      auxType = edmType.replace('Container','AuxContainer')
      return [f'{edmType}#{edmName}', f'{auxType}#{edmName}Aux.']

    outputEDM = []
    outputEDM += addEDM('xAOD::jFexTowerContainer'   , "L1_jFexDataTowers")
    outputEDM += addEDM('xAOD::jFexTowerContainer'   , "L1_jFexEmulatedTowers")
    outputEDM += addEDM('xAOD::jFexSRJetRoIContainer', 'L1_jFexSRJetRoISim')
    outputEDM += addEDM('xAOD::jFexLRJetRoIContainer', 'L1_jFexLRJetRoISim')
    outputEDM += addEDM('xAOD::jFexTauRoIContainer'  , 'L1_jFexTauRoISim'  )
    outputEDM += addEDM('xAOD::jFexFwdElRoIContainer', 'L1_jFexFwdElRoISim')
    outputEDM += addEDM('xAOD::jFexSumETRoIContainer', 'L1_jFexSumETRoISim')
    outputEDM += addEDM('xAOD::jFexMETRoIContainer'  , 'L1_jFexMETRoISim'  )
    outputEDM += addEDM('xAOD::jFexSRJetRoIContainer', 'L1_jFexSRJetRoI')
    outputEDM += addEDM('xAOD::jFexLRJetRoIContainer', 'L1_jFexLRJetRoI')
    outputEDM += addEDM('xAOD::jFexTauRoIContainer'  , 'L1_jFexTauRoI'  )
    outputEDM += addEDM('xAOD::jFexFwdElRoIContainer', 'L1_jFexFwdElRoI')
    outputEDM += addEDM('xAOD::jFexSumETRoIContainer', 'L1_jFexSumETRoI')
    outputEDM += addEDM('xAOD::jFexMETRoIContainer'  , 'L1_jFexMETRoI'  )

    outputEDM += addEDM('xAOD::jFexSRJetRoIContainer', 'L1_jFexSRJetxRoI')
    outputEDM += addEDM('xAOD::jFexLRJetRoIContainer', 'L1_jFexLRJetxRoI')
    outputEDM += addEDM('xAOD::jFexTauRoIContainer'  , 'L1_jFexTauxRoI'  )
    outputEDM += addEDM('xAOD::jFexFwdElRoIContainer', 'L1_jFexFwdElxRoI')
    outputEDM += addEDM('xAOD::jFexSumETRoIContainer', 'L1_jFexSumETxRoI')
    outputEDM += addEDM('xAOD::jFexMETRoIContainer'  , 'L1_jFexMETxRoI'  )
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    cfg.merge(OutputStreamCfg(flags, 'AOD', ItemList=outputEDM))
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    cfg.merge(SetupMetaDataForStreamCfg(flags, 'AOD'))

  # example of adding user algorithm
  # cfg.addEventAlgo(CompFactory.AnotherPackageAlg(),sequenceName="AthAlgSeq")

  for conf in args.postConfig:
    compName,propNameAndVal=conf.split(".",1)
    propName,propVal=propNameAndVal.split("=",1)
    applied = False
    for comp in cfg._allComponents():
      if comp.name==compName:
        applied = True
        exec(f"comp.{propNameAndVal}")
    if not applied:
      raise ValueError(f"postConfig {conf} had no effect ... typo?")

  # -------- CHANGES GO ABOVE ------------

if flags.Exec.MaxEvents==0: cfg.printConfig()
print("Configured Services:",*[svc.name for svc in cfg.getServices()])
#print("Configured EventAlgos:",*[alg.name for alg in cfg.getEventAlgos()])
#print("Configured CondAlgos:",*[alg.name for alg in cfg.getCondAlgos()])

#cfg.getService("StoreGateSvc").Dump=True

# ensure printout level is low enough if dumping
if cfg.getService("StoreGateSvc").Dump:
  cfg.getService("StoreGateSvc").OutputLevel=3


from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
L1CaloMonitorCfgHelper.printHanConfig()


if cfg.run().isFailure():
  import sys
  sys.exit(1)
