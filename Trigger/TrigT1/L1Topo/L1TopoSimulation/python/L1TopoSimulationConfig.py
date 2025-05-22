# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def L1LegacyTopoSimulationCfg(flags):
    
    acc = ComponentAccumulator()
    
    from L1TopoSimulation.L1TopoInputHistograms import configureEMTauInputProviderHistograms, configureEnergyInputProviderHistograms, configureJetInputProviderHistograms
    emtauProvider = CompFactory.LVL1.EMTauInputProvider("EMTauInputProvider")
    emtauProvider.MonTool = GenericMonitoringTool(flags, 'MonTool')
    emtauProvider.MonTool.HistPath = 'L1LegacyTopoSimulation/EMTauInputProvider'
    configureEMTauInputProviderHistograms(emtauProvider, flags)
    energyProvider = CompFactory.LVL1.EnergyInputProvider("EnergyInputProvider")
    energyProvider.MonTool = GenericMonitoringTool(flags, 'MonTool')
    energyProvider.MonTool.HistPath = 'L1LegacyTopoSimulation/EnergyInputProvider'
    configureEnergyInputProviderHistograms(energyProvider, flags)
    jetProvider = CompFactory.LVL1.JetInputProvider("JetInputProvider")
    jetProvider.MonTool = GenericMonitoringTool(flags, 'MonTool')
    jetProvider.MonTool.HistPath = 'L1LegacyTopoSimulation/JetInputProvider'
    configureJetInputProviderHistograms(jetProvider, flags)

    topoSimAlg = CompFactory.LVL1.L1TopoSimulation("L1LegacyTopoSimulation",
                                                    EMTAUInputProvider = emtauProvider,
                                                    JetInputProvider = jetProvider,
                                                    EnergyInputProvider = energyProvider,
                                                    IsLegacyTopo = True,
                                                    InputDumpFile = "inputdump_legacy.txt",
                                                    EnableInputDump = flags.Trigger.enableL1TopoDump,
                                                    UseBitwise = flags.Trigger.enableL1TopoBWSimulation,
                                                    MonHistBaseDir = "L1/L1LegacyTopoAlgorithms"
                                                   )

    # No muon inputs to legacy Topo
    topoSimAlg.MuonInputProvider.locationMuCTPItoL1Topo = ""
    topoSimAlg.MuonInputProvider.locationMuCTPItoL1Topo1 = ""
    topoSimAlg.MuonInputProvider.locationMuonRoI = ""
    topoSimAlg.MuonInputProvider.locationMuonRoI1 = ""
    topoSimAlg.MuonInputProvider.ROIBResultLocation = ""

    acc.addEventAlgo(topoSimAlg)
    return acc

