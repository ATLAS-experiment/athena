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
    
    expression = ""
    #Trigger selection
    from DerivationFrameworkHI import ListTriggers
    from CoolConvUtilities.ParticleTypeUtil import getTypeForRun
    info=getTypeForRun(flags.Input.RunNumbers[0])
    isSmallSystem = False
    if (info.getBeam1Type() < 11) or (info.getBeam2Type() < 11):
        isSmallSystem = True
    if not flags.Input.isMC:
        print('project: ', flags.Input.ProjectName,', isSmallSystem: ', isSmallSystem)
        TriggerDict = ListTriggers.GetTriggers(flags.Input.ProjectName, isSmallSystem)
        for i, key in enumerate(TriggerDict):
            expression = expression + '(' + key + ' && count(AntiKt4HIJets.pt >' + str(TriggerDict[key]) + '*GeV) >=1 ) ' + '|| (' + key + ' && count(AntiKt2HIJets.pt >' + str(TriggerDict[key]) + '*GeV) >=1 ) '
            if not i == len(TriggerDict) - 1:
                expression = expression + ' || '

    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg    
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    acc.addPublicTool(CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "HION7StringSkimmingTool",
                                                                             expression = expression,
                                                                             TrigDecisionTool=tdt), 
                      primary = True)

    return(acc)                             

def HION7GlobalAugmentationToolCfg(flags):
    """Configure the example augmentation tool"""
    acc = ComponentAccumulator()
    
    # Configure the augmentation tool
    # This adds FCalEtA, FCalEtC, ...
    augmentation_tool = CompFactory.DerivationFramework.HIGlobalAugmentationTool(name="HION7AugmentationTool",
                                                                                nHarmonic=5 # to capture higher-order harmonics for anisotropic flow
                                                                                )
    acc.addPublicTool(augmentation_tool, primary=True)

    return acc


def HION7KernelCfg(flags, name='HION7Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()
    
#########################################################################################
#Thinning
    from CoolConvUtilities.ParticleTypeUtil import getTypeForRun
    info=getTypeForRun(flags.Input.RunNumbers[0])
    isSmallSystem = False
    if (info.getBeam1Type() < 11) or (info.getBeam2Type() < 11):
        isSmallSystem = True
    pTCut = 20
    if isSmallSystem:
        pTCut = 15
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
         SelectionString         = "AntiKt2HIJets.pt > "+ str(pTCut) +"*GeV",
         InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    AntiKt4HIJetsThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
         flags,
         name                    = "AntiKt4HIJetsThinningTool",
         StreamName              = kwargs['StreamName'],
         JetKey                  = "AntiKt4HIJets",
         SelectionString         = "AntiKt4HIJets.pt > "+ str(pTCut) +"*GeV",
         InDetTrackParticlesKey  = "InDetTrackParticles"))
    
   
    thinningTools = [TrackParticleThinningTool,
                    AntiKt2HIJetsThinningTool,
                    AntiKt4HIJetsThinningTool]
    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import GenericTruthThinningCfg
        truth_thinning_expression = "(TruthParticles.status==1) && (TruthParticles.pt > 0.7*GeV) && (abs(TruthParticles.eta) < 2.7)"
        TruthParticleThinningTool = acc.getPrimaryAndMerge(GenericTruthThinningCfg(flags,
            name="TruthParticleThinningTool",
            StreamName=kwargs['StreamName'],
            ParticleSelectionString = truth_thinning_expression
            )
        )
        thinningTools += [TruthParticleThinningTool]
#########################################################################################
    skimmingTool = acc.getPrimaryAndMerge(HION7SkimmingToolCfg(flags))
    globalAugmentationTool = acc.getPrimaryAndMerge(HION7GlobalAugmentationToolCfg(flags))
    augmentationTool=[globalAugmentationTool]

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name,ThinningTools = thinningTools, SkimmingTools = [skimmingTool], AugmentationTools=augmentationTool),sequenceName="HION7Sequence")       
    return acc


def HION7Cfg(flags):
    
    acc = ComponentAccumulator()
    acc.merge(HION7KernelCfg(flags, name="HION7Kernel",StreamName = "StreamDAOD_HION7"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from DerivationFrameworkHI import ListSlimming
    
#########################################################################################
#Slimming
    HION7SlimmingHelper = SlimmingHelper("HION7SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    HION7SlimmingHelper.SmartCollections = ListSlimming.HION7SmartCollections()
    all_variables = ListSlimming.HION7AllVarContent()
    if flags.Input.isMC:
        all_variables += ListSlimming.HION7AllVarTruthContent()
    HION7SlimmingHelper.AllVariables = all_variables

    HION7ItemList = HION7SlimmingHelper.GetItemList()

    acc.merge(OutputStreamCfg(flags, "DAOD_HION7", ItemList=HION7ItemList, AcceptAlgs=["HION7Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HION7", AcceptAlgs=["HION7Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
