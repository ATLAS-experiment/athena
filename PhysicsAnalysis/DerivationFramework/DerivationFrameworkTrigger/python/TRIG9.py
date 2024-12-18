# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#========================================================================
# TRIG9.py
# This defines DAOD_TRIG9, a DAOD format for Run 3.
# It contains the variables and objects needed for TauTrigger performance
# such as online and offline tracks, RoIs, and offline objects.  
# Only events passing single lepton or tau trigger chains are kept.
# It requires the flag TRIG9 in Derivation_tf.py
#========================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def TRIG9KernelCfg(flags, name='TRIG9Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for TRIG9"""
    acc = ComponentAccumulator()

    # Augmentations
    augmentationTools = [ ]

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))
    
    from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
    from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod

    #allperiods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future2e34
    allperiods = TriggerPeriod.future2e34
    TriggerAPI.setConfigFlags(flags)
    trig_all = list(TriggerAPI.getAllHLT(allperiods).keys())
    
    # Add in Run 3 triggers
    TriggerListsHelper = kwargs['TriggerListsHelper']
    trig_all += TriggerListsHelper.Run3TriggerNames

    # Thinning tools...
    # track thinning
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg
    tp_thinning_expression = "InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 3.0*mm && InDetTrackParticles.pt > 10*GeV"
    TRIG9TrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
            flags,
            name                    = 'TRIG9TrackParticleThinningTool',
            StreamName              = kwargs['StreamName'],
            SelectionString         = tp_thinning_expression,
            InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with muons
    from DerivationFrameworkInDet.InDetToolsConfig import MuonTrackParticleThinningCfg
    TRIG9MuonTrackParticleThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
            flags,
            name                    = 'TRIG9MuonTrackParticleThinningTool',
            StreamName              = kwargs['StreamName'],
            MuonKey                 = "Muons",
            InDetTrackParticlesKey  = "InDetTrackParticles"))

    # tau thinning
    from DerivationFrameworkTau.TauCommonConfig import TauThinningCfg
    #tau_thinning_expression = "TauJets.pt >= 20*GeV"
    tau_thinning_expression = "( TauJets.pt >= 20*GeV && abs(TauJets.eta) < 2.5 && abs(TauJets.charge)==1.0 && (TauJets.nTracks == 1 || TauJets.nTracks == 3) )"

    TRIG9TauJetThinningTool = acc.getPrimaryAndMerge(TauThinningCfg(
            flags,
            name                 = 'TRIG9TauJetThinningTool',
            StreamName           = kwargs['StreamName'],
            Taus                 = "TauJets",
            TauTracks            = "TauTracks",
            TrackParticles       = "InDetTrackParticles",
            TauNeutralPFOs       = "TauNeutralParticleFlowObjects",
            TauSecondaryVertices = "TauSecondaryVertices",
            SelectionString      = tau_thinning_expression))

    # Finally the kernel itself
    thinningTools = [
                     TRIG9TrackParticleThinningTool,
                     TRIG9MuonTrackParticleThinningTool,
                     TRIG9TauJetThinningTool,
                    ]

    # Skimming
    skimmingTools = []

    # skimming events based on electron/muon + tau counting
    tauProngs13 = "( abs(TauJets.charge)==1.0 && (TauJets.nTracks == 1 || TauJets.nTracks == 3) )"
    #MuTrig: (pT(mu)>18 && pT(tau)>18), EleTrig: (pT(el)>22 && pT(tau)>18)
    e22   = '(count( Electrons.pt > 22.0*GeV && abs(Electrons.eta) < 2.5 && Electrons.DFCommonElectronsLHLoose) >= 1)'
    mu18  = '(count( Muons.pt > 18.0*GeV && abs(Muons.eta) < 2.5 && Muons.DFCommonMuonPassPreselection) >= 1)'
    tau20 = '(count( TauJets.pt > 20.0*GeV && abs(TauJets.eta) < 2.5 && '+tauProngs13+' && TauJets.DFTauLoose ) >= 1)'
    mutau = '('+mu18+' && '+tau20+')'
    etau  = '('+e22+' && '+tau20+')'
    skim_expression = '('+mutau+') || ('+etau+')'

    EventSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
        name="EventSkimmingTool", expression=skim_expression,
    )
    acc.addPublicTool(EventSkimmingTool)
    skimmingTools.append(EventSkimmingTool)

    # Pieces of trigger names to keep
    trig_keys = ['mediumRNN',]
    # Add specific triggers
    additional_triggers = [
        "HLT_mu24_ivarmedium_L1MU14FCH",
        "HLT_e26_lhtight_ivarloose_L1eEM26M",
    ]
    trig_keys += additional_triggers
    triggers = [t for t in trig_all for k in trig_keys if k in t]

    # remove not used triggers
    trig_veto = ['HLT_g','HLT_e17_','HLT_e24_','HLT_mu14_',]
    final_triggers = [t for t in triggers for k in trig_veto if k not in t]

    #remove duplicates
    final_triggers = sorted(list(set(final_triggers)))
    print('TRIG9 list of triggers used for skimming:')
    for trig in final_triggers: print(trig)

    TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool
    TRIG9TriggerSkimmingTool = TriggerSkimmingTool(name = "TRIG9TriggerPreSkimmingTool", 
                                                   TriggerListAND = [],
                                                   TriggerListOR  = final_triggers)
    acc.addPublicTool(TRIG9TriggerSkimmingTool)

    skimmingTools.append(TRIG9TriggerSkimmingTool)

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name,
                                      SkimmingTools = skimmingTools,
                                      ThinningTools = thinningTools,
                                      AugmentationTools = augmentationTools))

    return acc