def L1TopoSimulationCfg(flags, doMonitoring=True, readMuCTPI=False, name="L1TopoSimulation", DeactivateL1TopoMuons=False, UseMuonDecoder=False):

    acc = ComponentAccumulator()

    decoderTools = []
    maybeMissingRobs = []

    #Configure the MuonInputProvider
    muProvider=""

    if flags.Trigger.L1.doMuon and not DeactivateL1TopoMuons:
        muProvider = CompFactory.LVL1.MuonInputProvider("MuonInputProvider")

        """
        If muons coming from the decoding, we use MuonRoI, otherwise MuCTPIL1Topo
        So here we should be adding proper flag for P1, and when input file is RAW
        Simply, if muons are simulated, we will use MuCTPIL1Topo, if decoded MuonRoI
        """
    
        if readMuCTPI:
            muProvider.locationMuCTPItoL1Topo = ""
            muProvider.locationMuCTPItoL1Topo1 = ""
        else:
            muProvider.locationMuonRoI = ""
            muProvider.locationMuonRoI1 = ""
        
        #Configure the MuonRoiTools for the MIP
        from TrigT1MuonRecRoiTool.TrigT1MuonRecRoiToolConfig import RPCRecRoiToolCfg, TGCRecRoiToolCfg
        muProvider.RecRpcRoiTool = acc.popToolsAndMerge(RPCRecRoiToolCfg(flags))
        muProvider.RecTgcRoiTool = acc.popToolsAndMerge(TGCRecRoiToolCfg(flags))

        if UseMuonDecoder:
            from MuonConfig.MuonBytestreamDecodeConfig import RpcBytestreamDecodeCfg,TgcBytestreamDecodeCfg
            acc.merge(RpcBytestreamDecodeCfg(flags))
            acc.merge(TgcBytestreamDecodeCfg(flags))
            from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import MuonRoIByteStreamToolCfg
            muonRoiTool = acc.popToolsAndMerge(MuonRoIByteStreamToolCfg(flags, name="L1MuonBSDecoderToolInL1Topo", writeBS=False))
            decoderTools += [muonRoiTool]
            maybeMissingRobs += muonRoiTool.ROBIDs
            
    emtauProvider = ""
    jetProvider = ""
    energyProvider = ""

    if flags.Trigger.L1.doeFex:
        emtauProvider = CompFactory.LVL1.eFexInputProvider("eFexInputProvider")
    if flags.Trigger.L1.dojFex:
        jetProvider = CompFactory.LVL1.jFexInputProvider("jFexInputProvider")
    if flags.Trigger.L1.dogFex:
        energyProvider = CompFactory.LVL1.gFexInputProvider("gFexInputProvider")

    controlHistSvc = CompFactory.LVL1.ControlHistSvc("ControlHistSvc")
    
    IsData = True
    if flags.Input.isMC:
        IsData = False

    from L1TopoByteStream.L1TopoByteStreamConfig import L1TopoPhase1ByteStreamToolCfg
    l1topoBSTool = acc.popToolsAndMerge(L1TopoPhase1ByteStreamToolCfg(flags, "L1TopoBSDecoderTool"))
    decoderTools += [l1topoBSTool]
    maybeMissingRobs += l1topoBSTool.ROBIDs

    if IsData:
        decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(name="L1TriggerByteStreamDecoder",
                                                         DecoderTools=decoderTools,
                                                         MaybeMissingROBs=maybeMissingRobs)

        acc.addEventAlgo(decoderAlg, sequenceName='AthAlgSeq')

    if not flags.Trigger.enableL1CaloPhase1:
        if (emtauProvider != ""):
            emtauProvider.eFexEMRoIKey = ""
            emtauProvider.eFexTauRoIKey = ""
        if (jetProvider != ""):
            jetProvider.jFexSRJetRoIKey = ""
            jetProvider.jFexLRJetRoIKey = ""
            jetProvider.jFexFwdElRoIKey = ""
            jetProvider.jFexTauRoIKey = ""
            jetProvider.jFexMETRoIKey = ""
            jetProvider.jFexSumETRoIKey = ""
        if (energyProvider != ""):
            energyProvider.gFexSRJetRoIKey = ""
            energyProvider.gFexLRJetRoIKey = ""
            energyProvider.gMETComponentsJwojKey = ""
            energyProvider.gMHTComponentsJwojKey = ""
            energyProvider.gMETComponentsNoiseCutKey = ""
            energyProvider.gMETComponentsRmsKey = ""
            energyProvider.gScalarEJwojKey = ""

    topoSimAlg = CompFactory.LVL1.L1TopoSimulation(name,
                                                    MuonInputProvider = muProvider,
                                                    EMTAUInputProvider = emtauProvider,
                                                    JetInputProvider = jetProvider,
                                                    EnergyInputProvider = energyProvider,
                                                    ControlHistSvc = controlHistSvc, # if doMonitoring else "",
                                                    IsLegacyTopo = False,
                                                    doMonitoring = doMonitoring,
                                                    EnableInputDump = flags.Trigger.enableL1TopoDump,
                                                    UseBitwise = flags.Trigger.enableL1TopoBWSimulation
                                                    )
    if name!="L1TopoSimulation":
        topoSimAlg.MonHistBaseDir = "L1/OnlineL1TopoAlgorithms"

    acc.addEventAlgo(topoSimAlg)

        
    if doMonitoring:
        from L1TopoOnlineMonitoring import L1TopoOnlineMonitoringConfig as TopoMonConfig
        if name=="L1TopoSimulation":
            acc.addEventAlgo(TopoMonConfig.getL1TopoPhase1OnlineMonitor(flags,'L1/L1TopoSimDecisions',
                                                                        doHwMon=IsData,doComp=IsData, doMultComp=IsData))
        else:
            acc.addEventAlgo(TopoMonConfig.getL1TopoPhase1OnlineMonitor(flags,'L1/OnlineL1TopoSimDecisions',
                                                                        toolName=name+"_Monitor",
                                                                        doHwMon=IsData,doComp=IsData, doMultComp=IsData))

    return acc

