# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_JETM6.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def JETM6SkimmingToolCfg(flags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    from DerivationFrameworkJetEtMiss import TriggerLists
    electronTriggers = TriggerLists.single_el_Trig(flags)
    muonTriggers = TriggerLists.single_mu_Trig(flags)
    photonTriggers = TriggerLists.single_photon_Trig(flags)
    jetTriggers = TriggerLists.jetTrig(flags)

    # Trigger API doesn't currently return all triggers used in Run-3
    # Adding all jets triggers via explicit list for the moment
    jetTriggers += ["HLT_j0_pf_ftf_L1RD0_FILLED",
                    "HLT_j15_pf_ftf_L1RD0_FILLED",
                    "HLT_j25_pf_ftf_L1RD0_FILLED",
                    "HLT_j35_pf_ftf_L1RD0_FILLED",
                    "HLT_j45_pf_ftf_preselj20_L1RD0_FILLED",
                    "HLT_j45_pf_ftf_preselj20_L1J15",
                    "HLT_j60_pf_ftf_preselj50_L1J20",
                    "HLT_j85_pf_ftf_preselj50_L1J20",
                    "HLT_j110_pf_ftf_preselj80_L1J30",
                    "HLT_j175_pf_ftf_preselj140_L1J50",
                    "HLT_j260_pf_ftf_preselj200_L1J75",
                    "HLT_j360_pf_ftf_preselj225_L1J100",
                    "HLT_j420_pf_ftf_preselj225_L1J100",
                    "HLT_j0_L1RD0_FILLED",
                    "HLT_j15_L1RD0_FILLED",
                    "HLT_j25_L1RD0_FILLED",
                    "HLT_j35_L1RD0_FILLED",
                    "HLT_j45_preselj20_L1RD0_FILLED",
                    "HLT_j45_preselj20_L1J15",
                    "HLT_j60_preselj50_L1J20",
                    "HLT_j85_preselj50_L1J20",
                    "HLT_j110_preselj80_L1J30",
                    "HLT_j175_preselj140_L1J50",
                    "HLT_j260_preselj200_L1J75",
                    "HLT_j360_preselj225_L1J100",
                    "HLT_j420_L1J100",
                    "HLT_j420_pf_ftf_L1J100",
                    "HLT_j15f_L1RD0_FILLED",
                    "HLT_j25f_L1RD0_FILLED",
                    "HLT_j35f_L1RD0_FILLED",
                    "HLT_j45f_L1J15p31ETA49",
                    "HLT_j60f_L1J20p31ETA49",
                    "HLT_j85f_L1J20p31ETA49",
                    "HLT_j110f_L1J30p31ETA49",
                    "HLT_j175f_L1J50p31ETA49",
                    "HLT_j220f_L1J75p31ETA49",
                    "HLT_2j250c_j120c_pf_ftf_presel2j180XXj80_L1J100",
                    "HLT_3j200_pf_ftf_presel3j150_L1J100",
                    "HLT_4j115_pf_ftf_presel4j85_L13J50",
                    "HLT_5j70c_pf_ftf_presel5c50_L14J15",
                    "HLT_5j85_pf_ftf_presel5j50_L14J15",
                    "HLT_6j55c_pf_ftf_presel6j40_L14J15",
                    "HLT_6j70_pf_ftf_presel6j40_L14J15",
                    "HLT_7j45_pf_ftf_presel7j30_L14J15",
                    "HLT_10j40_pf_ftf_presel7j30_L14J15",
                    "HLT_3j200_L1J100",
                    "HLT_4j120_L13J50",
                    "HLT_5j70c_L14J15",
                    "HLT_5j85_L14J15",
                    "HLT_6j55c_L14J15",
                    "HLT_6j70_L14J15",
                    "HLT_7j45_L14J15",
                    "HLT_10j40_L14J15",
                    "HLT_6j35c_020jvt_pf_ftf_presel6c25_L14J15",
                    "HLT_3j200_pf_ftf_L1J100",
                    "HLT_6j35c_pf_ftf_presel6c25_L14J15",
                    "HLT_j85_a10sd_cssk_pf_jes_ftf_preselj50_L1J20",
                    "HLT_j85_a10t_lcw_jes_L1J20",
                    "HLT_j110_a10sd_cssk_pf_jes_ftf_preselj80_L1J30",
                    "HLT_j110_a10t_lcw_jes_L1J30",
                    "HLT_j175_a10sd_cssk_pf_jes_ftf_preselj140_L1J50",
                    "HLT_j175_a10t_lcw_jes_L1J50",
                    "HLT_j260_a10sd_cssk_pf_jes_ftf_preselj200_L1J75",
                    "HLT_j260_a10t_lcw_jes_L1J75",
                    "HLT_j360_a10sd_cssk_pf_jes_ftf_preselj225_L1J100",
                    "HLT_j360_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15",
                    "HLT_j360_a10t_lcw_jes_L1J100",
                    "HLT_j360_a10t_lcw_jes_L1SC111-CJ15",
                    "HLT_j420_35smcINF_a10t_lcw_jes_L1J100",
                    "HLT_j420_35smcINF_a10t_lcw_jes_L1SC111-CJ15",
                    "HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1J100",
                    "HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15",
                    "HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1J100",
                    "HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15",
                    "HLT_j460_a10t_lcw_jes_L1J100",
                    "HLT_j460_a10t_lcw_jes_L1SC111-CJ15",
                    "HLT_j460_a10r_L1J100",
                    "HLT_j460_a10r_L1SC111-CJ15",
                    "HLT_j460_a10_lcw_subjes_L1J100",
                    "HLT_j460_a10_lcw_subjes_L1SC111-CJ15",
                    "HLT_j420_a10t_lcw_jes_L1J100",
                    "HLT_j420_a10sd_cssk_pf_jes_ftf_preselj225_L1J100",
                    "HLT_2j330_35smcINF_a10t_lcw_jes_L1J100",
                    "HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1J100",
                    "HLT_2j330_35smcINF_a10t_lcw_jes_L1SC111-CJ15",
                    "HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15",
                    "HLT_j360_60smcINF_j360_a10t_lcw_jes_L1SC111-CJ15",
                    "HLT_j370_35smcINF_j370_a10t_lcw_jes_L1SC111-CJ15",
                    "HLT_j360_60smcINF_j360_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15",
                    "HLT_j370_35smcINF_j370_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15",
                    "HLT_2j330_a10t_lcw_jes_L1J100",
                    "HLT_2j330_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15",
                    "HLT_j0_HT1000_L1J100",
                    "HLT_j0_HT1000_L1HT190-J15s5pETA21",
                    "HLT_j0_HT1000_pf_ftf_preselj180_L1J100",
                    "HLT_j0_HT1000_pf_ftf_preselj180_L1HT190-J15s5pETA21",
                    "HLT_j0_HT1000_pf_ftf_preselcHT450_L1HT190-J15s5pETA21"
                ]

    # One electron or muon or high-pT photon + large-R jet or just a high-pT large-R jet
    jetsofflinesel = '(count( AntiKt10LCTopoJets.pt > 400.*GeV && abs(AntiKt10LCTopoJets.eta) < 2.5 ) >=1 || count( AntiKt10UFOCSSKJets.pt > 400.*GeV && abs(AntiKt10UFOCSSKJets.eta) < 2.5 ) >= 1)'
    if flags.Input.isMC:
        jetsofflinesel = '(count( AntiKt10LCTopoJets.pt > 180.*GeV && abs(AntiKt10LCTopoJets.eta) < 2.5 ) >=1 || count( AntiKt10UFOCSSKJets.pt > 180.*GeV && abs(AntiKt10UFOCSSKJets.eta) < 2.5 ) >= 1)'

    andstr = ' && '
    jetsel_lep = '(count( AntiKt10LCTopoJets.pt > 150.*GeV && abs(AntiKt10LCTopoJets.eta) < 2.5 ) >=1 || count( AntiKt10UFOCSSKJets.pt > 150.*GeV && abs(AntiKt10UFOCSSKJets.eta) < 2.5 ) >= 1)'
    elofflinesel = andstr.join(['count((Electrons.pt > 20*GeV) && (Electrons.DFCommonElectronsLHLoose)) >= 1',jetsel_lep])
    muofflinesel = andstr.join(['count((Muons.pt > 20*GeV) && (Muons.DFCommonMuonPassPreselection)) >= 1',jetsel_lep])
    gammaofflinesel = andstr.join(['count(Photons.pt > 150*GeV) >= 1',jetsel_lep])

    # TrigDecisionTool not used in skimming via xAODStringSkimmingTool here but with dedicated TriggerSkimmingTool
    # Explicitely set to "" here to be able to run over HL-LHC samples
    JETM6OfflineSkimmingTool_ele = CompFactory.DerivationFramework.xAODStringSkimmingTool( name = "JETM6OfflineSkimmingTool_ele",
                                                                                           expression = elofflinesel,
                                                                                           TrigDecisionTool="")
    JETM6OfflineSkimmingTool_mu    = CompFactory.DerivationFramework.xAODStringSkimmingTool( name = "JETM6OfflineSkimmingTool_mu",
                                                                                             expression = muofflinesel,
                                                                                             TrigDecisionTool="")
    JETM6OfflineSkimmingTool_gamma = CompFactory.DerivationFramework.xAODStringSkimmingTool( name = "JETM6OfflineSkimmingTool_gamma",
                                                                                             expression = gammaofflinesel,
                                                                                             TrigDecisionTool="")
    JETM6OfflineSkimmingTool_jets  = CompFactory.DerivationFramework.xAODStringSkimmingTool( name = "JETM6OfflineSkimmingTool_jets",
                                                                                             expression = jetsofflinesel,
                                                                                             TrigDecisionTool="")

    acc.addPublicTool(JETM6OfflineSkimmingTool_ele)
    acc.addPublicTool(JETM6OfflineSkimmingTool_mu)
    acc.addPublicTool(JETM6OfflineSkimmingTool_gamma)
    acc.addPublicTool(JETM6OfflineSkimmingTool_jets)

    if not flags.Input.isMC:
        JETM6TriggerSkimmingTool_ele   = CompFactory.DerivationFramework.TriggerSkimmingTool(name = "JETM6TriggerSkimmingTool_ele",   TriggerListOR = electronTriggers)
        acc.addPublicTool(JETM6TriggerSkimmingTool_ele)
        JETM6TriggerSkimmingTool_mu    = CompFactory.DerivationFramework.TriggerSkimmingTool(name = "JETM6TriggerSkimmingTool_mu",    TriggerListOR = muonTriggers)
        acc.addPublicTool(JETM6TriggerSkimmingTool_mu)
        JETM6TriggerSkimmingTool_gamma = CompFactory.DerivationFramework.TriggerSkimmingTool(name = "JETM6TriggerSkimmingTool_gamma", TriggerListOR = photonTriggers)
        acc.addPublicTool(JETM6TriggerSkimmingTool_gamma)
        JETM6TriggerSkimmingTool_jets  = CompFactory.DerivationFramework.TriggerSkimmingTool(name = "JETM6TriggerSkimmingTool_jets",  TriggerListOR = jetTriggers)
        acc.addPublicTool(JETM6TriggerSkimmingTool_jets)

        # Combine trigger and offline selection
        JETM6SkimmingTool_ele   = CompFactory.DerivationFramework.FilterCombinationAND(name="JETM6SkimmingTool_ele",   FilterList=[JETM6OfflineSkimmingTool_ele,   JETM6TriggerSkimmingTool_ele] )
        JETM6SkimmingTool_mu    = CompFactory.DerivationFramework.FilterCombinationAND(name="JETM6SkimmingTool_mu",    FilterList=[JETM6OfflineSkimmingTool_mu,    JETM6TriggerSkimmingTool_mu] )
        JETM6SkimmingTool_gamma = CompFactory.DerivationFramework.FilterCombinationAND(name="JETM6SkimmingTool_gamma", FilterList=[JETM6OfflineSkimmingTool_gamma, JETM6TriggerSkimmingTool_gamma] )
        JETM6SkimmingTool_jets  = CompFactory.DerivationFramework.FilterCombinationAND(name="JETM6SkimmingTool_jets",  FilterList=[JETM6OfflineSkimmingTool_jets,  JETM6TriggerSkimmingTool_jets] )

        acc.addPublicTool(JETM6SkimmingTool_ele)
        acc.addPublicTool(JETM6SkimmingTool_mu)
        acc.addPublicTool(JETM6SkimmingTool_gamma)
        acc.addPublicTool(JETM6SkimmingTool_jets)

        # Combine electron and muon channel
        JETM6SkimmingTool = CompFactory.DerivationFramework.FilterCombinationOR(name="JETM6SkimmingTool",
                                                                                FilterList=[JETM6SkimmingTool_ele, JETM6SkimmingTool_mu, JETM6SkimmingTool_gamma, JETM6SkimmingTool_jets])
        acc.addPublicTool(JETM6SkimmingTool, primary = True)

    else:
        JETM6SkimmingTool = CompFactory.DerivationFramework.FilterCombinationOR(
            name="JETM6SkimmingTool",
            FilterList=[JETM6OfflineSkimmingTool_ele,JETM6OfflineSkimmingTool_mu,JETM6OfflineSkimmingTool_gamma,JETM6OfflineSkimmingTool_jets])
    
        acc.addPublicTool(JETM6SkimmingTool, primary = True)

    return(acc)


