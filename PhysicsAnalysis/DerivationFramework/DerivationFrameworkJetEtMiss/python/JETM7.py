# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_JETM7.py
#====================================================================
# https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DerivationFramework

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def JETM7SkimmingToolCfg(flags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()


    jetSelection = '(count(AntiKt4EMPFlowJets.pt > 10.*GeV && abs(AntiKt4EMPFlowJets.eta) < 2.5) >= 1)'
    JETM7OfflineSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "JETM7OfflineSkimmingTool1",
                                                                                        expression = jetSelection)

    acc.addPublicTool(JETM7OfflineSkimmingTool, primary=True)

    return(acc)

# Main algorithm config
def JETM7KernelCfg(flags, name='JETM7Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM7"""
    acc = ComponentAccumulator()

    # Skimming
    if not flags.Input.isMC:
        skimmingTool = acc.getPrimaryAndMerge(JETM7SkimmingToolCfg(flags))

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    from DerivationFrameworkInDet.InDetToolsConfig import InDetTrackSelectionToolWrapperCfg
    DFCommonTrackSelection = acc.getPrimaryAndMerge(InDetTrackSelectionToolWrapperCfg(
        flags,
        name           = "DFJETM7CommonTrackSelectionLoose",
        CutLevel       = "Loose",
        DecorationName = "DFJETM7Loose"))

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation("JETM7CommonKernel", AugmentationTools = [DFCommonTrackSelection]))

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, JetTrackParticleThinningCfg
    # from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import GenericTruthThinningCfg
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import GenericObjectThinningCfg

    muonSelectionString = "(Muons.pt > 5*GeV)"
    electronSelectionString = "(Electrons.pt > 5*GeV)"
    photonSelectionString = "(Photons.pt > 5*GeV)"
    jetSelectionString = "(AntiKt4EMPFlowByVertexJets.pt > 7.*GeV && AntiKt4EMPFlowByVertexJets.Jvt > 0.4)"

    # Include inner detector tracks associated with muons
    JETM7MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(flags,
        name                    = "JETM7MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        SelectionString         = muonSelectionString,
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with electrons
    JETM7ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(flags,
        name                    = "JETM7ElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        SelectionString         = electronSelectionString,
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with by-vertex jets
    JETM7Akt4JetTPThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(flags,
        name                    = "JETM7Akt4JetTPThinningTool",
        StreamName              = kwargs['StreamName'],
        JetKey                  = "AntiKt4EMPFlowByVertexJets",
        SelectionString         = jetSelectionString,
        InDetTrackParticlesKey  = "InDetTrackParticles"))


    # Store EMPFlowByVertexJets with JVT > 0.4. This will result in jets extending up to about 2.6 in |eta|
    JETM7Akt4PFlowByVertexJetThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                                 name             = "JETM7Akt4PFlowByVertexJetThinningTool",
                                                                                 ContainerName    = "AntiKt4EMPFlowByVertexJets",
                                                                                 StreamName       = kwargs['StreamName'],
                                                                                 SelectionString  = jetSelectionString))

    # Store regular EMPFlowJets for |eta| > 2.4 to retain forward jets with a PV0 interpretation
    JETM7Akt4PFlowJetThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                                 name             = "JETM7Akt4PFlowJetThinningTool",
                                                                                 ContainerName    = "AntiKt4EMPFlowJets",
                                                                                 StreamName       = kwargs['StreamName'],
                                                                                 SelectionString  = "(abs(AntiKt4EMPFlowJets.eta) > 2.4)"))


    JETM7MuonThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                        name             = "JETM7MuonThinningTool",
                                                                        ContainerName    = "Muons",
                                                                        StreamName       = kwargs['StreamName'],
                                                                        SelectionString  = muonSelectionString))


    JETM7ElectronThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                            name             = "JETM7ElectronThinningTool",
                                                                            ContainerName    = "Electrons",
                                                                            StreamName       = kwargs['StreamName'],
                                                                            SelectionString  = electronSelectionString))


    JETM7PhotonThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(flags,
                                                                        name             = "JETM7PhotonThinningTool",
                                                                        ContainerName    = "Photons",
                                                                        StreamName       = kwargs['StreamName'],
                                                                        SelectionString  = photonSelectionString))


    # Extra jet content:
    acc.merge(JETM7ExtraContentCfg(flags))

    # Finally the kernel itself
    thinningTools = [JETM7MuonTPThinningTool,
                     JETM7ElectronTPThinningTool,
                     JETM7Akt4JetTPThinningTool,
                     JETM7Akt4PFlowByVertexJetThinningTool,
                     JETM7Akt4PFlowJetThinningTool,
                     JETM7MuonThinningTool,
                     JETM7ElectronThinningTool,
                     JETM7PhotonThinningTool,
                     ]

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, 
                                      ThinningTools = thinningTools,
                                      SkimmingTools =[skimmingTool] if not flags.Input.isMC else []))   


    return acc

