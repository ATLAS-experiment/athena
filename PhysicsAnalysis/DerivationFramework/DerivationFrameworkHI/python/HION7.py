# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# HION7.py 

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND

#########################################################################################
# in place of common PhysCommonAugmentations:
def PhysAugmentationsHION7Cfg(flags):

    """Configure the physics augmentation for HION7"""
    acc = ComponentAccumulator()

    # MC truth
    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import (
            AddStandardTruthContentsCfg,
            AddHFAndDownstreamParticlesCfg,
            AddMiniTruthCollectionLinksCfg,
            AddPVCollectionCfg)
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import TruthCollectionMakerCfg
        PhysCommonTruthCharmTool = acc.getPrimaryAndMerge(TruthCollectionMakerCfg(
            flags,
            name                    = "PhysCommonTruthCharmTool",
            NewCollectionName       = "TruthCharm",
            KeepNavigationInfo      = False,
            ParticleSelectionString = "(abs(TruthParticles.pdgId) == 4)",
            Do_Compress             = True))
        CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
        acc.addEventAlgo(CommonAugmentation("PhysCommonTruthCharmKernel",AugmentationTools=[PhysCommonTruthCharmTool]))
        acc.merge(AddHFAndDownstreamParticlesCfg(flags))
        acc.merge(AddStandardTruthContentsCfg(
                  flags,
                  navInputCollections =["TruthElectrons",
                                        "TruthMuons",
                                        "TruthPhotons",
                                        "TruthTaus",
                                        "TruthNeutrinos",
                                        "TruthBSM",
                                        "TruthBottom",
                                        "TruthTop",
                                        "TruthBoson",
                                        "TruthCharm",
                                        "TruthHFWithDecayParticles"]))
        # Re-point links on reco objects
        acc.merge(AddMiniTruthCollectionLinksCfg(flags))
        acc.merge(AddPVCollectionCfg(flags))
    # InDet, Muon, Egamma common augmentations
    from DerivationFrameworkInDet.InDetCommonConfig import InDetCommonCfg
    from DerivationFrameworkMuons.MuonsCommonConfig import MuonsCommonCfg
    from DerivationFrameworkEGamma.EGammaCommonConfig import EGammaCommonCfg
    acc.merge(InDetCommonCfg(flags,
                             DoVertexFinding = flags.Tracking.doVertexFinding,
                             AddPseudoTracks = flags.Tracking.doPseudoTracking,
                             DecoLRTTTVA = False,
                             DoR3LargeD0 = flags.Tracking.doLargeD0,
                             StoreSeparateLargeD0Container = flags.Tracking.storeSeparateLargeD0Container,
                             MergeLRT = False))
    acc.merge(MuonsCommonCfg(flags))
    acc.merge(EGammaCommonCfg(flags))

    return acc

def getDFJets(flags):
    """ Create updated version of AntiKt*HIJets"""
    acc = ComponentAccumulator()

    JetColl = flags.HeavyIon.HIJetPrefix
    from HIJetRec.HIJetRecConfigCA import HIJetRecCfg
    acc.merge(HIJetRecCfg(flags))
    if flags.HeavyIon.doHIBTagging:
        from BTagging.FlavorTaggingConfig import FlavorTaggingCfg
        acc.merge(FlavorTaggingCfg(flags, JetColl+"AntiKt4HIJets"))
        from BTagging.TrackLeptonConfig import TrackLeptonDecorationCfg
        acc.merge(TrackLeptonDecorationCfg(flags))

    return acc


#########################################################################################
#Skiming
def HION7SkimmingToolCfg(flags):
    """Configure the example skimming tool"""
    acc = ComponentAccumulator()
    JetColl = flags.HeavyIon.HIJetPrefix
    ExtraData  = []
    ExtraData += ['xAOD::JetContainer/'+JetColl+'AntiKt2HIJets']
    ExtraData += ['xAOD::JetContainer/'+JetColl+'AntiKt4HIJets']

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
            expression = expression + '(' + key + ' && count('+JetColl+'AntiKt4HIJets.pt >' + str(TriggerDict[key]) + '*GeV) >=1 ) ' + '|| (' + key + ' && count('+JetColl+'AntiKt2HIJets.pt >' + str(TriggerDict[key]) + '*GeV) >=1 ) '
            if not i == len(TriggerDict) - 1:
                expression = expression + ' || '
    else:
        expression = expression + 'count('+JetColl+'AntiKt2HIJets.pt > 15000) > 1 || count('+JetColl+'AntiKt4HIJets.pt > 15000) > 1'

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
    doTopoClus = True

    from AthenaConfiguration.Enums import HIMode
    if flags.Reco.HIMode == HIMode.HI:
        doTopoClus = False

    augmentation_tool = CompFactory.DerivationFramework.HIGlobalAugmentationTool(name="HION7AugmentationTool",
                                                                                nHarmonic=5, # to capture higher-order harmonics for anisotropic flow
                                                                                doTopoClusDec = doTopoClus
                                                                                )

    acc.addPublicTool(augmentation_tool, primary=True)

    return acc