def L1TopoSimulationStandaloneCfg(flags, outputEDM=[], doMuons = False, doMonitoring=True):

    acc = ComponentAccumulator()

    efex_provider_attr = ['eFexEMRoI','eFexTauRoI']
    jfex_provider_attr = ['jFexSRJetRoI','jFexLRJetRoI','jFexFwdElRoI','jFexTauRoI','jFexMETRoI','jFexSumETRoI']
    gfex_provider_attr = ['gFexSRJetRoI','gFexLRJetRoI', 'gScalarEJwoj','gMETComponentsJwoj','gMHTComponentsJwoj', 'gMETComponentsNoiseCut', 'gMETComponentsRms']
    #Note: Unused container outputs from gFEX are:
    #'gFexRhoRoI', 'gMSTComponentsJwoj', 'gScalarENoiseCut', 'gScalarERms'
    #as compared with: https://gitlab.cern.ch/atlas/athena/-/blob/main/Trigger/TrigT1/L1CaloFEX/L1CaloFEXSim/L1CaloFEXSim/gFEXSysSim.h#L128-136


    from L1TopoSimulation.L1TopoInputHistograms import configureMuonInputProviderHistograms, configureeFexInputProviderHistograms, configurejFexInputProviderHistograms, configuregFexInputProviderHistograms

    #Configure the MuonInputProvider
    muProvider=""
    if doMuons:
        muProvider = CompFactory.LVL1.MuonInputProvider("MuonInputProvider")

        if flags.Trigger.L1.doMuonTopoInputs:
            muProvider.locationMuCTPItoL1Topo = ""
            muProvider.locationMuCTPItoL1Topo1 = ""
        else:
            muProvider.locationMuonRoI = ""
            muProvider.locationMuonRoI1 = ""

        #Configure the MuonRoiTools for the MIP
        from TrigT1MuonRecRoiTool.TrigT1MuonRecRoiToolConfig import RPCRecRoiToolCfg, TGCRecRoiToolCfg
        muProvider.RecRpcRoiTool = acc.popToolsAndMerge(RPCRecRoiToolCfg(flags))
        muProvider.RecTgcRoiTool = acc.popToolsAndMerge(TGCRecRoiToolCfg(flags))
        muProvider.MonTool = GenericMonitoringTool(flags, 'MonTool')
        muProvider.MonTool.HistPath = 'L1TopoSimulation/MuonInputProvider'
        configureMuonInputProviderHistograms(muProvider, flags)


    efexProvider = CompFactory.LVL1.eFexInputProvider("eFexInputProvider")
    efexProvider.MonTool = GenericMonitoringTool(flags, 'MonTool')
    efexProvider.MonTool.HistPath = 'L1TopoSimulation/eFexInputProvider'
    configureeFexInputProviderHistograms(efexProvider, flags)
    jfexProvider = CompFactory.LVL1.jFexInputProvider("jFexInputProvider")
    jfexProvider.MonTool = GenericMonitoringTool(flags, 'MonTool')
    jfexProvider.MonTool.HistPath = 'L1TopoSimulation/jFexInputProvider'
    configurejFexInputProviderHistograms(jfexProvider, flags)
    gfexProvider = CompFactory.LVL1.gFexInputProvider("gFexInputProvider")
    gfexProvider.MonTool = GenericMonitoringTool(flags, 'MonTool')
    gfexProvider.MonTool.HistPath = 'L1TopoSimulation/gFexInputProvider'
    configuregFexInputProviderHistograms(gfexProvider, flags)

    for attr in efex_provider_attr:
        res = [x for x in outputEDM if attr in x]
        if len(res)>0:
            key = res[0].split('#')[1]
            print (f'Key found for eFEX: {key}')
            setattr(efexProvider,attr+'Key',key)
        else:
            setattr(efexProvider,attr+'Key','')

    for attr in jfex_provider_attr:
        res = [x for x in outputEDM if attr in x]
        if len(res)>0:
            key = res[0].split('#')[1]
            print (f'Key found for jFEX: {key}')
            setattr(jfexProvider,attr+'Key',key)
        else:
            setattr(jfexProvider,attr+'Key','')

    for attr in gfex_provider_attr:
        res = [x for x in outputEDM if attr in x]
        if len(res)>0:
            key = res[0].split('#')[1]
            print (f'Key found for gFEX: {key}')
            setattr(gfexProvider,attr+'Key',key)
        else:
            setattr(gfexProvider,attr+'Key','')

    topoSimAlg = CompFactory.LVL1.L1TopoSimulation("L1TopoSimulation",
                                                    MuonInputProvider = muProvider,
                                                    EMTAUInputProvider = efexProvider,
                                                    JetInputProvider = jfexProvider,
                                                    EnergyInputProvider = gfexProvider,
                                                    IsLegacyTopo = False,
                                                    doMonitoring = doMonitoring,
                                                    EnableInputDump = flags.Trigger.enableL1TopoDump,
                                                    UseBitwise = flags.Trigger.enableL1TopoBWSimulation
                                                    )

    acc.addEventAlgo(topoSimAlg)
    
    return acc

