# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_FTAGPU.py
# This defines DAOD_FTAGPU, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority 
# of physics analyses in ATLAS.
# It requires the flag FTAGPU in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from DerivationFrameworkFlavourTag.FtagBaseContent import (
    addCommonAugmentation
)
#skimming tool
def FTAGPUSkimmingToolCfg(flags):
    """Configure the skimming tool"""
    jetSelection = '(count(AntiKt4EMPFlowJets.pt > 10.*GeV && abs(AntiKt4EMPFlowJets.eta) < 2.5) >= 1)'
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)
    return xAODStringSkimmingToolCfg(flags, name = "FTAGPUOfflineSkimmingTool1",
                                     expression = jetSelection)

# Main algorithm config
def FTAGPUKernelCfg(flags, name='FTAGPUKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for FTAGPU"""
    acc = ComponentAccumulator()
    # Skimming
    skimmingTools = []
    if not flags.Input.isMC:
        skimmingTools = [acc.getPrimaryAndMerge(FTAGPUSkimmingToolCfg(flags)),]

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))
    # Thinning tools...
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import GenericObjectThinningCfg

    muonSelectionString = "(Muons.pt > 5*GeV)"
    electronSelectionString = "(Electrons.pt > 5*GeV)"
    photonSelectionString = "(Photons.pt > 5*GeV)"
    jetSelectionString = "(AntiKt4EMPFlowByVertexJets.pt > 7.*GeV && AntiKt4EMPFlowByVertexJets.Jvt > 0.4)"

    # Store EMPFlowByVertexJets with JVT > 0.4. This will result in jets extending up to about 2.6 in |eta|
    FTAGPUAkt4PFlowByVertexJetThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                                 name             = "FTAGPUAkt4PFlowByVertexJetThinningTool",
                                                                                 ContainerName    = "AntiKt4EMPFlowByVertexJets",
                                                                                 StreamName       = kwargs['StreamName'],
                                                                                 SelectionString  = jetSelectionString))

    FTAGPUMuonThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                        name             = "FTAGPUMuonThinningTool",
                                                                        ContainerName    = "Muons",
                                                                        StreamName       = kwargs['StreamName'],
                                                                        SelectionString  = muonSelectionString))


    FTAGPUElectronThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                            name             = "FTAGPUElectronThinningTool",
                                                                            ContainerName    = "Electrons",
                                                                            StreamName       = kwargs['StreamName'],
                                                                            SelectionString  = electronSelectionString))


    FTAGPUPhotonThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                        name             = "FTAGPUPhotonThinningTool",
                                                                        ContainerName    = "Photons",
                                                                        StreamName       = kwargs['StreamName'],
                                                                        SelectionString  = photonSelectionString))


    # Extra jet content:
    acc.merge(FTAGPUExtraContentCfg(flags))

    # Finally the kernel itself
    thinningTools = [FTAGPUAkt4PFlowByVertexJetThinningTool,
                     FTAGPUMuonThinningTool,
                     FTAGPUElectronThinningTool,
                     FTAGPUPhotonThinningTool,
                     ]
    

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, SkimmingTools = skimmingTools, ThinningTools = thinningTools))       
    
    # Extra jet content:
    acc.merge(FTAGPUExtraContentCfg(flags))
    
    return acc


