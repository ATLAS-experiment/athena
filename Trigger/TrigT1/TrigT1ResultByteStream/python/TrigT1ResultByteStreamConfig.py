#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format
from TrigEDMConfig.TriggerEDM import recordable
from TrigEDMConfig.Utils import getEDMListFromWriteHandles
from libpyeformat_helper import SourceIdentifier, SubDetector

from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import eFexByteStreamToolCfg, jFexRoiByteStreamToolCfg, jFexInputByteStreamToolCfg, gFexByteStreamToolCfg, gFexInputByteStreamToolCfg
from L1TopoByteStream.L1TopoByteStreamConfig import L1TopoPhase1ByteStreamToolCfg
from TrigT1MuonRecRoiTool.TrigT1MuonRecRoiToolConfig import RPCRecRoiToolCfg, TGCRecRoiToolCfg
from TrigT1MuctpiPhase1.TrigT1MuctpiPhase1Config import TrigThresholdDecisionToolCfg

_log = logging.getLogger('TrigT1ResultByteStreamConfig')

def RoIBResultByteStreamToolCfg(flags, name, writeBS=False):
  acc = ComponentAccumulator()
  tool = CompFactory.RoIBResultByteStreamTool(name)

  if not flags.Trigger.L1.doCTP:
    # disable CTP ByteStream decoding/encoding as part of RoIBResult
    tool.CTPModuleId = 0xFF

  if flags.Trigger.enableL1MuonPhase1 or not flags.Trigger.L1.doMuon:
    # disable legacy MUCTPI ByteStream decoding/encoding as part of RoIBResult
    tool.MUCTPIModuleId = 0xFF

  if not flags.Trigger.enableL1CaloLegacy or not flags.Trigger.L1.doCalo:
    # disable legacy L1Calo ByteStream decoding/encoding as part of RoIBResult
    tool.JetModuleIds = []
    tool.EMModuleIds = []

  if flags.Trigger.EDMVersion == 1 or not flags.Trigger.L1.doTopo:
    # disable legacy L1Topo ByteStream decoding/encoding as part of RoIBResult
    tool.L1TopoModuleIds = []

  if writeBS:
    # write BS == read RDO
    tool.RoIBResultReadKey="RoIBResult"
    tool.RoIBResultWriteKey=""
  else:
    # read BS == write RDO
    tool.RoIBResultReadKey=""
    tool.RoIBResultWriteKey="RoIBResult"

  acc.setPrivateTools(tool)
  return acc

def ExampleL1TriggerByteStreamToolCfg(flags, name, writeBS=False):
  acc = ComponentAccumulator()
  tool = CompFactory.ExampleL1TriggerByteStreamTool(name)
  muctpi_moduleid = 0
  muctpi_robid = int(SourceIdentifier(SubDetector.TDAQ_MUON_CTP_INTERFACE, muctpi_moduleid))
  tool.ROBIDs = [muctpi_robid]
  if writeBS:
    # write BS == read xAOD
    tool.MuonRoIContainerReadKey="LVL1MuonRoIs"
    tool.MuonRoIContainerWriteKey=""
    tool.L1TopoOutputLocID=""
  else:
    # read BS == write xAOD
    tool.MuonRoIContainerReadKey=""
    tool.MuonRoIContainerWriteKey=recordable("LVL1MuonRoIs")
  acc.setPrivateTools(tool)
  return acc

