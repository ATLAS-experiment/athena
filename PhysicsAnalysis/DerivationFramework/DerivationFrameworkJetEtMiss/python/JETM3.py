# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_JETM3.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def JETM3SkimmingToolCfg(flags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    from DerivationFrameworkJetEtMiss import TriggerLists
    electronTriggers = TriggerLists.single_el_Trig(flags)
    muonTriggers = TriggerLists.single_mu_Trig(flags)

    addRun3ElectronTriggers = ["HLT_e17_lhvloose_L1EM15VHI","HLT_e20_lhvloose_L1EM15VH", "HLT_e250_etcut_L1EM22VHI",
                               "HLT_e26_lhtight_ivarloose_L1EM22VHI","HLT_e26_lhtight_ivarloose_L1eEM26M",
                               "HLT_e60_lhmedium_L1EM22VHI","HLT_e60_lhmedium_L1eEM26M",
                               "HLT_e140_lhloose_L1EM22VHI","HLT_e140_lhloose_L1eEM26M",
                               "HLT_e300_etcut_L1EM22VHI","HLT_e300_etcut_L1eEM26M",
                               "HLT_e140_lhloose_noringer_L1EM22VHI","HLT_e140_lhloose_noringer_L1eEM26M"]

    addRund3MuonTriggers = ["HLT_mu24_ivarmedium_L1MU14FCH","HLT_mu50_L1MU14FCH","HLT_mu60_0eta105_msonly_L1MU14FCH","HLT_mu60_L1MU14FCH","HLT_mu80_msonly_3layersEC_L1MU14FCH"]

    electronTriggers = electronTriggers+addRun3ElectronTriggers
    muonTriggers = muonTriggers+addRund3MuonTriggers

    elofflinesel = '(count((Electrons.pt > 20*GeV) && (Electrons.DFCommonElectronsLHMedium)) >= 2)'
    muofflinesel = '(count((Muons.pt > 20*GeV) && (Muons.DFCommonMuonPassPreselection)) >= 2)'

    # TrigDecisionTool not used in skimming via xAODStringSkimmingTool here but with dedicated TriggerSkimmingTool
    # Explicitely set to "" here to be able to run over HL-LHC samples
    JETM3OfflineSkimmingTool_ele = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "JETM3OfflineSkimmingTool_ele",
                                                                                          expression = elofflinesel,
                                                                                          TrigDecisionTool="")
    JETM3OfflineSkimmingTool_mu = CompFactory.DerivationFramework.xAODStringSkimmingTool( name = "JETM3OfflineSkimmingTool_mu",
                                                                                          expression = muofflinesel,
                                                                                          TrigDecisionTool="")
    
    acc.addPublicTool(JETM3OfflineSkimmingTool_ele)
    acc.addPublicTool(JETM3OfflineSkimmingTool_mu)

    if not flags.Input.isMC:

        JETM3TriggerSkimmingTool_ele = CompFactory.DerivationFramework.TriggerSkimmingTool(name = "JETM3TriggerSkimmingTool_ele", TriggerListOR = electronTriggers)
        acc.addPublicTool(JETM3TriggerSkimmingTool_ele)
        JETM3TriggerSkimmingTool_mu = CompFactory.DerivationFramework.TriggerSkimmingTool(name = "JETM3TriggerSkimmingTool_mu", TriggerListOR = muonTriggers)
        acc.addPublicTool(JETM3TriggerSkimmingTool_mu)
        
        JETM3SkimmingTool_ele = CompFactory.DerivationFramework.FilterCombinationAND(name="JETM3SkimmingTool_ele", FilterList=[JETM3OfflineSkimmingTool_ele, JETM3TriggerSkimmingTool_ele] )
        JETM3SkimmingTool_mu = CompFactory.DerivationFramework.FilterCombinationAND(name="JETM3SkimmingTool_mu", FilterList=[JETM3OfflineSkimmingTool_mu, JETM3TriggerSkimmingTool_mu] )
        acc.addPublicTool(JETM3SkimmingTool_ele)
        acc.addPublicTool(JETM3SkimmingTool_mu)

        # Combine electron and muon channel
        JETM3SkimmingTool = CompFactory.DerivationFramework.FilterCombinationOR(name="JETM3SkimmingTool", FilterList=[JETM3SkimmingTool_ele, JETM3SkimmingTool_mu])
        acc.addPublicTool(JETM3SkimmingTool, primary = True)
    else:

        JETM3SkimmingTool = CompFactory.DerivationFramework.FilterCombinationOR(name="JETM3SkimmingTool", FilterList=[JETM3OfflineSkimmingTool_ele, JETM3OfflineSkimmingTool_mu])
        acc.addPublicTool(JETM3SkimmingTool, primary = True)

    return(acc)


