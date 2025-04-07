# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_FTAG_PU.py
# This defines DAOD_FTAG_PU, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority 
# of physics analyses in ATLAS.
# It requires the flag FTAG_PU in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from DerivationFrameworkFlavourTag.FtagBaseContent import (
    addCommonAugmentation
)
#skimming tool
def FTAG_PUSkimmingToolCfg(flags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()


    jetSelection = '(count(AntiKt4EMPFlowJets.pt > 10.*GeV && abs(AntiKt4EMPFlowJets.eta) < 2.5) >= 1)'
    FTAG_PUOfflineSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "FTAG_PUOfflineSkimmingTool1",
                                                                                        expression = jetSelection)

    acc.addPublicTool(FTAG_PUOfflineSkimmingTool, primary=True)

    return(acc)

# Main algorithm config
def FTAG_PUKernelCfg(flags, name='FTAG_PUKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for FTAG_PU"""
    acc = ComponentAccumulator()
    # Skimming
    skimmingTools = []
    if not flags.Input.isMC:
        skimmingTools = [acc.getPrimaryAndMerge(FTAG_PUSkimmingToolCfg(flags)),]

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))
    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, JetTrackParticleThinningCfg
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import GenericObjectThinningCfg

    muonSelectionString = "(Muons.pt > 5*GeV)"
    electronSelectionString = "(Electrons.pt > 5*GeV)"
    photonSelectionString = "(Photons.pt > 5*GeV)"
    jetSelectionString = "(AntiKt4EMPFlowByVertexJets.pt > 7.*GeV && AntiKt4EMPFlowByVertexJets.Jvt > 0.4)"

    # Include inner detector tracks associated with muons
    FTAG_PUMuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(flags,
        name                    = "FTAG_PUMuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        SelectionString         = muonSelectionString,
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with electrons
    FTAG_PUElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(flags,
        name                    = "FTAG_PUElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        SelectionString         = electronSelectionString,
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with by-vertex jets
    FTAG_PUAkt4JetTPThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(flags,
        name                    = "FTAG_PUAkt4JetTPThinningTool",
        StreamName              = kwargs['StreamName'],
        JetKey                  = "AntiKt4EMPFlowByVertexJets",
        SelectionString         = jetSelectionString,
        InDetTrackParticlesKey  = "InDetTrackParticles"))


    # Store EMPFlowByVertexJets with JVT > 0.4. This will result in jets extending up to about 2.6 in |eta|
    FTAG_PUAkt4PFlowByVertexJetThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                                 name             = "FTAG_PUAkt4PFlowByVertexJetThinningTool",
                                                                                 ContainerName    = "AntiKt4EMPFlowByVertexJets",
                                                                                 StreamName       = kwargs['StreamName'],
                                                                                 SelectionString  = jetSelectionString))

    # TrackParticles associated with small-R jets
    FTAG_PUAkt4PFlowJetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(flags,
        name            = "FTAG2Akt4PFlowJetTPThinningTool",
        StreamName      = kwargs['StreamName'],
        JetKey   = "AntiKt4EMPFlowJets",
        SelectionString = 'AntiKt4EMPFlowJets.pt > 15*GeV',
        InDetTrackParticlesKey  = "InDetTrackParticles"))


    FTAG_PUMuonThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                        name             = "FTAG_PUMuonThinningTool",
                                                                        ContainerName    = "Muons",
                                                                        StreamName       = kwargs['StreamName'],
                                                                        SelectionString  = muonSelectionString))


    FTAG_PUElectronThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                            name             = "FTAG_PUElectronThinningTool",
                                                                            ContainerName    = "Electrons",
                                                                            StreamName       = kwargs['StreamName'],
                                                                            SelectionString  = electronSelectionString))


    FTAG_PUPhotonThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                        name             = "FTAG_PUPhotonThinningTool",
                                                                        ContainerName    = "Photons",
                                                                        StreamName       = kwargs['StreamName'],
                                                                        SelectionString  = photonSelectionString))


    # Extra jet content:
    acc.merge(FTAG_PUExtraContentCfg(flags))

    # Finally the kernel itself
    thinningTools = [FTAG_PUMuonTPThinningTool,
                     FTAG_PUElectronTPThinningTool,
                     FTAG_PUAkt4JetTPThinningTool,
                     FTAG_PUAkt4PFlowByVertexJetThinningTool,
                     FTAG_PUAkt4PFlowJetTPThinningTool,
                     FTAG_PUMuonThinningTool,
                     FTAG_PUElectronThinningTool,
                     FTAG_PUPhotonThinningTool,
                     ]
    

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, SkimmingTools = skimmingTools, ThinningTools = thinningTools))       
    
    # Extra jet content:
    acc.merge(FTAG_PUExtraContentCfg(flags))
    
    return acc