def MuonRoIByteStreamToolCfg(flags, name, writeBS=False):
  acc = ComponentAccumulator()
  tool = CompFactory.MuonRoIByteStreamTool(name)
  muctpi_moduleid = 0  # No RoIB in Run 3, we always read the DAQ ROB
  muctpi_robid = int(SourceIdentifier(SubDetector.TDAQ_MUON_CTP_INTERFACE, muctpi_moduleid)) # 0x760000
  tool.ROBIDs = [muctpi_robid]
  tool.DoTopo = flags.Trigger.L1.doMuonTopoInputs

  from TrigT1ResultByteStream.TrigT1ResultByteStreamMonitoringConfig import L1MuonBSConverterMonitoringCfg
  tool.MonTool = acc.popToolsAndMerge(L1MuonBSConverterMonitoringCfg(flags, name, writeBS))

  # Build container names for each bunch crossing in the maximum readout window (size 5)
  containerBaseName = "LVL1MuonRoIs"
  containerNames = [
    containerBaseName + "BCm2",
    containerBaseName + "BCm1",
    containerBaseName,
    containerBaseName + "BCp1",
    containerBaseName + "BCp2",
  ]
  topocontainerBaseName = "L1MuCTPItoL1TopoLocationFromMuonRoI"
  topocontainerNames = [
    topocontainerBaseName + "-2",
    topocontainerBaseName + "-1",
    topocontainerBaseName,
    topocontainerBaseName + "1",
    topocontainerBaseName + "2",
  ]
  if writeBS:
    # write BS == read xAOD
    tool.MuonRoIContainerReadKeys += containerNames
  else:
    # read BS == write xAOD
    tool.MuonRoIContainerWriteKeys += [recordable(c) for c in containerNames]
    tool.L1TopoOutputLocID += topocontainerNames

  tool.RPCRecRoiTool = acc.popToolsAndMerge(RPCRecRoiToolCfg(flags))
  tool.TGCRecRoiTool = acc.popToolsAndMerge(TGCRecRoiToolCfg(flags))
  tool.TrigThresholdDecisionTool = acc.popToolsAndMerge(TrigThresholdDecisionToolCfg(flags))

  acc.setPrivateTools(tool)
  return acc

def doRoIBResult(flags):
  '''
  Helper function returning a logic combination of flags deciding
  whether the RoIBResult decoding/encoding is required in the job
  '''
  if flags.Trigger.L1.doCalo and flags.Trigger.enableL1CaloLegacy:
    # Only needed for legacy (Run-2) L1Calo system
    return True
  if flags.Trigger.L1.doMuon and not flags.Trigger.enableL1MuonPhase1:
    # Only needed for legacy (Run-2) MUCTPI data
    return True
  if flags.Trigger.L1.doTopo:
    # Currently only RoIBResult path implemented for L1Topo
    return True
  if flags.Trigger.L1.doCTP:
    # Currently only RoIBResult path implemented for CTP
    return True
  # Otherwise don't need RoIBResult
  return False

