#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
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

def MuonRoIByteStreamToolCfg(flags, name, writeBS=False, writeDecodedMuonRoIs=True):
  acc = ComponentAccumulator()
  tool = CompFactory.MuonRoIByteStreamTool(name)
  muctpi_moduleid = 0  # No RoIB in Run 3, we always read the DAQ ROB
  muctpi_robid = int(SourceIdentifier(SubDetector.TDAQ_MUON_CTP_INTERFACE, muctpi_moduleid)) # 0x760000
  tool.ROBIDs = [muctpi_robid]
  tool.DoTopo = flags.Trigger.L1.doMuonTopoInputs
  tool.WriteDecodedMuonRoIs = writeDecodedMuonRoIs

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
  if flags.Trigger.L1.doMuon and flags.Trigger.enableL1MuonPhase1 and flags.Trigger.doHLT :
    muonRoiTool = acc.popToolsAndMerge(MuonRoIByteStreamToolCfg(
      flags, name="L1MuonBSDecoderTool", writeBS=False))
    decoderTools += [muonRoiTool]

  ########################################
  # Run-3 L1Calo decoding
  ########################################
  if flags.Trigger.L1.doCalo and flags.Trigger.enableL1CaloPhase1:
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
  print("Please use: athena TrigT1CaloMonitoring/L1CaloPhase1Monitoring.py\nUse --help to see info about how options/examples for running the command")
  import sys
  sys.exit(1)
