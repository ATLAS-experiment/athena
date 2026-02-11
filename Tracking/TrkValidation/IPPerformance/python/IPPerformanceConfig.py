# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging
from SimulationConfig.SimEnums import SimulationFlavour
def EventSelectorAlgCfg(flags, name="EventSelectorAlg", **kwargs):

    acc = ComponentAccumulator()
    eventSelectorAlg = CompFactory.EventSelectorAlg("EventSelectorAlg", **kwargs)
    eventSelectorAlg.isMC = flags.Input.isMC
    eventSelectorAlg.configFileName = "$IPPerformance_DIR/data/IPPerformance/eventSelection_verysimple.config"
    acc.addEventAlgo(eventSelectorAlg)
    
    return acc


def JetCalibratorCfg(flags, name="JetCalibrator", **kwargs):
    acc = ComponentAccumulator()
    jct = CompFactory.JetCalibrationTool()
    #jct.name="jetCalibration"
    jct.JetCollection = "AntiKt4EMTopo"
    jct.ConfigFile = "JES_MC16Recommendation_Consolidated_EMTopo_Apr2019_Rel21.config"
    jct.CalibArea = "00-04-82"
    if flags.Input.isMC:
        jct.CalibSequence = "JetArea_Residual_EtaJES_GSC_Smear"
    else:
        jct.CalibSequence = "JetArea_Residual_EtaJES_GSC_Insitu"
        
    #jct.CalibSequence = "JetArea_Residual_EtaJES_GSC_Smear" #MC
    #jct.CalibSequence = "JetArea_Residual_EtaJES_GSC_Insitu" #Data
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
#    jetUncertaintiesTool.IsData = False #TODO 
    jetUncertaintiesTool.IsData = not flags.Input.isMC
    jetUncertaintiesTool.OutputLevel = logging.ERROR
   

    #filesInput = kwargs.get("filesInput", None)
    jetCalibrator = CompFactory.JetCalibrator("JetCalibrator", **kwargs)
    jetCalibrator.isMC = flags.Input.isMC
    jetCalibrator.isFullSim = flags.Sim.ISF.Simulator in [
            SimulationFlavour.FullG4MT,
            SimulationFlavour.FullG4MT_QS,
            SimulationFlavour.PassBackG4MT,
            SimulationFlavour.AtlasG4,
            SimulationFlavour.AtlasG4_QS,
            SimulationFlavour.CosmicsG4,
            ]

    #if filesInput is not None:
     #   jetCalibrator.filesInput = filesInput
    #else:
        #raise ValueError("filesInput parameter must be provided to JetCalibratorCfg.")
    #jetCalibrator.isFullSim = isFullSim
    jetCalibrator.jetCalibration = jct
    jetCalibrator.JESUncertTool = jetUncertaintiesTool 
    jetCalibrator.jetCleaning = jetCleaningTool 
    jetCalibrator.JVTTool = jvt 
    jetCalibrator.configFileName = "$IPPerformance_DIR/data/IPPerformance/jetCalibration_cmake.config" 

    acc.addEventAlgo(jetCalibrator)
    return acc


def JetSelectorCfg(flags,name="JetSelector", **kwargs):
    acc = ComponentAccumulator()
    jetCleaningTool = CompFactory.JetCleaningTool()
    jetCleaningTool.CutLevel = "LooseBad"
    jetCleaningTool.DoUgly = False

    jetSelector = CompFactory.JetSelector("JetSelector", **kwargs)
    jetSelector.jetCleaning = jetCleaningTool
    jetSelector.configFileName = "$IPPerformance_DIR/data/IPPerformance/jetSelection.config"

    acc.addEventAlgo(jetSelector)
    return acc

   

def IPNtupleDumperCfg(flags,name="IPNtupleDumper", **kwargs):
    acc = ComponentAccumulator()
    
    trackVertexAssociationTool= CompFactory.CP.TrackVertexAssociationTool()
    trackVertexAssociationTool.WorkingPoint= "Prompt_MaxWeight"
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    TriggerDecisionTool = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    LoosePrimaryselTool= CompFactory.InDet.InDetTrackSelectionTool()
    LoosePrimaryselTool.CutLevel = "LoosePrimary"
    TightPrimaryselTool= CompFactory.InDet.InDetTrackSelectionTool()
    TightPrimaryselTool.CutLevel = "TightPrimary"

    ipNtupleDumper = CompFactory.IPNtupleDumper("IPNtupleDumper",ipSaveHistosOnly=True,ipSaveAdditionalHistos=True, **kwargs)
    ipNtupleDumper.isMC = flags.Input.isMC
    ipNtupleDumper.trktovxtool = trackVertexAssociationTool
    ipNtupleDumper.trigDecTool = TriggerDecisionTool
    ipNtupleDumper.LoosePrimary_selTool = LoosePrimaryselTool
    ipNtupleDumper.TightPrimary_selTool = TightPrimaryselTool
    ipNtupleDumper.configFileName = "$IPPerformance_DIR/data/IPPerformance/IPNtupleDumper.config"

    acc.addEventAlgo(ipNtupleDumper)
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
