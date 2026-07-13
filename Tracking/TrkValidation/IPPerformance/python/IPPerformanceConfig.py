# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.CFElements import seqAND
#from AthenaConfiguration.Enums import LHCPeriod

def EventStatusSelection_And_VertexSelectionCfg(flags):

    acc = ComponentAccumulator()
    if not flags.Input.isMC:
        acc.addEventAlgo(CompFactory.CP.EventStatusSelectionAlg("EventStatusSelectionAlg"))

    vertexSelectionAlg = CompFactory.CP.VertexSelectionAlg("VertexSelectionAlg",VertexContainer="PrimaryVertices",MinTracks=3)
    vertexSelectionAlg.CutFlowSvc = None
    acc.addEventAlgo(vertexSelectionAlg)
    return acc


def JetCalibratorAlgCfg(flags):
    acc = ComponentAccumulator()
    from JetCalibTools.JetCalibToolsConfig import defineJetCalibTool
    config = "JES_MC16Recommendation_Consolidated_EMTopo_Apr2019_Rel21.config"
    if flags.Input.isMC:
        if not flags.Sim.ISF.Simulator.isFullSim():
            config = "JES_MC16Recommendation_AFII_EMTopo_Apr2019_Rel21.config"
    if flags.Input.isMC:
        CalibSequence = "JetArea_Residual_EtaJES_GSC_Smear"
    else:
        CalibSequence = "JetArea_Residual_EtaJES_GSC_Insitu"
    jct = defineJetCalibTool(jetcollection="AntiKt4EMTopo",context="AnalysisLatest",configfile=config,calibarea="00-04-82",calibseq=CalibSequence,data_type = "mc" if flags.Input.isMC else "data",
          rhoname="auto", pvname="PrimaryVertices", gscdepth="auto")
    jetCalibratorAlg = CompFactory.JetCalibratorAlg("JetCalibratorAlg")
    jetCalibratorAlg.calibrationTool = jct
    acc.addEventAlgo(jetCalibratorAlg)
    return acc

def JetSelectorCfg(flags,name="JetSelector", **kwargs):
    acc = ComponentAccumulator()

    from JetSelectorTools.JetSelectorToolsConfig import JetCleaningToolCfg
    jetCleaningTool = acc.popToolsAndMerge(JetCleaningToolCfg(flags,name="JetCleaningTool",jetdef="AntiKt4EMTopoJets",cleaningLevel="LooseBad",useDecorations=True))

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
            TTVAToolCfg(flags,name="TTVATool", WorkingPoint="Prompt_MaxWeight")))

    if "trigDecTool" not in kwargs:
        from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
        kwargs.setdefault("trigDecTool", acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags)))

    if "trackSelectionTools" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            InDetTrackSelectionTool_LoosePrimary_Cfg, InDetTrackSelectionTool_TightPrimary_Cfg)
        kwargs.setdefault("trackSelectionTools", [
            acc.popToolsAndMerge(InDetTrackSelectionTool_LoosePrimary_Cfg(flags)),
            acc.popToolsAndMerge(InDetTrackSelectionTool_TightPrimary_Cfg(flags)) ])
    
    ipNtupleDumper = CompFactory.IPNtupleDumper(name, **kwargs)
#    if flags.GeoModel.Run == LHCPeriod.Run2:
#      ipNtupleDumper.JetsKey = "AntiKt4EMTopoJets_Selected"
#    else:
#      ipNtupleDumper.JetsKey = "AntiKt4EMPFlowJets"   
    acc.addEventAlgo(ipNtupleDumper) 
#    acc.addEventAlgo(CompFactory.IPNtupleDumper(name, **kwargs))
    return acc


def IPPerformanceCfg(flags,name="IPPerformance", **kwargs):
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    histSvc = CompFactory.THistSvc()
    histSvc.Output += ["MYSTREAM DATAFILE='IPPerformance.root' OPT='RECREATE'"]
    acc.addService(histSvc)
    
    seqName = "IPPerformanceSequence"
    acc.addSequence(seqAND(seqName))
    
    from IPPerformance.IPPerformanceConfig import EventStatusSelection_And_VertexSelectionCfg
    acc.merge(EventStatusSelection_And_VertexSelectionCfg(flags),sequenceName=seqName)

    from IPPerformance.IPPerformanceConfig import JetCalibratorAlgCfg
    acc.merge(JetCalibratorAlgCfg(flags),sequenceName=seqName)

    from IPPerformance.IPPerformanceConfig import JetSelectorCfg
    acc.merge(JetSelectorCfg(flags),sequenceName=seqName)

    from IPPerformance.IPPerformanceConfig import IPNtupleDumperCfg
    acc.merge(IPNtupleDumperCfg(flags),sequenceName=seqName)

    return acc