def L1TriggerByteStreamDecoderCfg(flags, returnEDM=False):
  acc = ComponentAccumulator()
  decoderTools = []
  maybeMissingRobs = []

  ########################################
  # Legacy decoding via RoIBResult
  ########################################
  if not flags.Trigger.doLVL1: #if we rerun L1, don't decode the original RoIBResult
    if doRoIBResult(flags):
      roibResultTool = acc.popToolsAndMerge(RoIBResultByteStreamToolCfg(
        flags, name="RoIBResultBSDecoderTool", writeBS=False))
      decoderTools += [roibResultTool]
      # Always treat L1Topo as "maybe missing" as it was under commissioning in Run 2 and had readout issues in Run 3
      for module_id in roibResultTool.L1TopoModuleIds:
        maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_TOPO_PROC, module_id)))
      if flags.Trigger.EDMVersion == 2 and not flags.Trigger.doHLT:
        # L1Calo occasional readout errors weren't caught by HLT in 2015 - ignore these in offline reco, see ATR-24493
        for module_id in roibResultTool.JetModuleIds:
          maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_JET_PROC_ROI, module_id)))
        for module_id in roibResultTool.EMModuleIds:
          maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_CLUSTER_PROC_ROI, module_id)))

  ########################################
  # Run-3 L1Muon decoding (only when running HLT - offline we read it from HLT result)
  ########################################
  if flags.Trigger.L1.doMuon and flags.Trigger.enableL1MuonPhase1 and flags.Trigger.doHLT:
    muonRoiTool = acc.popToolsAndMerge(MuonRoIByteStreamToolCfg(
      flags, name="L1MuonBSDecoderTool", writeBS=False))
    decoderTools += [muonRoiTool]

  ########################################
  # Run-3 L1Calo decoding
  ########################################
  if flags.Trigger.L1.doCalo and flags.Trigger.enableL1CaloPhase1:
    # Add a temporary flag _decodeTOBsOfflineDuringCommissioning to enable decoding TOBs in reco/monitoring jobs
    # during commissioning of the phase-1 L1Calo system. Once the online decoding of TOBs in HLT is fully commissioned
    # and running, the reco/monitoring jobs should always read the TOBs from HLT result instead of decoding them again,
    # as the TOBs in HLT result are the objects linked to Trigger Decisions in the HLT Navigation and can be used for
    # analysis and studies. The duplicated TOBs decoded offline are only needed as long as TOBs in HLT result may be
    # missing because the decoding is not yet enabled in HLT. See ATR-26025 and ATR-26026.
    _decodeTOBsOfflineDuringCommissioning = True  # TODO: Remove this and relevant code below once no longer needed
    _extraOfflineTOBSuffix = '_OfflineCopy'

    #--------------------
    # eFex
    #--------------------
    if flags.Trigger.L1.doeFex:
      # Online case in HLT with TOB decoding only
      if flags.Trigger.doHLT:
        eFexByteStreamTool = acc.popToolsAndMerge(eFexByteStreamToolCfg(
          flags,
          'eFexBSDecoderTool',
          writeBS=False,
          TOBs=True,
          xTOBs=False,
          multiSlice=False
        ))
      # Reco/monitoring case with temporarily added decoding of TOBs
      elif _decodeTOBsOfflineDuringCommissioning:
        eFexByteStreamTool = acc.popToolsAndMerge(eFexByteStreamToolCfg(
          flags,
          'eFexBSDecoderTool',
          writeBS=False,
          TOBs=True,
          xTOBs=True,
          multiSlice=True,
          decodeInputs=flags.Trigger.L1.doCaloInputs
        ))
        eFexByteStreamTool.eEMContainerWriteKey.Path += _extraOfflineTOBSuffix
        eFexByteStreamTool.eTAUContainerWriteKey.Path += _extraOfflineTOBSuffix
      # Reco/monitoring case (either online but downstream from HLT, or at Tier-0) with xTOB, input tower and multi-slice decoding
      else:
        eFexByteStreamTool = acc.popToolsAndMerge(eFexByteStreamToolCfg(
          flags,
          'eFexBSDecoderTool',
          writeBS=False,
          TOBs=False,
          xTOBs=True,
          multiSlice=True,
          decodeInputs=flags.Trigger.L1.doCaloInputs
        ))
      decoderTools += [eFexByteStreamTool]
      # Allow the data to be missing at T0, due to the commissioning of the phase-1 L1Calo in RAW data from 2022
      # Forbit the data to be missing at Point 1 (2023+)
      if not flags.Trigger.doHLT:
        maybeMissingRobs += eFexByteStreamTool.ROBIDs  

    #--------------------
    # jFex
    #--------------------
    if flags.Trigger.L1.dojFex:
      # Online case in HLT with TOB decoding only
      if flags.Trigger.doHLT:
        jFexRoiByteStreamTool = acc.popToolsAndMerge(jFexRoiByteStreamToolCfg(
          flags,
          'jFexBSDecoderTool',
          writeBS=False
        ))
      # Reco/monitoring case (either online but downstream from HLT, or at Tier-0) with xTOB decoding only
      else:
        jFexRoiByteStreamTool = acc.popToolsAndMerge(jFexRoiByteStreamToolCfg(
          flags,
          'jFexBSDecoderTool',
          writeBS=False,
          xTOBs=True
        ))
      decoderTools += [jFexRoiByteStreamTool]
      maybeMissingRobs += jFexRoiByteStreamTool.ROBIDs  # Allow the data to be missing during commissioning of the phase-1 L1Calo (2022)

      # Input towers decoding
      if flags.Trigger.L1.doCaloInputs:
        jFexInputByteStreamTool = acc.popToolsAndMerge(jFexInputByteStreamToolCfg(
          flags,
          'jFexInputBSDecoderTool',
          writeBS=False
        ))
        decoderTools += [jFexInputByteStreamTool]
        maybeMissingRobs += jFexInputByteStreamTool.ROBIDs  # Allow the data to be missing during commissioning of the phase-1 L1Calo (2022)

      # Temporary addition of TOB decoding to reco/monitoring case:
      if _decodeTOBsOfflineDuringCommissioning and not flags.Trigger.doHLT:
        jFexRoiByteStreamTool_TOB = acc.popToolsAndMerge(jFexRoiByteStreamToolCfg(
          flags,
          'jFexBSDecoderTool_TOB',
          writeBS=False,
          xTOBs=False
        ))
        jFexRoiByteStreamTool_TOB.jJRoIContainerWriteKey.Path += _extraOfflineTOBSuffix
        jFexRoiByteStreamTool_TOB.jLJRoIContainerWriteKey.Path += _extraOfflineTOBSuffix
        jFexRoiByteStreamTool_TOB.jTauRoIContainerWriteKey.Path += _extraOfflineTOBSuffix
        jFexRoiByteStreamTool_TOB.jEMRoIContainerWriteKey.Path += _extraOfflineTOBSuffix
        jFexRoiByteStreamTool_TOB.jTERoIContainerWriteKey.Path += _extraOfflineTOBSuffix
        jFexRoiByteStreamTool_TOB.jXERoIContainerWriteKey.Path += _extraOfflineTOBSuffix
        decoderTools += [jFexRoiByteStreamTool_TOB]
        maybeMissingRobs += jFexRoiByteStreamTool_TOB.ROBIDs

    #--------------------
    # gFex
    #--------------------
    if flags.Trigger.L1.dogFex:
      # Online case in HLT with TOB decoding (no 'else' case because gFex doesn't have xTOBs to decode offline)
      if flags.Trigger.doHLT:
        gFexByteStreamTool = acc.popToolsAndMerge(gFexByteStreamToolCfg(
          flags,
          'gFexBSDecoderTool',
          writeBS=False
        ))
        decoderTools += [gFexByteStreamTool]
        maybeMissingRobs += gFexByteStreamTool.ROBIDs  # Allow the data to be missing during commissioning of the phase-1 L1Calo (2022)
      # Reco/monitoring case with temporarily added decoding of TOBs
      elif _decodeTOBsOfflineDuringCommissioning:
        gFexByteStreamTool_TOB = acc.popToolsAndMerge(gFexByteStreamToolCfg(
          flags,
          'gFexBSDecoderTool_TOB',
          writeBS=False
        ))
        gFexByteStreamTool_TOB.gFexRhoOutputContainerWriteKey.Path += _extraOfflineTOBSuffix
        gFexByteStreamTool_TOB.gFexSRJetOutputContainerWriteKey.Path += _extraOfflineTOBSuffix
        gFexByteStreamTool_TOB.gFexLRJetOutputContainerWriteKey.Path += _extraOfflineTOBSuffix
        gFexByteStreamTool_TOB.gScalarEJwojOutputContainerWriteKey.Path += _extraOfflineTOBSuffix
        gFexByteStreamTool_TOB.gMETComponentsJwojOutputContainerWriteKey.Path += _extraOfflineTOBSuffix
        gFexByteStreamTool_TOB.gMHTComponentsJwojOutputContainerWriteKey.Path += _extraOfflineTOBSuffix
        gFexByteStreamTool_TOB.gMSTComponentsJwojOutputContainerWriteKey.Path += _extraOfflineTOBSuffix
        gFexByteStreamTool_TOB.gMETComponentsNoiseCutOutputContainerWriteKey.Path += _extraOfflineTOBSuffix
        gFexByteStreamTool_TOB.gMETComponentsRmsOutputContainerWriteKey.Path += _extraOfflineTOBSuffix
        gFexByteStreamTool_TOB.gScalarENoiseCutOutputContainerWriteKey.Path += _extraOfflineTOBSuffix
        gFexByteStreamTool_TOB.gScalarERmsOutputContainerWriteKey.Path += _extraOfflineTOBSuffix
        decoderTools += [gFexByteStreamTool_TOB]
        maybeMissingRobs += gFexByteStreamTool_TOB.ROBIDs

      # Input towers decoding
      if flags.Trigger.L1.doCaloInputs:
        gFexInputByteStreamTool = acc.popToolsAndMerge(gFexInputByteStreamToolCfg(
          flags,
          'gFexInputBSDecoderTool',
          writeBS=False
        ))
        decoderTools += [gFexInputByteStreamTool]
        maybeMissingRobs += gFexInputByteStreamTool.ROBIDs  # Allow the data to be missing during commissioning of the phase-1 L1Calo (2022)

  ########################################
  # Run-3 L1Topo decoding
  ########################################
  if flags.Trigger.L1.doTopo and flags.Trigger.enableL1CaloPhase1 and flags.Trigger.L1.doTopoPhase1:
    topoByteStreamTool = acc.popToolsAndMerge(L1TopoPhase1ByteStreamToolCfg(
      flags,
      "L1TopoBSDecoderTool",
      writeBS=False
    ))
    decoderTools += [topoByteStreamTool]
    maybeMissingRobs += topoByteStreamTool.ROBIDs  # Allow the data to be missing during commissioning of the phase-1 L1Topo (2022)

  decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(name="L1TriggerByteStreamDecoder",
                                                         DecoderTools=decoderTools,
                                                         MaybeMissingROBs=list(set(maybeMissingRobs)))

  if flags.Trigger.doHLT or flags.DQ.Steering.doHLTMon:
    from TrigT1ResultByteStream.TrigT1ResultByteStreamMonitoringConfig import L1TriggerByteStreamDecoderMonitoringCfg
    decoderAlg.MonTool = acc.popToolsAndMerge(L1TriggerByteStreamDecoderMonitoringCfg(flags, decoderAlg.getName(), decoderTools))

  acc.addEventAlgo(decoderAlg, primary=True)

  # The decoderAlg needs to load ByteStreamMetadata for the detector mask
  from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
  readBSAcc = ByteStreamReadCfg(flags)
  readBSAcc.getEventAlgo('SGInputLoader').Load.add(
    ('ByteStreamMetadataContainer', 'InputMetaDataStore+ByteStreamMetadata'))
  acc.merge(readBSAcc)

  # In reconstruction/monitoring jobs add the decoders' output EDM to the output file
  if not flags.Trigger.doHLT:
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    outputEDM = getEDMListFromWriteHandles([tool for tool in decoderAlg.DecoderTools if 'RoIBResult' not in tool.getName()])
    _log.info('Adding the following output EDM to ItemList: %s', outputEDM)
    acc.merge(addToESD(flags, outputEDM))
    acc.merge(addToAOD(flags, outputEDM))

  # Return outputEDM as a second object to be used for compatibility with RecExCommon output configuration,
  # because the above calls to addToESD/addtoAOD are no-op when this fragment is wrapped in RecExCommon.
  # See discussions in https://gitlab.cern.ch/atlas/athena/-/merge_requests/55891#note_5912844
  if returnEDM:
    return acc, outputEDM
  return acc