if __name__ == '__main__':
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  from AthenaCommon.Logging import logging
  from AthenaCommon.Constants import VERBOSE,DEBUG,WARNING,INFO
  import argparse
  from argparse import RawTextHelpFormatter
  import sys
  from libpyeformat_helper import SourceIdentifier, SubDetector
  
  log = logging.getLogger('runL1TopoSim')
  log.setLevel(DEBUG)
  algLogLevel = DEBUG

  parser = argparse.ArgumentParser("Running L1TopoSimulation standalone for the BS input", formatter_class=RawTextHelpFormatter)
  parser.add_argument("-i","--inputs",nargs='*',action="store", dest="inputs", help="Inputs will be used in commands", required=True)
  parser.add_argument("-m","--module",action="store", dest="module", help="Input modules wants to be simulated.",default="", required=False)
  parser.add_argument("-bw","--useBitWise",action="store_true", dest="useBW", help="Run with L1Topo Bitwise simulation?",default=True, required=False)
  parser.add_argument("-ifex","--doCaloInput",action="store_true", dest="doCaloInput", help="Decoding L1Calo inputs",default=False, required=False)
  parser.add_argument("-fCtp","--forceCtp",action="store_true", dest="forceCtp", help="Force to CTP monitoring as primary in Sim/Hdw comparison.",default=False, required=False)
  parser.add_argument("-hdwMon","--algoHdwMon",action="store_true", dest="algoHdwMon", help="Fill algorithm histograms based on hardware decision.",default=False, required=False)
  parser.add_argument("-perfMon","--perfMonitoring",action="store_true", dest="perfmon", help="Enable performance monitoring",default=False, required=False)
  parser.add_argument("-redoMenu","--rederiveMenu",action="store_true", dest="redoMenu", help="Will rederive the menu based in local code",default=False, required=False)
  parser.add_argument("-l","--logLevel",action="store", dest="log", help="Log level.",default="warning", required=False)
  parser.add_argument("-n","--nevent", type=int, action="store", dest="nevent", help="Maximum number of events will be executed.",default=0, required=False)
  parser.add_argument("-s","--skipEvents", type=int, action="store", dest="skipEvents", help="How many events will be skipped.",default=0, required=False)
  parser.add_argument("-d","--enableL1TopoDump", type=bool, action="store", dest="enableL1TopoDump", help="Whether to output events into inputdump.txt",default=False, required=False) 
  args = parser.parse_args()

  supportedSubsystems = ['Muons','jFex','eFex','gFex','Topo']
  args_subsystem = args.module.split(',')
  subsystem = list( set(args_subsystem) & set(supportedSubsystems) )
  filename = args.inputs

  if len(subsystem)==0:
      log.warning(f'subsystem not given or the given subsystem not supported with one of the: {supportedSubsystems}')

  if args.log == 'info': algLogLevel = INFO  
  if args.log == 'warning': algLogLevel = WARNING
  if args.log == 'debug': algLogLevel = DEBUG
  if args.log == 'verbose': algLogLevel = VERBOSE

  flags = initConfigFlags()

  flags.Exec.OutputLevel = algLogLevel
  if(args.nevent > 0):
    flags.Exec.MaxEvents = args.nevent
  flags.Trigger.triggerMenuSetup = 'PhysicsP1_pp_run3_v1'
  if args.redoMenu:
      flags.Trigger.triggerConfig = 'FILE'
  else:
      flags.Trigger.triggerConfig = 'DB'
  flags.Input.Files = args.inputs
  flags.Concurrency.NumThreads = 1
  flags.Concurrency.NumConcurrentEvents = 1
  flags.Exec.SkipEvents = args.skipEvents
  flags.Output.AODFileName = 'AOD.pool.root'
  flags.Trigger.L1.doMuon = True
  flags.Trigger.enableL1MuonPhase1 = True
  flags.Trigger.L1.doMuonTopoInputs = True
  flags.Trigger.enableL1TopoBWSimulation = args.useBW
  flags.PerfMon.doFullMonMT = args.perfmon
  flags.PerfMon.OutputJSON = 'perfmonmt_test.json'
  flags.Trigger.enableL1TopoDump = args.enableL1TopoDump 
  if not flags.Input.isMC:
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
  flags.lock()

  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  acc = MainServicesCfg(flags)

  if args.perfmon:
      from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
      acc.merge(PerfMonMTSvcCfg(flags))

  from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
  acc.merge(ByteStreamReadCfg(flags, type_names=['CTP_RDO/CTP_RDO']))

  # Generate run3 L1 menu
  from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg,generateL1Menu
  acc.merge(L1ConfigSvcCfg(flags))
  if "data22" not in filename:   
    generateL1Menu(flags)
  
  # Produce xAOD L1 RoIs from RoIBResult
  from AnalysisTriggerAlgs.AnalysisTriggerAlgsConfig import RoIBResultToxAODCfg
  xRoIBResultAcc, xRoIBResultOutputs = RoIBResultToxAODCfg(flags)
  acc.merge(xRoIBResultAcc)
  
  decoderTools = []
  outputEDM = []
  maybeMissingRobs = []

  from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import RoIBResultByteStreamToolCfg
  roibResultTool = acc.popToolsAndMerge(RoIBResultByteStreamToolCfg(flags, name="RoIBResultBSDecoderTool", writeBS=False))
  decoderTools += [roibResultTool]

  for module_id in roibResultTool.L1TopoModuleIds:
      maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_TOPO_PROC, module_id)))

  for module_id in roibResultTool.JetModuleIds:
      maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_JET_PROC_ROI, module_id)))

  for module_id in roibResultTool.EMModuleIds:
      maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_CLUSTER_PROC_ROI, module_id)))

  
  def addEDM(edmType, edmName):
    auxType = edmType.replace('Container','AuxContainer')
    return [f'{edmType}#{edmName}',
            f'{auxType}#{edmName}Aux.']

  outputEDM += ['CTP_RDO#*']
  outputEDM += ['ROIB::RoIBResult#*']

  outputEDM += addEDM('xAOD::JetEtRoI'         , 'LVL1JetEtRoI')
  outputEDM += addEDM('xAOD::JetRoIContainer'  , 'LVL1JetRoIs')
  outputEDM += addEDM('xAOD::EmTauRoIContainer', 'LVL1EmTauRoIs')
  #different naming scheme of Aux branches and types for legacy EnergySumRoI (MET,sumET)
  outputEDM += ["xAOD::EnergySumRoI#LVL1EnergySumRoI", "xAOD::EnergySumRoIAuxInfo#LVL1EnergySumRoIAux."]
  
  if 'Muons' in subsystem:
      from MuonConfig.MuonBytestreamDecodeConfig import RpcBytestreamDecodeCfg,TgcBytestreamDecodeCfg
      rpcdecodingAcc = RpcBytestreamDecodeCfg(flags)
      acc.merge(rpcdecodingAcc)
      tgcdecodingAcc = TgcBytestreamDecodeCfg(flags) 
      acc.merge(tgcdecodingAcc)
      
      from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import MuonRoIByteStreamToolCfg
      muonRoiTool = acc.popToolsAndMerge(MuonRoIByteStreamToolCfg(flags, name="L1MuonBSDecoderTool", writeBS=False))
      decoderTools += [muonRoiTool]
      outputEDM += addEDM('xAOD::MuonRoIContainer'     , '*')
      maybeMissingRobs += muonRoiTool.ROBIDs

  if 'jFex' in subsystem:
      from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import jFexRoiByteStreamToolCfg,jFexInputByteStreamToolCfg
      jFexTool = acc.popToolsAndMerge(jFexRoiByteStreamToolCfg(flags, 'jFexBSDecoder', writeBS=False))
      decoderTools += [jFexTool]
      outputEDM += addEDM('xAOD::jFexSRJetRoIContainer', jFexTool.jJRoIContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::jFexLRJetRoIContainer', jFexTool.jLJRoIContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::jFexTauRoIContainer'  , jFexTool.jTauRoIContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::jFexFwdElRoIContainer', jFexTool.jEMRoIContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::jFexSumETRoIContainer', jFexTool.jTERoIContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::jFexMETRoIContainer'  , jFexTool.jXERoIContainerWriteKey.Path)
      maybeMissingRobs += jFexTool.ROBIDs
      if args.doCaloInput:
          jFexInputByteStreamTool = acc.popToolsAndMerge(jFexInputByteStreamToolCfg(flags, 'jFexInputBSDecoderTool', writeBS=False))
          decoderTools += [jFexInputByteStreamTool]
          outputEDM += addEDM('xAOD::jFexTowerContainer', jFexInputByteStreamTool.jTowersWriteKey.Path)
          maybeMissingRobs += jFexInputByteStreamTool.ROBIDs
      

  if 'eFex' in subsystem:
      from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import eFexByteStreamToolCfg
      eFexTool = acc.popToolsAndMerge(eFexByteStreamToolCfg(flags, 'eFexBSDecoder', writeBS=False, decodeInputs=args.doCaloInput))
      decoderTools += [eFexTool]
      outputEDM += addEDM('xAOD::eFexEMRoIContainer', eFexTool.eEMContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::eFexTauRoIContainer', eFexTool.eTAUContainerWriteKey.Path)
      if args.doCaloInput:
          outputEDM += addEDM('xAOD::eFexTowerContainer', eFexTool.eTowerContainerWriteKey.Path)
      maybeMissingRobs += eFexTool.ROBIDs


  if 'gFex' in subsystem:
      from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import gFexByteStreamToolCfg,gFexInputByteStreamToolCfg
      gFexTool = acc.popToolsAndMerge(gFexByteStreamToolCfg(flags, 'gFexBSDecoder', writeBS=False))
      decoderTools += [gFexTool]
      outputEDM += addEDM('xAOD::gFexJetRoIContainer', gFexTool.gFexRhoOutputContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::gFexJetRoIContainer', gFexTool.gFexSRJetOutputContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::gFexJetRoIContainer', gFexTool.gFexLRJetOutputContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::gFexGlobalRoIContainer', gFexTool.gScalarEJwojOutputContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::gFexGlobalRoIContainer', gFexTool.gMETComponentsJwojOutputContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::gFexGlobalRoIContainer', gFexTool.gMHTComponentsJwojOutputContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::gFexGlobalRoIContainer', gFexTool.gMSTComponentsJwojOutputContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::gFexGlobalRoIContainer', gFexTool.gMETComponentsNoiseCutOutputContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::gFexGlobalRoIContainer', gFexTool.gMETComponentsRmsOutputContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::gFexGlobalRoIContainer', gFexTool.gScalarENoiseCutOutputContainerWriteKey.Path)
      outputEDM += addEDM('xAOD::gFexGlobalRoIContainer', gFexTool.gScalarERmsOutputContainerWriteKey.Path)
      maybeMissingRobs += gFexTool.ROBIDs
      if args.doCaloInput:
          gFexInputByteStreamTool = acc.popToolsAndMerge(gFexInputByteStreamToolCfg(flags, 'gFexInputByteStreamTool', writeBS=False))
          decoderTools += [gFexInputByteStreamTool]
          outputEDM += addEDM('xAOD::gFexTowerContainer', gFexInputByteStreamTool.gTowersWriteKey.Path)
          maybeMissingRobs += gFexInputByteStreamTool.ROBIDs

  if 'Topo' in subsystem:
      from L1TopoByteStream.L1TopoByteStreamConfig import L1TopoPhase1ByteStreamToolCfg
      l1topoBSTool = acc.popToolsAndMerge(L1TopoPhase1ByteStreamToolCfg(flags, "L1TopoBSDecoderTool"))
      decoderTools += [l1topoBSTool]
      outputEDM += addEDM('xAOD::L1TopoRawDataContainer', l1topoBSTool.L1TopoPhase1RAWDataWriteContainer.Path)
      maybeMissingRobs += l1topoBSTool.ROBIDs

  decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(name="L1TriggerByteStreamDecoder",
                                                         DecoderTools=decoderTools,
                                                         MaybeMissingROBs=maybeMissingRobs,
                                                         OutputLevel=algLogLevel)
  
  acc.addEventAlgo(decoderAlg, sequenceName='AthAlgSeq')
  
  roib2topo = CompFactory.LVL1.RoiB2TopoInputDataCnv(name='RoiB2TopoInputDataCnv')
  roib2topo.OutputLevel = algLogLevel
  acc.addEventAlgo(roib2topo, sequenceName="AthAlgSeq")
  from L1TopoByteStream.L1TopoByteStreamConfig import L1TopoByteStreamCfg
  acc.merge(L1TopoByteStreamCfg(flags), sequenceName='AthAlgSeq')
  outputEDM += addEDM('xAOD::L1TopoRawDataContainer', 'L1TopoRawData')

  acc.merge(L1TopoSimulationStandaloneCfg(flags,outputEDM,doMuons=('Muons' in subsystem)), sequenceName='AthAlgSeq')
  if args.algoHdwMon:
      acc.getEventAlgo('L1TopoSimulation').FillHistoBasedOnHardware = True
      acc.getEventAlgo('L1TopoSimulation').PrescaleDAQROBAccess = 1
  outputEDM += addEDM('xAOD::L1TopoSimResultsContainer','L1_TopoSimResults')
  
  # phase1 mon
  from L1TopoOnlineMonitoring import L1TopoOnlineMonitoringConfig as TopoMonConfig
  acc.addEventAlgo(
      TopoMonConfig.getL1TopoPhase1OnlineMonitor(flags,'L1/L1TopoOffline',True,True,True,True,True,args.forceCtp,algLogLevel),
      sequenceName="AthAlgSeq"
  )

  from GaudiSvc.GaudiSvcConf import THistSvc # noqa: F401
  histSvc = CompFactory.THistSvc(Output = ["EXPERT DATAFILE='expert-monitoring-l1topo.root', OPT='RECREATE'"])
  acc.addService(histSvc)

  from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
  log.debug('Adding the following output EDM to ItemList: %s', outputEDM)
  acc.merge(OutputStreamCfg(flags, 'AOD', ItemList=outputEDM))
  from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
  acc.merge(SetupMetaDataForStreamCfg(flags, 'AOD'))

  if args.log == 'verbose' or args.perfmon:
      acc.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
  
  if acc.run().isFailure():
    sys.exit(1)