def FTAGPUCfg(flags):
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    FTAGPUTriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(FTAGPUKernelCfg(flags, name="FTAGPUKernel", StreamName = 'StreamDAOD_FTAGPU', TriggerListsHelper = FTAGPUTriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    FTAGPUSlimmingHelper = SlimmingHelper("FTAGPUSlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    from DerivationFrameworkFlavourTag import FtagBaseContent

    addCommonAugmentation(flags, acc, FTAGPUSlimmingHelper)
    FTAGPUSlimmingHelper.AppendToDictionary.update({
    "DuplicatedTrks"      : "xAOD::TrackParticleContainer",
    "DuplicatedTrksAux."  : "xAOD::TrackParticleAuxContainer",
    })
    FTAGPUSlimmingHelper.SmartCollections = ["AntiKt4EMPFlowJets",
                                            "AntiKt4TruthJets",
                                            ]
    #FtagBaseContent.add_baseline_slimming_smartcollections(FTAGPUSlimmingHelper)
    
    FTAGPUSlimmingHelper.AllVariables = ["Electrons", "Photons", "Muons",
                                          "EventInfo",
                                          "PrimaryVertices",
                                          "InDetTrackParticles",
                                          "TruthParticles",
                                          "TruthVertices",
                                          "TruthEvents",
                                          "TruthBottom", "TruthElectrons","TruthMuons","TruthTaus",
                                          "JetAssociatedPixelClusters",
                                          "JetAssociatedSCTClusters",
                                          "PixelClusters",
                                          "SCT_Clusters",
                                          "DuplicatedTrks",]
    
    FTAGPUSlimmingHelper.ExtraVariables = ["TruthPrimaryVertices.t.x.y.z",
                                            "Electrons.TruthLink",
                                            "Muons.TruthLink.segmentDeltaPhi.segmentDeltaEta.ParamEnergyLoss.ParamEnergyLossSigmaPlus.ParamEnergyLossSigmaMinus.MeasEnergyLoss.MeasEnergyLossSigma",
                                            "Photons.TruthLink",
                                            "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.DFCommonJets_fJvt.GhostBHadronsFinalPt.SumPtChargedPFOPt1000.SumPtTrkPt1000.TrackSumMass.TrackSumPt.TrackWidthPt500.TracksForBTagging.JetEMScaleMomentum_pt.JetEMScaleMomentum_eta.HECQuality.GhostHBosonsPt.GNNVerticesLink.InclusiveGNNVerticesLink",
                                            "TruthEvents.signalProcessVertexLink",
                                            ]
    FTAGPUSlimmingHelper.StaticContent += ["xAOD::TrackParticleContainer#DuplicatedTrks","xAOD::TrackParticleAuxContainer#DuplicatedTrksAux."]
    # Add truth containers
    if flags.Input.isMC:
        FtagBaseContent.add_truth_to_SlimmingHelper(FTAGPUSlimmingHelper)
        if flags.Trigger.EDMVersion == 3:
            # Add truth labels to Run 3 trigger jets
            from DerivationFrameworkFlavourTag.FtagDerivationConfig import HLTJetFTagDecorationCfg
            acc.merge(HLTJetFTagDecorationCfg(flags))

   
    # Trigger content
    FTAGPUSlimmingHelper.IncludeTriggerNavigation = True
    FTAGPUSlimmingHelper.IncludeJetTriggerContent = False
    FTAGPUSlimmingHelper.IncludeMuonTriggerContent = True
    FTAGPUSlimmingHelper.IncludeEGammaTriggerContent = True
    FTAGPUSlimmingHelper.IncludeTauTriggerContent = False
    FTAGPUSlimmingHelper.IncludeEtMissTriggerContent = False
    FTAGPUSlimmingHelper.IncludeBJetTriggerContent = False
    FTAGPUSlimmingHelper.IncludeBPhysTriggerContent = False
    FTAGPUSlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = FTAGPUSlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = FTAGPUTriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(FTAGPUSlimmingHelper)
    


    jetOutputList = ["AntiKt4EMPFlowByVertexJets"]
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addJetsToSlimmingTool
    addJetsToSlimmingTool(FTAGPUSlimmingHelper, jetOutputList, FTAGPUSlimmingHelper.SmartCollections)

    # Flavour tagging
    from BTagging.FlavorTaggingConfig import JetBTagginglessByVertexAlgCfg
    acc.merge(JetBTagginglessByVertexAlgCfg(
        flags,
        "AntiKt4EMPFlowByVertexJets",
        dzCut_vec=[5]))

    # Output stream
    FTAGPUItemList = FTAGPUSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_FTAGPU", ItemList=FTAGPUItemList, AcceptAlgs=["FTAGPUKernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_FTAGPU", AcceptAlgs=["FTAGPUKernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

def FTAGPUExtraContentCfg(flags):
    acc = ComponentAccumulator()

    from JetRecConfig.JetRecConfig import JetRecCfg
    jetList = []
    #=======================================
    # CSSK R = 0.4 UFO jets
    #=======================================
    from JetRecConfig.StandardSmallRJets import AntiKt4UFOCSSK
    jetList += [AntiKt4UFOCSSK]

    from JetRecConfig.StandardSmallRJets import AntiKt4PV0Track, AntiKt4EMPFlowByVertex 

    #======================================= 
    # R = 0.4 track-jets (needed for Rtrk) 
    #=======================================
    jetList += [AntiKt4PV0Track]

    #======================================= 
    # R = 0.4 by-vertex jets 
    #=======================================
    jetList += [AntiKt4EMPFlowByVertex]

    for jd in jetList:
        acc.merge(JetRecCfg(flags,jd))

    #=======================================
    # More detailed truth information
    #=======================================

    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTopQuarkAndDownstreamParticlesCfg
        acc.merge(AddTopQuarkAndDownstreamParticlesCfg(flags, generations=4, rejectHadronChildren=True))

    #=======================================
    # Add Run-2 jet trigger collections
    # Only needed for Run-2 due to different aux container type (JetTrigAuxContainer) which required special wrapper for conversion to AuxContainerBase
    # In Run-3, the aux. container type is directly JetAuxContainer (no conversion needed)
    #=======================================

    if flags.Trigger.EDMVersion == 2:
        triggerNames = ["JetContainer_a4tcemsubjesFS", "JetContainer_a4tcemsubjesISFS", "JetContainer_a10tclcwsubjesFS", "JetContainer_GSCJet"]

        for trigger in triggerNames:
            wrapperName = trigger+'AuxWrapper'
            auxContainerName = 'HLT_xAOD__'+trigger+'Aux'

            acc.addEventAlgo(CompFactory.xAODMaker.AuxStoreWrapper( wrapperName, SGKeys = [ auxContainerName+"." ] ))

    return acc