def TRIG9Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    TRIG9TriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(TRIG9KernelCfg(flags, name="TRIG9Kernel", StreamName = 'StreamDAOD_TRIG9', TriggerListsHelper = TRIG9TriggerListsHelper))


    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    TRIG9SlimmingHelper = SlimmingHelper("TRIG9SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    TRIG9SlimmingHelper.SmartCollections = ["EventInfo",
                                            "Electrons",
                                            "Photons",
                                            "Muons",
                                            "PrimaryVertices",
                                            "InDetTrackParticles",
                                            "AntiKt4EMPFlowJets",
                                            "MET_Baseline_AntiKt4EMPFlow",
                                            "BTagging_AntiKt4EMPFlow",
                                            "TauJets",
                                            ]

    TRIG9SlimmingHelper.StaticContent = [ 
                            "TrigRoiDescriptorCollection#HLT_eTAURoIs",
                            "TrigRoiDescriptorCollection#HLT_jTAURoIs",
                            "TrigRoiDescriptorCollection#HLT_cTAURoIs",
                            "TrigRoiDescriptorCollection#HLT_TAURoI",
                            "TrigRoiDescriptorCollection#HLT_Roi_Tau",
                            "TrigRoiDescriptorCollection#HLT_Roi_Tau_probe",
                            "TrigRoiDescriptorCollection#HLT_Roi_TauCore",
                            "TrigRoiDescriptorCollection#HLT_Roi_TauCore_probe",
                            "TrigRoiDescriptorCollection#HLT_Roi_TauIso",
                            "TrigRoiDescriptorCollection#HLT_Roi_TauIso_probe",
                            ]

    TRIG9SlimmingHelper.ExtraVariables += [ 
                            "TruthPrimaryVertices.t.x.y.z",
                            "PrimaryVertices.t.x.y.z.numberDoF.chiSquared.covariance.trackParticleLinks",
                            "EventInfo.hardScatterVertexLink.timeStampNSOffset",
                           ]

    TRIG9SlimmingHelper.AllVariables += ['HLT_TrigTauRecMerged_MVA','HLT_tautrack_MVA']

    # Truth containers
    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(TRIG9SlimmingHelper)
        TRIG9SlimmingHelper.AllVariables += ['InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']
        TRIG9SlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
                                              "Muons.TruthLink",
                                              "Photons.TruthLink"]

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTauAndDownstreamParticlesCfg
        acc.merge(AddTauAndDownstreamParticlesCfg(flags))
        TRIG9SlimmingHelper.AllVariables += ['TruthTausWithDecayParticles','TruthTausWithDecayVertices']




    # Trigger content
    TRIG9SlimmingHelper.IncludeTriggerNavigation = True
    TRIG9SlimmingHelper.IncludeAdditionalTriggerContent = True
    TRIG9SlimmingHelper.IncludeJetTriggerContent = False
    TRIG9SlimmingHelper.IncludeMuonTriggerContent = False
    TRIG9SlimmingHelper.IncludeEGammaTriggerContent = False
    TRIG9SlimmingHelper.IncludeTauTriggerContent = False
    TRIG9SlimmingHelper.IncludeEtMissTriggerContent = False
    TRIG9SlimmingHelper.IncludeBJetTriggerContent = False
    TRIG9SlimmingHelper.IncludeBPhysTriggerContent = False
    TRIG9SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TRIG9SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = TRIG9TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TRIG9SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = TRIG9TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(TRIG9SlimmingHelper)

    # Output stream
    TRIG9ItemList = TRIG9SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_TRIG9", ItemList=TRIG9ItemList, AcceptAlgs=["TRIG9Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_TRIG9", AcceptAlgs=["TRIG9Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData,MetadataCategory.TruthMetaData]))

    return acc