def FTAG_PUCfg(flags):
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    FTAG_PUTriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(FTAG_PUKernelCfg(flags, name="FTAG_PUKernel", StreamName = 'StreamDAOD_FTAG_PU', TriggerListsHelper = FTAG_PUTriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    FTAG_PUSlimmingHelper = SlimmingHelper("FTAG_PUSlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    from DerivationFrameworkFlavourTag import FtagBaseContent

    addCommonAugmentation(flags, acc, FTAG_PUSlimmingHelper)

    FTAG_PUSlimmingHelper.SmartCollections = ["AntiKt4EMPFlowJets",
                                            "AntiKt4TruthJets",
                                            ]
    #FtagBaseContent.add_baseline_slimming_smartcollections(FTAG_PUSlimmingHelper)
    
    FTAG_PUSlimmingHelper.AllVariables = ["Electrons", "Photons", "Muons",
                                          "EventInfo",
                                          "PrimaryVertices",
                                          "InDetTrackParticles",
                                        ]
    
    # Add truth containers
    if flags.Input.isMC:
        FtagBaseContent.add_truth_to_SlimmingHelper(FTAG_PUSlimmingHelper)
        if flags.Trigger.EDMVersion == 3:
            # Add truth labels to Run 3 trigger jets
            from DerivationFrameworkFlavourTag.FtagDerivationConfig import HLTJetFTagDecorationCfg
            acc.merge(HLTJetFTagDecorationCfg(flags))

   
    # Trigger content
    FTAG_PUSlimmingHelper.IncludeTriggerNavigation = True
    FTAG_PUSlimmingHelper.IncludeJetTriggerContent = False
    FTAG_PUSlimmingHelper.IncludeMuonTriggerContent = True
    FTAG_PUSlimmingHelper.IncludeEGammaTriggerContent = True
    FTAG_PUSlimmingHelper.IncludeTauTriggerContent = False
    FTAG_PUSlimmingHelper.IncludeEtMissTriggerContent = False
    FTAG_PUSlimmingHelper.IncludeBJetTriggerContent = False
    FTAG_PUSlimmingHelper.IncludeBPhysTriggerContent = False
    FTAG_PUSlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        #AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = FTAG_PUSlimmingHelper, 
         #                                      OutputContainerPrefix = "TrigMatch_", 
          #                                     TriggerList = FTAG_PUTriggerListsHelper.Run2TriggerNamesTau)
        # This was adding the tau triggers even though its False
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = FTAG_PUSlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = FTAG_PUTriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(FTAG_PUSlimmingHelper)
    


    jetOutputList = ["AntiKt4EMPFlowByVertexJets"]
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addJetsToSlimmingTool
    addJetsToSlimmingTool(FTAG_PUSlimmingHelper, jetOutputList, FTAG_PUSlimmingHelper.SmartCollections)

    # Flavour tagging 
    from DerivationFrameworkFlavourTag.FtagDerivationConfig import JetCollectionsBTaggingCfg
    acc.merge(JetCollectionsBTaggingCfg(flags, ["AntiKt4EMPFlowByVertexJets"], ByVertex=True, dzCut_vec=[5, 4], useMinZ0Vertex_vec=[True,False]))
    acc.merge(JetCollectionsBTaggingCfg(flags, ["AntiKt4EMPFlowJets"]))

    # Output stream
    FTAG_PUItemList = FTAG_PUSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_FTAG_PU", ItemList=FTAG_PUItemList, AcceptAlgs=["FTAG_PUKernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_FTAG_PU", AcceptAlgs=["FTAG_PUKernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

def FTAG_PUExtraContentCfg(flags):
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