# Main algorithm config
def JETM6KernelCfg(flags, name='JETM6Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM6"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    from DerivationFrameworkInDet.InDetToolsConfig import InDetTrackSelectionToolWrapperCfg
    DFCommonTrackSelection = acc.getPrimaryAndMerge(InDetTrackSelectionToolWrapperCfg(
        flags,
        name           = "DFJETM6CommonTrackSelectionLoose",
        CutLevel       = "Loose",
        DecorationName = "DFJETM6Loose"))

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation("JETM6CommonKernel", AugmentationTools = [DFCommonTrackSelection]))

    # Skimming
    skimmingTool = acc.getPrimaryAndMerge(JETM6SkimmingToolCfg(flags))

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg, MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, JetTrackParticleThinningCfg, TauTrackParticleThinningCfg

    # https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DaodRecommendations
    JETM6_thinning_expression = "InDetTrackParticles.DFJETM6Loose && (InDetTrackParticles.pt > 0.5*GeV) && (abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5.0*mm) && (abs(InDetTrackParticles.d0) < 5.0*mm)"
    JETM6TrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        flags,
        name                    = "JETM6TrackParticleThinningTool",
        StreamName              = kwargs['StreamName'], 
        SelectionString         = JETM6_thinning_expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with jets 

    JETM6Akt4PFlowJetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
        flags,
        name                         = "JETM6Akt4PFlowJetTPThinningTool",
        StreamName                   = kwargs['StreamName'],
        JetKey                       = "AntiKt4EMPFlowJets",
        SelectionString              = "AntiKt4EMPFlowJets.pt > 15*GeV",
        InDetTrackParticlesKey       = "InDetTrackParticles"))

    JETM6Akt10LCJetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
        flags,
        name                         = "JETM6Akt10LCJetTPThinningTool",
        StreamName                   = kwargs['StreamName'],
        JetKey                       = "AntiKt10LCTopoJets",
        SelectionString              = "AntiKt10LCTopoJets.pt > 150*GeV",
        InDetTrackParticlesKey       = "InDetTrackParticles"))

    JETM6Akt10UFOJetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
        flags,
        name                         = "JETM6Akt10UFOJetTPThinningTool",
        StreamName                   = kwargs['StreamName'],
        JetKey                       = "AntiKt10UFOCSSKJets",
        SelectionString              = "AntiKt10UFOCSSKJets.pt > 150*GeV",
        InDetTrackParticlesKey       = "InDetTrackParticles"))

    # Include inner detector tracks associated with muons
    JETM6MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "JETM6MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with electonrs
    JETM6ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                    = "JETM6ElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with taus
    JETM6TauTPThinningTool = acc.getPrimaryAndMerge(TauTrackParticleThinningCfg(
        flags,
        name                   = "JETM6TauTPThinningTool",
        StreamName             = kwargs['StreamName'],
        TauKey                 = "TauJets",
        InDetTrackParticlesKey = "InDetTrackParticles",
        DoTauTracksThinning    = True,
        TauTracksKey           = "TauTracks"))

    thinningTools = [JETM6TrackParticleThinningTool,
                     JETM6MuonTPThinningTool,
                     JETM6ElectronTPThinningTool,
                     JETM6TauTPThinningTool,
                     JETM6Akt10LCJetTPThinningTool,
                     JETM6Akt10UFOJetTPThinningTool,
                     JETM6Akt4PFlowJetTPThinningTool]

    if flags.Input.isMC:
        JETM6TruthJetInputThin = CompFactory.DerivationFramework.ViewContainerThinning( name = "JETM6ViewContThinning",
                                                                                        StreamName           = kwargs['StreamName'],
                                                                                        TruthParticleKey     = "TruthParticles",
                                                                                        TruthParticleViewKey = "JetInputTruthParticles")

        acc.addPublicTool(JETM6TruthJetInputThin)
        thinningTools.append(JETM6TruthJetInputThin)

    # Finally the kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, 
                                      ThinningTools = thinningTools,
                                      SkimmingTools = [skimmingTool]))

    
    # Extra jet content:
    acc.merge(JETM6ExtraContentCfg(flags))

    return acc


