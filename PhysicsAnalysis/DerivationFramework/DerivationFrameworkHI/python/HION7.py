# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# HION7.py 

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND

#########################################################################################
#Skiming
def HION7SkimmingToolCfg(flags):
    """Configure the example skimming tool"""
    acc = ComponentAccumulator()
    ExtraData  = []
    ExtraData += ['xAOD::JetContainer/AntiKt2HIJets']
    ExtraData += ['xAOD::JetContainer/AntiKt4HIJets']

    acc.addSequence( seqAND("HION7Sequence") )
    acc.getSequence("HION7Sequence").ExtraDataForDynamicConsumers = ExtraData
    acc.getSequence("HION7Sequence").ProcessDynamicDataDependencies = True
    
    expression = "count(AntiKt2HIJets.pt > 25000) > 1 || count(AntiKt4HIJets.pt > 25000) > 1"

    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg    
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    acc.addPublicTool(CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "HION7StringSkimmingTool",
                                                                             expression = expression,
                                                                             TrigDecisionTool=tdt), 
                      primary = True)

    return(acc)                             


def HION7KernelCfg(flags, name='HION7Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()
    
#########################################################################################
#Thinning
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg,JetTrackParticleThinningCfg
    
    track_thinning_expression  = "InDetTrackParticles.pt > 0.9*GeV"
    TrackParticleThinningTool  = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
         flags,
         name                    = "PHYSTrackParticleThinningTool",
         StreamName              = kwargs['StreamName'], 
         SelectionString         = track_thinning_expression,
         InDetTrackParticlesKey  = "InDetTrackParticles"))

    AntiKt2HIJetsThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
         flags,
         name                    = "AntiKt2HIJetsThinningTool",
         StreamName              = kwargs['StreamName'],
         JetKey                  = "AntiKt2HIJets",
         SelectionString         = "AntiKt2HIJets.pt > 20*GeV",
         InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    AntiKt4HIJetsThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
         flags,
         name                    = "AntiKt4HIJetsThinningTool",
         StreamName              = kwargs['StreamName'],
         JetKey                  = "AntiKt4HIJets",
         SelectionString         = "AntiKt4HIJets.pt > 20*GeV",
         InDetTrackParticlesKey  = "InDetTrackParticles"))
    
   
    thinningTools = [TrackParticleThinningTool,
                    AntiKt2HIJetsThinningTool,
                    AntiKt4HIJetsThinningTool]
#########################################################################################
    skimmingTool = acc.getPrimaryAndMerge(HION7SkimmingToolCfg(flags))
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name,ThinningTools = thinningTools, SkimmingTools = [skimmingTool]),sequenceName="HION7Sequence")       
    return acc


def HION7Cfg(flags):
    
    acc = ComponentAccumulator()
    acc.merge(HION7KernelCfg(flags, name="HION7Kernel",StreamName = "StreamDAOD_HION7"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
#########################################################################################
#Slimming
    HION7SlimmingHelper = SlimmingHelper("HION7SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    HION7SlimmingHelper.SmartCollections = ["EventInfo",
                                            "Electrons",
                                            "Photons",
                                            "Muons",
                                            "PrimaryVertices",
                                            "InDetTrackParticles"]
    HION7SlimmingHelper.AllVariables =["AntiKt2HIJets",
                                       "AntiKt4HIJets",
                                       "CaloSums",
                                       "ZdcModules",
                                       "ZdcTriggerTowers",
                                       "MBTSForwardEventInfo",
                                       "MBTSModules"]
    HION7ItemList = HION7SlimmingHelper.GetItemList()

    acc.merge(OutputStreamCfg(flags, "DAOD_HION7", ItemList=HION7ItemList, AcceptAlgs=["HION7Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HION7", AcceptAlgs=["HION7Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