def HION7KernelCfg(flags, name='HION7Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()

    acc.merge(PhysAugmentationsHION7Cfg(flags))
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

    JetColl = flags.HeavyIon.HIJetPrefix

    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg,JetTrackParticleThinningCfg
    
    minTrackPt = flags.HeavyIon.MinTrackPt
    track_thinning_expression  = "InDetTrackParticles.pt > "+str(minTrackPt)+"*GeV"
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
         JetKey                  = JetColl+"AntiKt2HIJets",
         SelectionString         = JetColl+"AntiKt2HIJets.pt > "+ str(pTCut) +"*GeV",
         InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    AntiKt4HIJetsThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
         flags,
         name                    = "AntiKt4HIJetsThinningTool",
         StreamName              = kwargs['StreamName'],
         JetKey                  = JetColl+"AntiKt4HIJets",
         SelectionString         = JetColl+"AntiKt4HIJets.pt > "+ str(pTCut) +"*GeV",
         InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    thinningTools = [TrackParticleThinningTool,
                    AntiKt2HIJetsThinningTool,
                    AntiKt4HIJetsThinningTool]
    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import GenericTruthThinningCfg
        truth_thinning_expression = "(TruthParticles.status==1) && (TruthParticles.pt > "+str(minTrackPt-0.2)+"*GeV) && (abs(TruthParticles.eta) < 2.7)"
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

    JetColl = flags.HeavyIon.HIJetPrefix
    acc.merge(getDFJets(flags))

    acc.merge(HION7KernelCfg(flags, name="HION7Kernel",StreamName = "StreamDAOD_HION7"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from DerivationFrameworkHI import ListSlimming
    
#########################################################################################
#Slimming
    HION7SlimmingHelper = SlimmingHelper("HION7SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    HION7SlimmingHelper.SmartCollections = ListSlimming.HION7SmartCollections()
    AllVars = ListSlimming.HION7AllVarContent()
    AllVars += ListSlimming.HION7ExtraContainersTrigger()
    ExtraVars = ListSlimming.HION7BasicJetVars(JetColl)
    from DerivationFrameworkFlavourTag import FtagBaseContent
    if flags.Input.isMC:
        AllVars += ListSlimming.HION7AllVarTruthContent()
        if flags.HeavyIon.doHIBTagging:
            FtagBaseContent.add_truth_to_SlimmingHelper(HION7SlimmingHelper)
    if flags.HeavyIon.doHIBTagging:
        from DerivationFrameworkFlavourTag.FtagBaseContent import addCommonAugmentation
        addCommonAugmentation(flags, acc, HION7SlimmingHelper, JetColl+"AntiKt4HIJets")
        AllVars += ListSlimming.HION7AllVarFromFTAG1()
        # update AppendToDictionary
        extra_AppendToDictionary = {}
        FtagBaseContent.update_AppendToDictionary_in_SlimmingHelper(HION7SlimmingHelper, flags, extra_AppendToDictionary)
        # Add ExtraVariables
        ExtraVars += ListSlimming.HION7ExtraVarForBtag(JetColl)
        FtagBaseContent.add_ExtraVariables_to_SlimmingHelper(HION7SlimmingHelper, flags)

    HION7SlimmingHelper.ExtraVariables = ExtraVars
    HION7SlimmingHelper.AllVariables = AllVars

    HION7ItemList  = HION7SlimmingHelper.GetItemList()
    HIJetRemovedBranches=ListSlimming.makeHIJetRemovedBranchList()
    jet_var_str = '.-'.join ([''] + HIJetRemovedBranches)
    jetRlist = flags.HeavyIon.Jet.RValues #Default [0.2,0.4]
    for jetR in jetRlist:
        output = ["xAOD::JetContainer#"+JetColl+"AntiKt"+str(jetR)+"HIJets",
                "xAOD::JetAuxContainer#"+JetColl+"AntiKt"+str(jetR)+"HIJetsAux.-PseudoJet"+jet_var_str]
        HION7ItemList += output

    acc.merge(OutputStreamCfg(flags, "DAOD_HION7", ItemList=HION7ItemList, AcceptAlgs=["HION7Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HION7", AcceptAlgs=["HION7Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
