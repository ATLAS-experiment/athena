# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_JETM4.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def JETM4SkimmingToolCfg(flags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    from DerivationFrameworkJetEtMiss import TriggerLists
    triggerlist = TriggerLists.single_photon_Trig(flags)
    addRun3PhotonTriggers = ["HLT_g140_loose_L1EM22VHI","HLT_g300_etcut_L1EM22VHI"]
    triggerlist = triggerlist+addRun3PhotonTriggers

    triggers = '||'.join(triggerlist)

    JETM4SkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "JETM4SkimmingTool",
                                                                               expression = triggers)
    acc.addPublicTool(JETM4SkimmingTool, primary = True)

    return(acc)


# Main algorithm config
def JETM4KernelCfg(flags, name='JETM4Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM4"""
    acc = ComponentAccumulator()

    # Skimming
    if not flags.Input.isMC:
        skimmingTool = acc.getPrimaryAndMerge(JETM4SkimmingToolCfg(flags))

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg, MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, TauTrackParticleThinningCfg

    # https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DaodRecommendations
    JETM4_thinning_expression = "( abs(InDetTrackParticles.d0) < 5*mm ) && ( abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5*mm )"
    JETM4TrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        flags,
        name                    = "JETM4TrackParticleThinningTool",
        StreamName              = kwargs['StreamName'], 
        SelectionString         = JETM4_thinning_expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with muons
    JETM4MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "JETM4MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with electonrs
    JETM4ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                    = "JETM4ElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with photons
    JETM4PhotonTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                     = "JETM4PhotonTPThinningTool",
        StreamName               = kwargs['StreamName'],
        SGKey                    = "Photons",
        InDetTrackParticlesKey   = "InDetTrackParticles",
        GSFConversionVerticesKey = "GSFConversionVertices"))

    # Include inner detector tracks associated with taus
    JETM4TauTPThinningTool = acc.getPrimaryAndMerge(TauTrackParticleThinningCfg(
        flags,
        name                   = "JETM4TauTPThinningTool",
        StreamName             = kwargs['StreamName'],
        TauKey                 = "TauJets",
        InDetTrackParticlesKey = "InDetTrackParticles",
        DoTauTracksThinning    = True,
        TauTracksKey           = "TauTracks"))


    thinningTools = [JETM4TrackParticleThinningTool,
                     JETM4MuonTPThinningTool,
                     JETM4ElectronTPThinningTool,
                     JETM4PhotonTPThinningTool,
                     JETM4TauTPThinningTool]

    if flags.Input.isMC:
        truth_cond_WZH    = "((abs(TruthParticles.pdgId) >= 23) && (abs(TruthParticles.pdgId) <= 25))"                                      # W, Z and Higgs
        truth_cond_Lepton = "((abs(TruthParticles.pdgId) >= 11) && (abs(TruthParticles.pdgId) <= 16) && (TruthParticles.barcode < 200000))" # Leptons
        truth_cond_Quark  = "((abs(TruthParticles.pdgId) <=  5 && (TruthParticles.pt > 10000.)) || (abs(TruthParticles.pdgId) == 6))"       # Quarks
        truth_cond_Gluon  = "((abs(TruthParticles.pdgId) == 21) && (TruthParticles.pt > 10000.))"                                           # Gluons
        truth_cond_Photon = "((abs(TruthParticles.pdgId) == 22) && (TruthParticles.pt > 10000.) && (TruthParticles.barcode < 200000))"      # Photon

        truth_expression = '('+truth_cond_WZH+' || '+truth_cond_Lepton +' || '+truth_cond_Quark+'||'+truth_cond_Gluon+' || '+truth_cond_Photon+')'

        preserveAllDescendants = False

        JETM4TruthThinningTool = CompFactory.DerivationFramework.GenericTruthThinning ( name = "JETM4TruthThinningTool",
                                                                                        StreamName              = kwargs['StreamName'],
                                                                                        ParticleSelectionString = truth_expression,
                                                                                        PreserveDescendants     = preserveAllDescendants,
                                                                                        PreserveGeneratorDescendants = not preserveAllDescendants,
                                                                                        PreserveAncestors = True)

        acc.addPublicTool(JETM4TruthThinningTool)
        thinningTools.append(JETM4TruthThinningTool)

    # Finally the kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, 
                                      ThinningTools = thinningTools,
                                      SkimmingTools = [skimmingTool] if not flags.Input.isMC else []))
    
    # Extra jet content:
    acc.merge(JETM4ExtraContentCfg(flags))

    return acc