# Main algorithm config
def JETM3KernelCfg(flags, name='JETM3Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM3"""
    acc = ComponentAccumulator()

    # Skimming
    skimmingTool = acc.getPrimaryAndMerge(JETM3SkimmingToolCfg(flags))

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg, MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, TauTrackParticleThinningCfg

    # https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DaodRecommendations
    JETM3_thinning_expression = "( abs(InDetTrackParticles.d0) < 5*mm ) && ( abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5*mm )"
    JETM3TrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        flags,
        name                    = "JETM3TrackParticleThinningTool",
        StreamName              = kwargs['StreamName'], 
        SelectionString         = JETM3_thinning_expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with muons
    JETM3MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "JETM3MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with electonrs
    JETM3ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                    = "JETM3ElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with photons
    JETM3PhotonTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                     = "JETM3PhotonTPThinningTool",
        StreamName               = kwargs['StreamName'],
        SGKey                    = "Photons",
        InDetTrackParticlesKey   = "InDetTrackParticles",
        GSFConversionVerticesKey = "GSFConversionVertices"))

    # Include inner detector tracks associated with taus
    JETM3TauTPThinningTool = acc.getPrimaryAndMerge(TauTrackParticleThinningCfg(
        flags,
        name                   = "JETM3TauTPThinningTool",
        StreamName             = kwargs['StreamName'],
        TauKey                 = "TauJets",
        InDetTrackParticlesKey = "InDetTrackParticles",
        DoTauTracksThinning    = True,
        TauTracksKey           = "TauTracks"))


    thinningTools = [JETM3TrackParticleThinningTool,
                     JETM3MuonTPThinningTool,
                     JETM3ElectronTPThinningTool,
                     JETM3PhotonTPThinningTool,
                     JETM3TauTPThinningTool]

    if flags.Input.isMC:
        truth_cond_WZH    = "((abs(TruthParticles.pdgId) >= 23) && (abs(TruthParticles.pdgId) <= 25))"                                      # W, Z and Higgs
        truth_cond_Lepton = "((abs(TruthParticles.pdgId) >= 11) && (abs(TruthParticles.pdgId) <= 16) && !(TruthParticle.isSimulationParticle))" # Leptons
        truth_cond_Quark  = "((abs(TruthParticles.pdgId) <=  5 && (TruthParticles.pt > 10000.)) || (abs(TruthParticles.pdgId) == 6))"       # Quarks
        truth_cond_Gluon  = "((abs(TruthParticles.pdgId) == 21) && (TruthParticles.pt > 10000.))"                                           # Gluons
        truth_cond_Photon = "((abs(TruthParticles.pdgId) == 22) && (TruthParticles.pt > 10000.) && !(TruthParticle.isSimulationParticle))"      # Photon

        truth_expression = '('+truth_cond_WZH+' || '+truth_cond_Lepton +' || '+truth_cond_Quark+'||'+truth_cond_Gluon+' || '+truth_cond_Photon+')'

        preserveAllDescendants = False

        JETM3TruthThinningTool = CompFactory.DerivationFramework.GenericTruthThinning ( name = "JETM3TruthThinningTool",
                                                                                        StreamName              = kwargs['StreamName'],
                                                                                        ParticleSelectionString = truth_expression,
                                                                                        PreserveDescendants     = preserveAllDescendants,
                                                                                        PreserveGeneratorDescendants = not preserveAllDescendants,
                                                                                        PreserveAncestors = True)

        acc.addPublicTool(JETM3TruthThinningTool)
        thinningTools.append(JETM3TruthThinningTool)

    # Finally the kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, 
                                      ThinningTools = thinningTools,
                                      SkimmingTools = [skimmingTool]))

    
    # Extra jet content:
    acc.merge(JETM3ExtraContentCfg(flags))

    return acc


def JETM3ExtraContentCfg(flags):

    acc = ComponentAccumulator()

    from JetRecConfig.JetRecConfig import JetRecCfg, getModifier
    from JetRecConfig.StandardJetMods import stdJetModifiers
    from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow, AntiKt4EMPFlowLowPt, AntiKt4EMTopoLowPt, AntiKt4UFOCSSKLowPt

    #=======================================
    # Schedule additional jet decorations
    #=======================================
    bJVTTool = getModifier(AntiKt4EMPFlow, stdJetModifiers['bJVT'], stdJetModifiers['bJVT'].modspec, flags=flags)
    acc.addEventAlgo(CompFactory.JetDecorationAlg(name='bJVTAlg',
                                                  JetContainer='AntiKt4EMPFlowJets', 
                                                  Decorators=[bJVTTool]))

    #======================================= 
    # Low pT cut containers
    #=======================================
    AntiKt4UFOCSSKLowPt_JETM3 = AntiKt4UFOCSSKLowPt.clone(
        modifiers = AntiKt4UFOCSSKLowPt.modifiers+("NNJVT",)
    )
    jetList = [AntiKt4EMPFlowLowPt, AntiKt4EMTopoLowPt, AntiKt4UFOCSSKLowPt_JETM3]


    for jd in jetList:
        acc.merge(JetRecCfg(flags,jd))


    # PFlow augmentation tool
    from DerivationFrameworkJetEtMiss.PFlowCommonConfig import PFlowCommonCfg
    acc.merge(PFlowCommonCfg(flags))

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


