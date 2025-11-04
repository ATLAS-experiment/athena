# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# STDM7.py - derivation for exclusive dilepton analyses
#            skimms dilepton (e or mu) events, contains InDetTracks and AFP information

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

def STDM7SkimmingToolCfg(flags):
    '''Configure the STDM7 skimming tool'''
    acc = ComponentAccumulator()

    # skim on two good leptons    
    muonsRequirements = '(Muons.pt >= 4*GeV) && (abs(Muons.eta) < 2.6)' \
                        '&& (Muons.DFCommonMuonPassPreselection) && (Muons.DFCommonMuonPassIDCuts)'
    electronsRequirements = '(Electrons.pt > 11*GeV) && (abs(Electrons.eta) < 2.6)' \
                            '&& ((Electrons.DFCommonElectronsLHLoose) || ((Electrons.DFCommonElectronsDNNLoose) && (Electrons.pt > 15*GeV)))'
    
    chargedParticleRequirements = '(TruthParticles.pt > 500) && (TruthParticles.isGenStable)' \
                                  '&& (TruthParticles.charge != 0)' \
                                  '&& (TruthParticles.theta > 0.163803) && (TruthParticles.theta < 2.97778)' \
                                  '&& (TruthParticles.HSBool)' 
    # theta selection correspond to |eta|<2.5 (minus epsilon); avoids floating point exception if theta=0 or pi
    
    muonOnlySelection = 'count('+muonsRequirements+') >=2'
    electronOnlySelection = 'count('+electronsRequirements+') >= 2'
    electronMuonSelection = '(count('+electronsRequirements+') + count('+muonsRequirements+')) >= 2'
    
    # for MC we may skim on the charged-particle multiplicity
    filterVal = -1 # Todo: nChFilter flag to be implemented later
    if filterVal > -1 and flags.Input.isMC :
        chargedParticleSelection = 'count('+chargedParticleRequirements+') < '+str(filterVal)
        offlineExpression = '(('+muonOnlySelection+' || '+electronOnlySelection+' || '+electronMuonSelection+') && ('+chargedParticleSelection+'))'
    else:
        offlineExpression = '(('+muonOnlySelection+' || '+electronOnlySelection+' || '+electronMuonSelection+'))'
    
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    STDM7StringSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "STDM7StringSkimmingTool",
                                                                                     expression = offlineExpression,
                                                                                     TrigDecisionTool=tdt)
    acc.addPublicTool(STDM7StringSkimmingTool)
    
    # require an OR of el and mu triggers, in the past we had a dedicated SM list but this should do just fine
    from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
    from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, TriggerType
    allperiods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future2e34
    TriggerAPI.setConfigFlags(flags)
    trig_el    = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el,  livefraction=0.8)
    trig_mu    = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu,  livefraction=0.8)
    trig_em    = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, additionalTriggerType=TriggerType.mu,  livefraction=0.8)

    # Read list of triggers from PHYS
    extra_notau = []
    from PathResolver import PathResolver
    with open(PathResolver.FindCalibFile("DerivationFrameworkPhys/run2ExtraMatchingTriggers.txt")) as fp:
        for line in fp:
            line = line.strip()
            if line == "" or line.startswith("#"):
                continue
            extra_notau.append(line)

    ## Merge and remove duplicates
    trigger_names_full_notau = list(set(trig_el+trig_mu+trig_em+extra_notau))
    STDM7TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(name = "STDM7TriggerSkimmingTool",
                                                                                   OutputLevel   = 0,
                                                                                   TriggerListOR = trigger_names_full_notau,
                                                                                   TriggerListAND = [] )
    acc.addPublicTool(STDM7TriggerSkimmingTool)

    # the two skimming tools go into an AND filter combination tool
    acc.addPublicTool(CompFactory.DerivationFramework.FilterCombinationAND("STDM7SkimmingTool",
                                                                           FilterList = [STDM7StringSkimmingTool,STDM7TriggerSkimmingTool]),
                      primary = True)

    return(acc)


def STDM7KernelCfg(flags, name='STDM7Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Inner detector group recommendations for indet tracks in analysis
    # --> removed InDetTrackParticle thinning as we need all

    # Finally the kernel itself
    thinningTools = [] # none for the moment
    skimmingTools = [ acc.getPrimaryAndMerge(STDM7SkimmingToolCfg(flags)) ]

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, 
                                      ThinningTools = thinningTools,
                                      SkimmingTools = skimmingTools))
    return acc


def STDM7Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    STDM7TriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(STDM7KernelCfg(flags, name="STDM7Kernel", StreamName = 'StreamDAOD_STDM7', TriggerListsHelper = STDM7TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    STDM7SlimmingHelper = SlimmingHelper("STDM7SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    STDM7SlimmingHelper.SmartCollections = ["EventInfo",
                                            "Electrons",
                                            "Photons",
                                            "Muons",
                                            "PrimaryVertices",
                                            "InDetTrackParticles",
                                            "AntiKt4EMTopoJets",
                                            "AntiKt4EMPFlowJets",
                                            "MET_Baseline_AntiKt4EMTopo",
                                            "MET_Baseline_AntiKt4EMPFlow",
                                            "TauJets"
                                            # custom reconstruction for yy->WW
                                            # this has not yet been migrated to master, and smart slimming is not yet available
                                            # "LowPtRoITrackParticles", 
                                            # "LowPtRoIVertexContainer"
    ]

    # STDM7 needs AFP information
    STDM7SlimmingHelper.AllVariables = [ "AFPSiHitContainer",
                                         "AFPToFHitContainer",
                                         "AFPSiHitsClusterContainer",
                                         "AFPTrackContainer",
                                         "AFPToFTrackContainer",    
                                         "AFPProtonContainer",
                                         "AFPVertexContainer"
    ]

    excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    StaticContent = []
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData]
    
    STDM7SlimmingHelper.StaticContent = StaticContent
    
    # Truth containers
    if flags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(STDM7SlimmingHelper)
        STDM7SlimmingHelper.AllVariables += ['TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm','TruthPileupParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']
        STDM7SlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
                                              "Muons.TruthLink",
                                              "Photons.TruthLink",
                                              "AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
                                              "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
                                              "TruthPrimaryVertices.t.x.y.z",
                                              "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                              "EventInfo.hardScatterVertexLink.timeStampNSOffset",
                                              "TauJets.dRmax.etOverPtLeadTrk",
                                               # "TauJets_MuonRM.dRmax.etOverPtLeadTrk", # copied from PHYS but container is not in STDM7
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET.ex.ey",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht.ex.ey"]

    # Trigger content
    STDM7SlimmingHelper.IncludeTriggerNavigation = False
    STDM7SlimmingHelper.IncludeJetTriggerContent = False
    STDM7SlimmingHelper.IncludeMuonTriggerContent = False
    STDM7SlimmingHelper.IncludeEGammaTriggerContent = False
    STDM7SlimmingHelper.IncludeTauTriggerContent = False
    STDM7SlimmingHelper.IncludeEtMissTriggerContent = False
    STDM7SlimmingHelper.IncludeBJetTriggerContent = False
    STDM7SlimmingHelper.IncludeBPhysTriggerContent = False
    STDM7SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = STDM7SlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_", 
                                         TriggerList = STDM7TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = STDM7SlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = STDM7TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(STDM7SlimmingHelper)

    # Output stream    
    STDM7ItemList = STDM7SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_STDM7", ItemList=STDM7ItemList, AcceptAlgs=["STDM7Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_STDM7", AcceptAlgs=["STDM7Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