def JETM4ExtraContentCfg(flags):

    acc = ComponentAccumulator()

    # PFlow augmentation tool
    from DerivationFrameworkJetEtMiss.PFlowCommonConfig import PFlowCommonCfg
    acc.merge(PFlowCommonCfg(flags))

    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.StandardSmallRJets import AntiKt4UFOCSSKLowPt
    AntiKt4UFOCSSKLowPt_JETM4 = AntiKt4UFOCSSKLowPt.clone(
        modifiers = AntiKt4UFOCSSKLowPt.modifiers+("NNJVT",)
    )
    jetList = [AntiKt4UFOCSSKLowPt_JETM4]
    for jd in jetList:
        acc.merge(JetRecCfg(flags,jd))

    return acc


def JETM4Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM4TriggerListsHelper = TriggerListsHelper(flags)

    # Skimming, thinning, augmentation, extra content
    acc.merge(JETM4KernelCfg(flags, name="JETM4Kernel", StreamName = 'StreamDAOD_JETM4', TriggerListsHelper = JETM4TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM4SlimmingHelper = SlimmingHelper("JETM4SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    JETM4SlimmingHelper.SmartCollections = ["EventInfo","InDetTrackParticles", "PrimaryVertices",
                                            "Electrons", "Photons", "Muons", "TauJets",
                                            "MET_Baseline_AntiKt4EMTopo","MET_Baseline_AntiKt4EMPFlow",
                                            "AntiKt4EMPFlowJets","AntiKt4EMTopoJets","AntiKt4UFOCSSKLowPtJets",
                                            "AntiKt10UFOCSSKJets",
                                            "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                            "AntiKtVR30Rmax4Rmin02PV0TrackJets",
                                            "BTagging_AntiKt4EMPFlow",
                                            "BTagging_AntiKtVR30Rmax4Rmin02Track"]

    JETM4SlimmingHelper.AllVariables = ["CHSGChargedParticleFlowObjects", "CHSGNeutralParticleFlowObjects",
                                        "MuonSegments",
                                        "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape","Kt4EMPFlowNeutEventShape"]

    JETM4SlimmingHelper.ExtraVariables = ["TauJets.truthJetLink.truthParticleLink.IsTruthMatched"]

    # Low-level variables
    from DerivationFrameworkJetEtMiss.CommonJETMXContent import ClusterVariables, FlowElementVariables
    JETM4SlimmingHelper.ExtraVariables += [".".join(["CaloCalTopoClusters"] + ClusterVariables)]
    JETM4SlimmingHelper.ExtraVariables += [".".join(["GlobalChargedParticleFlowObjects"] + FlowElementVariables)]
    JETM4SlimmingHelper.ExtraVariables += [".".join(["GlobalNeutralParticleFlowObjects"] + FlowElementVariables)]

    # Truth containers
    if flags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(JETM4SlimmingHelper)

        JETM4SlimmingHelper.AppendToDictionary.update({'TruthParticles': 'xAOD::TruthParticleContainer',
                                                       'TruthParticlesAux': 'xAOD::TruthParticleAuxContainer'})
        
        JETM4SlimmingHelper.AllVariables += ["TruthParticles", "TruthEvents", "TruthVertices", 
                                             "MuonTruthParticles", "egammaTruthParticles",]
        JETM4SlimmingHelper.SmartCollections += ["AntiKt4TruthJets","AntiKt10TruthJets",
                                                 "AntiKt10TruthSoftDropBeta100Zcut10Jets"]

    # Trigger content
    JETM4SlimmingHelper.IncludeTriggerNavigation = False
    JETM4SlimmingHelper.IncludeJetTriggerContent = False
    JETM4SlimmingHelper.IncludeMuonTriggerContent = False
    JETM4SlimmingHelper.IncludeEGammaTriggerContent = True
    JETM4SlimmingHelper.IncludeTauTriggerContent = False
    JETM4SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM4SlimmingHelper.IncludeBJetTriggerContent = False
    JETM4SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM4SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM4SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = JETM4TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM4SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = JETM4TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(JETM4SlimmingHelper)

    # Output stream    
    JETM4ItemList = JETM4SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_JETM4", ItemList=JETM4ItemList, AcceptAlgs=["JETM4Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_JETM4", AcceptAlgs=["JETM4Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