def JETM3Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM3TriggerListsHelper = TriggerListsHelper(flags)

    # Skimming, thinning, augmentation, extra content
    acc.merge(JETM3KernelCfg(flags, name="JETM3Kernel", StreamName = 'StreamDAOD_JETM3', TriggerListsHelper = JETM3TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM3SlimmingHelper = SlimmingHelper("JETM3SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    JETM3SlimmingHelper.SmartCollections = ["EventInfo","InDetTrackParticles", "PrimaryVertices",
                                            "Electrons", "Photons", "Muons", "TauJets",
                                            "MET_Baseline_AntiKt4EMTopo","MET_Baseline_AntiKt4EMPFlow",
                                            "AntiKt4EMPFlowJets","AntiKt4EMTopoJets",
                                            "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                            "BTagging_AntiKt4EMPFlow"]
    
    JETM3SlimmingHelper.AllVariables = ["CHSGChargedParticleFlowObjects", "CHSGNeutralParticleFlowObjects",
                                        "MuonSegments","LVL1JetRoIs",
                                        "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape","Kt4EMPFlowNeutEventShape"]

    JETM3SlimmingHelper.ExtraVariables = ["AntiKt4EMPFlowJets.passOnlyBJVT.DFCommonJets_bJvt.isJvtHS.isJvtPU",
                                          "Muons.energyLossType.EnergyLoss.ParamEnergyLoss.MeasEnergyLoss.EnergyLossSigma.MeasEnergyLossSigma.ParamEnergyLossSigmaPlus.ParamEnergyLossSigmaMinus"]
    
    # Low-level variables
    from DerivationFrameworkJetEtMiss.CommonJETMXContent import ClusterVariables, FlowElementVariables
    JETM3SlimmingHelper.ExtraVariables += [".".join(["CaloCalTopoClusters"] + ClusterVariables)]
    JETM3SlimmingHelper.ExtraVariables += [".".join(["GlobalChargedParticleFlowObjects"] + FlowElementVariables)]
    JETM3SlimmingHelper.ExtraVariables += [".".join(["GlobalNeutralParticleFlowObjects"] + FlowElementVariables)]

    # Truth containers
    if flags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(JETM3SlimmingHelper)

        JETM3SlimmingHelper.AppendToDictionary.update({'TruthParticles': 'xAOD::TruthParticleContainer',
                                                       'TruthParticlesAux': 'xAOD::TruthParticleAuxContainer'})
        
        JETM3SlimmingHelper.AllVariables += ["AntiKt4TruthJets", "InTimeAntiKt4TruthJets", "OutOfTimeAntiKt4TruthJets", 
                                             "TruthParticles", "TruthEvents", "TruthVertices", 
                                             "MuonTruthParticles", "egammaTruthParticles",]
        JETM3SlimmingHelper.SmartCollections += ["AntiKt4TruthWZJets","AntiKt4TruthJets","AntiKt10TruthJets",
                                                 "AntiKt10TruthTrimmedPtFrac5SmallR20Jets",
                                                 "AntiKt10TruthSoftDropBeta100Zcut10Jets"]

    # Trigger content
    JETM3SlimmingHelper.IncludeTriggerNavigation = False
    JETM3SlimmingHelper.IncludeJetTriggerContent = True
    JETM3SlimmingHelper.IncludeMuonTriggerContent = True
    JETM3SlimmingHelper.IncludeEGammaTriggerContent = True
    JETM3SlimmingHelper.IncludeTauTriggerContent = False
    JETM3SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM3SlimmingHelper.IncludeBJetTriggerContent = False
    JETM3SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM3SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM3SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = JETM3TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM3SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = JETM3TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(JETM3SlimmingHelper)

    jetOutputList = ["AntiKt4EMPFlowLowPtJets","AntiKt4EMTopoLowPtJets", "AntiKt4UFOCSSKLowPtJets"]
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addJetsToSlimmingTool
    addJetsToSlimmingTool(JETM3SlimmingHelper, jetOutputList, JETM3SlimmingHelper.SmartCollections)

    # Output stream
    JETM3ItemList = JETM3SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_JETM3", ItemList=JETM3ItemList, AcceptAlgs=["JETM3Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_JETM3", AcceptAlgs=["JETM3Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

