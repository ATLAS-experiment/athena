# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging
#from SimulationConfig.SimEnums import SimulationFlavour
def EventSelectorAlgCfg(flags, name="EventSelectorAlg", **kwargs):

    acc = ComponentAccumulator()
    eventSelectorAlg = CompFactory.EventSelectorAlg("EventSelectorAlg", **kwargs)
    eventSelectorAlg.isMC = flags.Input.isMC
    acc.addEventAlgo(eventSelectorAlg)
    
    return acc


def JetCalibratorCfg(flags, name="JetCalibrator", **kwargs):
    acc = ComponentAccumulator()
    jct = CompFactory.JetCalibrationTool()
    #jct.name="jetCalibration"
    jct.JetCollection = "AntiKt4EMTopo"
    #jct.ConfigFile = "JES_MC16Recommendation_Consolidated_EMTopo_Apr2019_Rel21.config"
    config = "JES_MC16Recommendation_Consolidated_EMTopo_Apr2019_Rel21.config"
    if flags.Input.isMC:
        if not flags.Sim.ISF.Simulator.isFullSim():
            config = "JES_MC16Recommendation_AFII_EMTopo_Apr2019_Rel21.config"
    jct.ConfigFile = config

    jct.CalibArea = "00-04-82"
    if flags.Input.isMC:
        jct.CalibSequence = "JetArea_Residual_EtaJES_GSC_Smear"
    else:
        jct.CalibSequence = "JetArea_Residual_EtaJES_GSC_Insitu"
        
    jct.IsData = not flags.Input.isMC
    #jct.RhoKey = "auto"
    #jct.PrimaryVerticesContainerName = "PrimaryVertices"
    #jct.GSCDepth = "auto"
    jct.OutputLevel = logging.INFO

    jetCleaningTool = CompFactory.JetCleaningTool()
    jetCleaningTool.CutLevel = "LooseBad"
    #this tool is not used
    jvt = CompFactory.JetVertexTaggerTool()
    jvt.JetContainer="Jets_Calib" #TOBE checked
    jvt.JVTFileName = "JetMomentTools/JVTlikelihood_20140805.root"

    jetUncertaintiesTool = CompFactory.JetUncertaintiesTool()
    #the following parameters have to be configured here 
    jetUncertaintiesTool.ConfigFile = "rel21/Summer2019/R4_GlobalReduction_SimpleJER.config"  
    jetUncertaintiesTool.JetDefinition = "AntiKt4EMTopo"  
    jetUncertaintiesTool.MCType = "MC16"  
    jetUncertaintiesTool.IsData = not flags.Input.isMC
    jetUncertaintiesTool.OutputLevel = logging.ERROR
   

    jetCalibrator = CompFactory.JetCalibrator("JetCalibrator", **kwargs)
    jetCalibrator.isMC = flags.Input.isMC
    jetCalibrator.isFullSim = flags.Sim.ISF.Simulator.isFullSim()
    jetCalibrator.jetCalibration = jct
    jetCalibrator.JESUncertTool = jetUncertaintiesTool 
    jetCalibrator.jetCleaning = jetCleaningTool 
    jetCalibrator.JVTTool = jvt 

    acc.addEventAlgo(jetCalibrator)
    return acc


def JetSelectorCfg(flags,name="JetSelector", **kwargs):
    acc = ComponentAccumulator()
    jetCleaningTool = CompFactory.JetCleaningTool()
    jetCleaningTool.CutLevel = "LooseBad"
    jetCleaningTool.DoUgly = False

    jetSelector = CompFactory.JetSelector("JetSelector", **kwargs)
    jetSelector.jetCleaning = jetCleaningTool

    acc.addEventAlgo(jetSelector)
    return acc

   

def IPNtupleDumperCfg(flags,name="IPNtupleDumper", **kwargs):
    acc = ComponentAccumulator()
   
    kwargs.setdefault("ipSaveHistosOnly", True)
    kwargs.setdefault("ipSaveAdditionalHistos", True)
    kwargs.setdefault("isMC", flags.Input.isMC)
    
    if "trktovxtool" not in kwargs:
        from TrackVertexAssociationTool.TrackVertexAssociationToolConfig import TTVAToolCfg
        kwargs.setdefault("trktovxtool", acc.popToolsAndMerge(
            TTVAToolCfg(flags, WorkingPoint="Prompt_MaxWeight")))

    if "trigDecTool" not in kwargs:
        from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
        kwargs.setdefault("trigDecTool", acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags)))

    if "trackSelectionTools" not in kwargs:
        from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
            InDetTrackSelectionTool_LoosePrimary_Cfg, InDetTrackSelectionTool_TightPrimary_Cfg)
        kwargs.setdefault("trackSelectionTools", [
            acc.popToolsAndMerge(InDetTrackSelectionTool_LoosePrimary_Cfg(flags)),
            acc.popToolsAndMerge(InDetTrackSelectionTool_TightPrimary_Cfg(flags)) ])

    acc.addEventAlgo(CompFactory.IPNtupleDumper(name, **kwargs))
    return acc


def IPPerformanceCfg(flags,name="IPPerformance", **kwargs):
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    histSvc = CompFactory.THistSvc()
    #histSvc.Output += ["MYSTREAM DATAFILE='" + args.outputFile + "' OPT='RECREATE'"]
    #histSvc.Output += ["MYSTREAM DATAFILE='{flags.Output.HISTFileName}' TYPE='ROOT' OPT='RECREATE'"]
    histSvc.Output += ["MYSTREAM DATAFILE='IPPerformance.root' OPT='RECREATE'"]
    #histSvc.Output += ["MYSTREAM DATAFILE='DAOD_IDTIDE.pool.root' OPT='UPDATE'"]
    acc.addService(histSvc)
    
    from IPPerformance.IPPerformanceConfig import EventSelectorAlgCfg
    acc.merge(EventSelectorAlgCfg(flags))

    from IPPerformance.IPPerformanceConfig import JetCalibratorCfg
    #@TODO: Use other ways to determine the type of the sample
    #acc.merge(JetCalibratorCfg(flags, filesInput="CI_samples"))
    acc.merge(JetCalibratorCfg(flags))

    from IPPerformance.IPPerformanceConfig import JetSelectorCfg
    acc.merge(JetSelectorCfg(flags))

    from IPPerformance.IPPerformanceConfig import IPNtupleDumperCfg
    acc.merge(IPNtupleDumperCfg(flags))

    return acc