def JETM7ExtraContentCfg(flags):

    acc = ComponentAccumulator()

    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.StandardSmallRJets import AntiKt4PV0Track, AntiKt4EMPFlowByVertex 

    #======================================= 
    # R = 0.4 track-jets (needed for Rtrk) 
    #=======================================
    jetList = [AntiKt4PV0Track]

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

def JETM7Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM7TriggerListsHelper = TriggerListsHelper(flags)

    # Skimming, thinning, augmentation, extra content
    acc.merge(JETM7KernelCfg(flags, name="JETM7Kernel", StreamName = 'StreamDAOD_JETM7', TriggerListsHelper = JETM7TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM7SlimmingHelper = SlimmingHelper("JETM7SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    JETM7SlimmingHelper.SmartCollections = ["Electrons", "Photons", "Muons",
                                            "PrimaryVertices",
                                            "InDetTrackParticles",
                                            "AntiKt4EMPFlowJets",
                                            "EventInfo"
                                            ]

    
    JETM7SlimmingHelper.ExtraVariables = ["InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights"]
    JETM7SlimmingHelper.AllVariables = ["BTagging_AntiKt4EMPFlow","BTagging_AntiKt4EMPFlowByVertex"] #ByVertex"]
    # Truth containers
    if flags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(JETM7SlimmingHelper)

        JETM7SlimmingHelper.AppendToDictionary.update({'TruthParticles': 'xAOD::TruthParticleContainer',
                                                       'TruthParticlesAux': 'xAOD::TruthParticleAuxContainer'})

        JETM7SlimmingHelper.SmartCollections += ["AntiKt4TruthWZJets"]

        JETM7SlimmingHelper.AllVariables += ["TruthTopQuarkWithDecayParticles","TruthTopQuarkWithDecayVertices",
                                             "AntiKt4TruthJets", "InTimeAntiKt4TruthJets", "OutOfTimeAntiKt4TruthJets", "TruthParticles",
                                             "TruthVertices", "TruthEvents"]
        JETM7SlimmingHelper.ExtraVariables += ["InDetTrackParticles.truthMatchProbability", "TruthVertices.barcode.z"]


    # Trigger content
    JETM7SlimmingHelper.IncludeTriggerNavigation = True
    JETM7SlimmingHelper.IncludeJetTriggerContent = False
    JETM7SlimmingHelper.IncludeMuonTriggerContent = False
    JETM7SlimmingHelper.IncludeEGammaTriggerContent = False
    JETM7SlimmingHelper.IncludeTauTriggerContent = False
    JETM7SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM7SlimmingHelper.IncludeBJetTriggerContent = False
    JETM7SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM7SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM7SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = JETM7TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM7SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = JETM7TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(JETM7SlimmingHelper)

    jetOutputList = ["AntiKt4EMPFlowByVertexJets"]
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addJetsToSlimmingTool
    addJetsToSlimmingTool(JETM7SlimmingHelper, jetOutputList, JETM7SlimmingHelper.SmartCollections)
    
    # Flavour tagging
    from DerivationFrameworkFlavourTag.FtagDerivationConfig import FtagJetCollectionsCfg
    acc.merge(FtagJetCollectionsCfg(flags,jetOutputList))
    
    # Output stream 
    JETM7ItemList = JETM7SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_JETM7", ItemList=JETM7ItemList, AcceptAlgs=["JETM7Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_JETM7", AcceptAlgs=["JETM7Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