def JETM6ExtraContentCfg(flags):

    acc = ComponentAccumulator()

    #=======================================
    # More detailed truth information
    #=======================================

    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTopQuarkAndDownstreamParticlesCfg, AddTruthCollectionNavigationDecorationsCfg
        acc.merge(AddTopQuarkAndDownstreamParticlesCfg(flags, generations=4,rejectHadronChildren=True))
        acc.merge(AddTruthCollectionNavigationDecorationsCfg(flags, TruthCollections=["TruthTopQuarkWithDecayParticles","TruthBosonsWithDecayParticles"],prefix='Top'))


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


def JETM6Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM6TriggerListsHelper = TriggerListsHelper(flags)

    # Skimming, thinning, augmentation, extra content
    acc.merge(JETM6KernelCfg(flags, name="JETM6Kernel", StreamName = 'StreamDAOD_JETM6', TriggerListsHelper = JETM6TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM6SlimmingHelper = SlimmingHelper("JETM6SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    JETM6SlimmingHelper.SmartCollections = ["EventInfo","InDetTrackParticles","PrimaryVertices",
                                            "Electrons","Photons","Muons","TauJets",
                                            "MET_Baseline_AntiKt4EMPFlow",
                                            "AntiKt4EMPFlowJets",
                                            "AntiKt10LCTopoJets","AntiKt10UFOCSSKJets",
                                            "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                            "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                            "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                            "BTagging_AntiKt4EMPFlow"]

    JETM6SlimmingHelper.AllVariables = ["Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape","UFOCSSK"]
    JETM6SlimmingHelper.ExtraVariables  = ['CaloCalTopoClusters.calE.calEta.calM.calPhi.CENTER_MAG',
                                           'GlobalChargedParticleFlowObjects.chargedObjectLinks'
                                           'GlobalNeutralParticleFlowObjects.chargedObjectLinks'
                                           'CSSKGChargedParticleFlowObjects.pt.eta.phi.m.matchedToPV.originalObjectLink'
                                           'CSSKGNeutralParticleFlowObjects.pt.eta.phi.m.originalObjectLink',
                                           'AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.SizeParameter',
                                           'AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets.SizeParameter',
                                           'AntiKt10TruthTrimmedPtFrac5SmallR20Jets.SizeParameter',
                                           'AntiKt10TruthSoftDropBeta100Zcut10Jets.SizeParameter']

    # Truth containers
    if flags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(JETM6SlimmingHelper)

        JETM6SlimmingHelper.AppendToDictionary.update({'TruthParticles': 'xAOD::TruthParticleContainer',
                                                       'TruthParticlesAux': 'xAOD::TruthParticleAuxContainer'})
        
        JETM6SlimmingHelper.AllVariables += ["TruthEvents", "TruthParticles", "TruthTopQuarkWithDecayParticles", "TruthTopQuarkWithDecayVertices","TruthHFWithDecayParticles"]
        JETM6SlimmingHelper.SmartCollections += ["AntiKt4TruthJets", "AntiKt10TruthJets"]

    #Low-level inputs
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addOriginCorrectedClustersToSlimmingTool
    addOriginCorrectedClustersToSlimmingTool(JETM6SlimmingHelper,writeLC=True,writeEM=True) 

    JETM6SlimmingHelper.AppendToDictionary.update({"CSSKGChargedParticleFlowObjects":'xAOD::FlowElementContainer',
                                                   "CSSKGChargedParticleFlowObjectsAux":'xAOD::ShallowAuxContainer',
                                                   "CSSKGNeutralParticleFlowObjects":'xAOD::FlowElementContainer',
                                                   "CSSKGNeutralParticleFlowObjectsAux":'xAOD::ShallowAuxContainer'})

    # Trigger content
    JETM6SlimmingHelper.IncludeTriggerNavigation = False
    JETM6SlimmingHelper.IncludeJetTriggerContent = True
    JETM6SlimmingHelper.IncludeMuonTriggerContent = True
    JETM6SlimmingHelper.IncludeEGammaTriggerContent = True
    JETM6SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    JETM6SlimmingHelper.IncludeTauTriggerContent = False
    JETM6SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM6SlimmingHelper.IncludeBJetTriggerContent = False
    JETM6SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM6SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM6SlimmingHelper,
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = JETM6TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM6SlimmingHelper,
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = JETM6TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3
    if flags.Trigger.EDMVersion == 3:
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(JETM6SlimmingHelper)
        # Run 2 is added here temporarily to allow testing/comparison/debugging
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM6SlimmingHelper,
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = JETM6TriggerListsHelper.Run3TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM6SlimmingHelper,
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = JETM6TriggerListsHelper.Run3TriggerNamesNoTau)

    # Output stream    
    JETM6ItemList = JETM6SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_JETM6", ItemList=JETM6ItemList, AcceptAlgs=["JETM6Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_JETM6", AcceptAlgs=["JETM6Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