def L1TriggerByteStreamEncoderCfg(flags):
  acc = ComponentAccumulator()

  # Legacy encoding via RoIBResult
  if doRoIBResult(flags):
    roibResultTool = acc.popToolsAndMerge(RoIBResultByteStreamToolCfg(
      flags, name="RoIBResultBSEncoderTool", writeBS=True))
    acc.addPublicTool(roibResultTool)

  # Run-3 L1Muon encoding
  if flags.Trigger.L1.doMuon and flags.Trigger.enableL1MuonPhase1:
    muonRoiTool = acc.popToolsAndMerge(MuonRoIByteStreamToolCfg(
      flags, name="L1MuonBSEncoderTool", writeBS=True))
    acc.addPublicTool(muonRoiTool)

  # TODO: Run-3 L1Calo, L1Topo, CTP

  return acc

def MuCTPIPhase1ByteStreamAlgoCfg(flags):
  #print("MUCTPI DQ DEBUG python include algo")
  acc = ComponentAccumulator()
  alg = CompFactory.MuCTPIPhase1ByteStreamAlgo()
  acc.addEventAlgo(alg)
  return acc

if __name__ == '__main__':
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  flags = initConfigFlags()
  import glob
  import sys

  import argparse
  parser = argparse.ArgumentParser(prog='python -m TrigT1ResultByteStream.TrigT1ResultByteStreamConfig',
                                   description="""Bytestream decoder athena script.\n\n
                                   Example: python -m TrigT1ResultByteStream.TrigT1ResultByteStreamConfig --filesInput "data22*" --evtMax 10 --outputs eTOBs exTOBs """)
  parser.add_argument('--evtMax',type=int,default=-1,help="number of events to process (-1 = til end of files)")
  parser.add_argument('--skipEvents',type=int,default=0,help="number of events to skip")
  parser.add_argument('--filesInput',nargs='+',help="input files",required=True)
  parser.add_argument('--userAlgs',nargs='+',help="names of user algorithms to add, can specify as Type/Name or just Type",default=[])
  parser.add_argument('--outputLevel',default="WARNING",choices={ 'INFO','WARNING','DEBUG','VERBOSE'})
  parser.add_argument('--outputHISTFile',default="",help="if specified, will activate monitoring")
  parser.add_argument('--outputs',nargs='+',choices={"eTOBs","exTOBs","seTOBs","eDataTowers","jTOBs","jxTOBS","jTowers","gTOBs","gCaloTowers","Topo","legacy","tTowers","sCells","eEmulatedTowers"},required=True,
                      help="What data to decode and output.")
  args = parser.parse_args()

  _log.setLevel(logging.DEBUG)

  from AthenaCommon import Constants
  algLogLevel = getattr(Constants,args.outputLevel)


  if any(["data23" in f for f in args.filesInput]):
    flags.IOVDb.GlobalTag = "CONDBR2-HLTP-2022-02"
    flags.Trigger.triggerConfig='DB'
    from AthenaConfiguration.Enums import LHCPeriod
    flags.GeoModel.Run = LHCPeriod.Run3 # needed for LArGMConfig

  elif any(["data22" in f for f in args.filesInput]):
    flags.Trigger.triggerConfig='DB'
    from AthenaConfiguration.Enums import LHCPeriod
    flags.GeoModel.Run = LHCPeriod.Run3 # needed for LArGMConfig
    flags.IOVDb.GlobalTag = "CONDBR2-ES1PA-2022-07"
  
  

  flags.Exec.OutputLevel = algLogLevel
  flags.Exec.MaxEvents = args.evtMax
  flags.Exec.SkipEvents = args.skipEvents
  flags.Input.Files = [file for x in args.filesInput for file in glob.glob(x)]
  flags.Concurrency.NumThreads = 1
  flags.Concurrency.NumConcurrentEvents = 1
  flags.Output.HISTFileName = args.outputHISTFile

  if any(["data" in f for f in args.filesInput]):
    s=args.filesInput[0].replace('*','').replace('.data','')
    flags.Output.AODFileName = "AOD."+(s.split("/")[-1]).split('_SFO')[0]+"pool.root"
  else:
    flags.Output.AODFileName = 'AOD.pool.root'

  flags.DQ.useTrigger = False # don't do TrigDecisionTool in MonitorCfg helper methods
  flags.DQ.enableLumiAccess = False

  flags.Trigger.enableL1CaloLegacy = 'legacy' in args.outputs
  flags.lock()

  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
  acc = MainServicesCfg(flags)
  acc.merge(ByteStreamReadCfg(flags)) # configure reading bytestream


  # Generate run3 L1 menu
  from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg,generateL1Menu
  acc.merge(L1ConfigSvcCfg(flags))

  if not any(["data22" in f for f in args.filesInput]) and not any(["data23" in f for f in args.filesInput]):
    generateL1Menu(flags)


  decoderTools = []
  outputEDM = []
  maybeMissingRobs = []


  def addEDM(edmType, edmName):
    auxType = edmType.replace('Container','AuxContainer')
    return [f'{edmType}#{edmName}',
            f'{auxType}#{edmName}Aux.']

  if "eEmulatedTowers" in args.outputs:
    # building eTowers from supercells and trigger towers
    args.outputs += ["sCells","tTowers"] # need both of these

    # next few lines are attempt to include masked channels
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg # should this be LArOnOffIdMappingSCCfg?
    acc.merge( LArOnOffIdMappingCfg(flags) )
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    acc.merge(addFolders(flags,"/LAR/BadChannels/BadChannelsSC","LAR",tag="LARBadChannelsBadChannelsSC-RUN3-UPD1-00",className="CondAttrListCollection"))
    acc.addCondAlgo(CompFactory.LArBadChannelCondAlg(ReadKey="/LAR/BadChannels/BadChannelsSC",isSC=True,CablingKey="LArOnOffIdMapSC"))
        
    acc.addEventAlgo( CompFactory.LVL1.eFexTowerBuilder("eFexTowerBuilder"), sequenceName='AthAlgSeq' )
    outputEDM += addEDM('xAOD::eFexTowerContainer', acc.getEventAlgo("eFexTowerBuilder").eFexContainerWriteKey)

  if "sCells" in args.outputs:
    from L1CaloFEXSim.L1CaloFEXSimCfg import ReadSCellFromByteStreamCfg
    acc.merge( ReadSCellFromByteStreamCfg(flags,"SCell") )
    outputEDM += ["CaloCellContainer#SCell"]

  if "tTowers" in args.outputs:
    from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags, type_names=['xAOD::TriggerTowerContainer/xAODTriggerTowers',
                                                            'xAOD::TriggerTowerAuxContainer/xAODTriggerTowersAux.']))





  ########################################
  # Legacy decoding via RoIBResult
  ########################################
  if 'legacy' in args.outputs:
    # Produce xAOD L1 RoIs from RoIBResult. RoIB readout only
    from AnalysisTriggerAlgs.AnalysisTriggerAlgsCAConfig import RoIBResultToxAODCfg
    xRoIBResultAcc, xRoIBResultOutputs = RoIBResultToxAODCfg(flags)
    acc.merge(xRoIBResultAcc)

    roibResultTool = acc.popToolsAndMerge(RoIBResultByteStreamToolCfg(
      flags, name="RoIBResultBSDecoderTool", writeBS=False))
    decoderTools += [roibResultTool]

    # Always treat L1Topo as "maybe missing" as it was under commissioning in Run 2 and had readout issues in Run 3
    for module_id in roibResultTool.L1TopoModuleIds:
      maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_TOPO_PROC, module_id)))
    if flags.Trigger.EDMVersion == 2 and not flags.Trigger.doHLT:
      # L1Calo occasional readout errors weren't caught by HLT in 2015 - ignore these in offline reco, see ATR-24493
      for module_id in roibResultTool.JetModuleIds:
        maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_JET_PROC_ROI, module_id)))
      for module_id in roibResultTool.EMModuleIds:
        maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_CLUSTER_PROC_ROI, module_id)))


    outputEDM += addEDM('xAOD::JetEtRoI'         , 'LVL1JetEtRoI')
    outputEDM += addEDM('xAOD::JetRoIContainer'  , 'LVL1JetRoIs')
    outputEDM += addEDM('xAOD::EmTauRoIContainer', 'LVL1EmTauRoIs')
    outputEDM += addEDM('xAOD::EnergySumRoI'     , 'LVL1EnergySumRoI')

    # CPM and JEM RoIs (independent from above). Both readouts
    from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
    type_names = [
      # ===== CPM ================================================================
      "xAOD::CPMTowerContainer/CPMTowers",
      "xAOD::CPMTowerAuxContainer/CPMTowersAux.",
      "xAOD::CPMTowerContainer/CPMTowersOverlap",
      "xAOD::CPMTowerAuxContainer/CPMTowersOverlapAux.",
      # ===== CPMTOBROIS =========================================================
      "xAOD::CPMTobRoIContainer/CPMTobRoIs",
      "xAOD::CPMTobRoIAuxContainer/CPMTobRoIsAux.",
      "xAOD::CPMTobRoIContainer/CPMTobRoIsRoIB",
      "xAOD::CPMTobRoIAuxContainer/CPMTobRoIsRoIBAux.",
      # ===== JEMTOBROIS =========================================================
      "xAOD::JEMTobRoIContainer/JEMTobRoIs",
      "xAOD::JEMTobRoIAuxContainer/JEMTobRoIsAux.",
      "xAOD::JEMTobRoIContainer/JEMTobRoIsRoIB",
      "xAOD::JEMTobRoIAuxContainer/JEMTobRoIsRoIBAux.",
    ]
    acc.merge(ByteStreamReadCfg(flags, type_names=type_names))

    outputEDM += [item.replace('/','#') for item in type_names]

  ########################################
  # jFEX ROIs
  ########################################
  if 'jTOBs' in args.outputs:
    jFexTool = acc.popToolsAndMerge(jFexRoiByteStreamToolCfg(
      flags, 'jFexBSDecoder_TOB'))
    for module_id in jFexTool.ROBIDs:
        maybeMissingRobs.append(module_id)

    decoderTools += [jFexTool]

  ########################################
  # jFEX xTOBs
  ########################################
  if 'jxTOBs' in args.outputs:
    jFexTool = acc.popToolsAndMerge(jFexRoiByteStreamToolCfg(
      flags, 'jFexBSDecoder_xTOB', xTOBs=True))
    for module_id in jFexTool.ROBIDs:
        maybeMissingRobs.append(module_id)

    decoderTools += [jFexTool]

  ########################################
  # jFEX input Data
  ########################################
  if 'jTowers' in args.outputs:
    inputjFexTool = acc.popToolsAndMerge(jFexInputByteStreamToolCfg(
      flags, 'jFexInputBSDecoder'))
    for module_id in inputjFexTool.ROBIDs:
        maybeMissingRobs.append(module_id)

    decoderTools += [inputjFexTool]

  ########################################
  # eFEX ROIs and Input data
  ########################################
  if any( [x in args.outputs for x in ['eTOBs','exTOBs','eDataTowers']] ):
    eFexTool = acc.popToolsAndMerge(eFexByteStreamToolCfg(
      flags,
      'eFexBSDecoder',
      TOBs='eTOBs' in args.outputs,
      xTOBs='exTOBs' in args.outputs,
      decodeInputs='eDataTowers' in args.outputs
    ))
    # eFexTool_xTOBs = eFexByteStreamToolCfg('eFexBSDecoder_xTOBs', flags,xTOBs=True)
    decoderTools += [eFexTool]

    # allow for missing ROBs for eFEX decoding:
    maybeMissingRobs += eFexTool.ROBIDs

  ########################################
  # gFEX ROIs
  ########################################
  if 'gTOBs' in args.outputs:
    gFexTool = acc.popToolsAndMerge(gFexByteStreamToolCfg(
      flags, 'gFexBSDecoder'))
    decoderTools += [gFexTool]

  ########################################
  # gFEX input Data
  ########################################
  if 'gCaloTowers' in args.outputs:
    inputgFexTool = acc.popToolsAndMerge(gFexInputByteStreamToolCfg(
      flags, 'gFexInputBSDecoder'))
    decoderTools += [inputgFexTool]

  ########################################
  # Topo data
  ########################################
  if 'Topo' in args.outputs:
    l1topoBSTool = acc.popToolsAndMerge(L1TopoPhase1ByteStreamToolCfg(
      flags, "L1TopoBSDecoderTool"))
    decoderTools += [l1topoBSTool]
    # allow for missing Topo ROBs:
    maybeMissingRobs += l1topoBSTool.ROBIDs

  if 'seTOBs' in args.outputs:
    # efex simulation
    acc.addEventAlgo(CompFactory.LVL1.eTowerMakerFromSuperCells('eTowerMakerFromSuperCells'),sequenceName='AthAlgSeq') # builds eTowers from sCells
    acc.addEventAlgo(CompFactory.LVL1.eFEXDriver('eFEXDriver',
      eFEXSysSimTool = CompFactory.LVL1.eFEXSysSim('eFEXSysSimTool') # have to do this so that property settings below take effect
                                                 ),sequenceName='AthAlgSeq') # creates L1_eEMRoI etc
    acc.getEventAlgo('eFEXDriver').eFEXSysSimTool.Key_eFexEMOutputContainer = "L1_eEMRoISim" # changes key of output
    acc.getEventAlgo('eFEXDriver').eFEXSysSimTool.Key_eFexEMxTOBOutputContainer = "L1_eEMxRoISim"
    acc.getEventAlgo('eFEXDriver').eFEXSysSimTool.Key_eFexTauOutputContainer = "L1_eTauRoISim"
    acc.getEventAlgo('eFEXDriver').eFEXSysSimTool.Key_eFexTauxTOBOutputContainer = "L1_eTauxRoISim"

  decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(name="L1TriggerByteStreamDecoder",
                                                         DecoderTools=decoderTools, OutputLevel=algLogLevel, MaybeMissingROBs=list(set(maybeMissingRobs)))

  acc.addEventAlgo(decoderAlg, sequenceName='AthAlgSeq')

  from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
  outputEDM += getEDMListFromWriteHandles([tool for tool in decoderAlg.DecoderTools if 'RoIBResult' not in tool.getName()])
  _log.debug('Adding the following output EDM to ItemList: %s', outputEDM)
  acc.merge(OutputStreamCfg(flags, 'AOD', ItemList=outputEDM))

  # get rid of warning about propagating input attribute list ... since there is none
  # note it's odd that the AthenaCommon.globalflags input format property doesn't get updated appropriately by flags??
  acc.getEventAlgo("EventInfoTagBuilder").PropagateInput = (flags.Input.Format != Format.BS)


  if flags.Exec.MaxEvents==1: # if doing 1 event, will dump storegate too
    acc.getService("StoreGateSvc").Dump=True
    acc.getService("StoreGateSvc").OutputLevel=Constants.INFO

  # schedule user algs, if any
  for alg in args.userAlgs: acc.addEventAlgo(CompFactory.getComp(alg.split('/')[0])(name=alg.split('/')[-1]),sequenceName='AthEndSeq')



  if args.outputHISTFile != "":
    from AthenaMonitoring.AthMonitorCfgHelper import getDQTHistSvc
    acc.merge(getDQTHistSvc(flags))
    if "seTOBs" in args.outputs and "eTOBs" in args.outputs and "exTOBs" in args.outputs:
      from TrigT1CaloMonitoring.EfexSimMonitorAlgorithm import EfexSimMonitoringConfig
      acc.merge(EfexSimMonitoringConfig(flags))


  acc.printConfig()
  flags.dump()

  if acc.run().isFailure():
    sys.exit(1)
