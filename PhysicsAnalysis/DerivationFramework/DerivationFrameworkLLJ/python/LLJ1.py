# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_PHYS.py
# This defines DAOD_PHYS, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority 
# of physics analyses in ATLAS.
# It requires the flag PHYS in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def LLJ1KernelCfg(flags, name='LLJ1Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for LLJ1"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Thinning tools
    # These are set up in PhysCommonThinningConfig. Only thing needed here the list of tools to schedule 
    thinningToolsArgs = {
        'TrackParticleThinningToolName'       : "LLJ1TrackParticleThinningTool",
        'MuonTPThinningToolName'              : "LLJ1MuonTPThinningTool",
        'TauJetThinningToolName'              : "LLJ1TauJetThinningTool",
        'TauJets_MuonRMThinningToolName'      : "LLJ1TauJets_MuonRMThinningTool",
        'DiTauThinningToolName'               : "LLJ1DiTauThinningTool",
        'DiTauTPThinningToolName'             : "LLJ1DiTauTPThinningTool",
        'DiTauLowPtThinningToolName'          : "LLJ1DiTauLowPtThinningTool",
        'DiTauLowPtTPThinningToolName'        : "LLJ1DiTauLowPtTPThinningTool",
    } 
    # Configure the thinning tools
    from DerivationFrameworkPhys.PhysCommonThinningConfig import PhysCommonThinningCfg
    acc.merge(PhysCommonThinningCfg(flags, StreamName = kwargs['StreamName'], **thinningToolsArgs))
    # Get them from the CA so they can be added to the kernel
    thinningTools = []
    for key in thinningToolsArgs:
        thinningTools.append(acc.getPublicTool(thinningToolsArgs[key]))

    ### skimming tool
    skimmingTool = acc.getPrimaryAndMerge(LLJ1SkimmingToolCfg(flags))

    # The kernel algorithm itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, ThinningTools = thinningTools))      
    acc.addEventAlgo(DerivationKernel(name, SkimmingTools = [skimmingTool]))       

    return acc

def LLJ1SkimmingToolCfg(flags):
    """Configure the LLJ1 skimming tool"""

    ### Define selection for event skimming
    largeRJetsForSkimming = ["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets."]

    ### 1 jet object skimming
    sel_1jet_template = "((count (abs({0}eta) < 2.8 && {0}pt > 150*GeV && {0}m > 30*GeV)  >= 1))"
    topology_selection_1jet = "({})".format(
        " || ".join([sel_1jet_template.format(j) for j in largeRJetsForSkimming])
    )

    ### do the job
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "LLJ1ObjectsSkimming",
                                                                            expression = topology_selection_1jet,
                                                                            ), 
                        primary = True)

    ### trigger skimming
    TriggersList = [
        ### baseline run-2
        'HLT_j360_a10_lcw_sub_L1J100',
        'HLT_j420_a10_lcw_L1J100',
        'HLT_j460_a10t_lcw_jes_L1J100',
        ### new run-3
        'HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1J100',
        'HLT_j460_a10_lcw_subjes_L1J100',
        'HLT_j460_a10r_L1J100',
        ### new run-3 mass cut
        'HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1J100',
        'HLT_j420_35smcINF_a10t_lcw_jes_L1J100',
    ]
    print(TriggersList)

    ### do the job
    acc.addPublicTool(CompFactory.DerivationFramework.TriggerSkimmingTool(name = "LLJ1TriggerSkimming", 
                                                                            TriggerListOR = TriggersList), 
                                                                            primary = True)
    
    return(acc)                          


def LLJ1Cfg(flags):
    stream_name = 'StreamDAOD_LLJ1'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    LLJ1TriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(LLJ1KernelCfg(flags, name="LLJ1Kernel", StreamName = stream_name, TriggerListsHelper = LLJ1TriggerListsHelper))
    
    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    LLJ1SlimmingHelper = SlimmingHelper("LLJ1SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    LLJ1SlimmingHelper.SmartCollections = ["EventInfo",
                                           "Electrons",
                                           "Photons",
                                           "Muons",
                                           "PrimaryVertices",
                                           "InDetTrackParticles",
                                           "AntiKt4EMTopoJets",
                                           "AntiKt4EMPFlowJets",

                                           "MET_Baseline_AntiKt4EMTopo",
                                           "MET_Baseline_AntiKt4EMPFlow",
                                           "TauJets",
                                           "TauJets_MuonRM",
                                           "DiTauJets",
                                           "DiTauJetsLowPt",
                                           "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                           "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                           "AntiKtVR30Rmax4Rmin02PV0TrackJets",
                                          ]
    
    excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    StaticContent = []
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData]   

    LLJ1SlimmingHelper.StaticContent = StaticContent
   
    # Extra content
    LLJ1SlimmingHelper.ExtraVariables += ["AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
                                              "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
                                              "TruthPrimaryVertices.t.x.y.z",
                                              "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                              "EventInfo.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ",
                                              "TauJets.dRmax.etOverPtLeadTrk",
                                              "TauJets_MuonRM.dRmax.etOverPtLeadTrk",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET.ex.ey",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht.ex.ey"]

    ### add more variables
    LLJ1SlimmingHelper.AllVariables += ["CaloCalTopoClusters", "CaloCalFwdTopoTowers",
                                        "GlobalChargedParticleFlowObjects", "GlobalNeutralParticleFlowObjects",
                                        "CHSGChargedParticleFlowObjects","CHSGNeutralParticleFlowObjects",
                                        "CSSKGChargedParticleFlowObjects","CSSKGNeutralParticleFlowObjects",
                                        "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape",
                                        "Kt4EMPFlowNeutEventShape","Kt4UFOCSSKEventShape","Kt4UFOCSSKNeutEventShape"
                                        ]

    LLJ1SlimmingHelper.ExtraVariables += ["AntiKt4EMPFlowJets.GhostTower",
                                          "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.SizeParameter",
                                          "UFOCSSK.pt.eta.phi.m.signalType.otherObjectLinks.chargedObjectLinks",
                                          "UFO.pt.eta.phi.m.signalType.otherObjectLinks.chargedObjectLinks",
                                          "InDetTrackParticles.particleHypothesis.vx.vy.vz.btagIp_d0Uncertainty.btagIp_z0SinThetaUncertainty.btagIp_z0SinTheta.btagIp_trackMomentum.btagIp_trackDisplacement.btagIp_invalidIp",
                                          "GSFTrackParticles.particleHypothesis.vx.vy.vz",
                                          "PrimaryVertices.x.y.z.covariance.trackWeights",
                                          "TauJets.clusterLinks",
                                          "Electrons.neutralGlobalFELinks.chargedGlobalFELinks",
                                          "Photons.neutralGlobalFELinks",
                                          "Muons.energyLossType.EnergyLoss.ParamEnergyLoss.MeasEnergyLoss.EnergyLossSigma.MeasEnergyLossSigma.ParamEnergyLossSigmaPlus.ParamEnergyLossSigmaMinus.clusterLinks.FSR_CandidateEnergy.neutralGlobalFELinks.chargedGlobalFELinks",
                                          "MuonSegments.x.y.z.px.py.pz"
                                          ]

    LLJ1SlimmingHelper.AppendToDictionary.update({'CSSKGNeutralParticleFlowObjects': 'xAOD::FlowElementContainer',
                                                   'CSSKGNeutralParticleFlowObjectsAux': 'xAOD::ShallowAuxContainer',
                                                   'CSSKGChargedParticleFlowObjects': 'xAOD::FlowElementContainer',
                                                   'CSSKGChargedParticleFlowObjectsAux': 'xAOD::ShallowAuxContainer',
                                                   'UFO': 'xAOD::FlowElementContainer',
                                                   'UFOAux': 'xAOD::FlowElementAuxContainer',
                                                   'Kt4UFOCSSKEventShape': 'xAOD::EventShape',
                                                   'Kt4UFOCSSKEventShapeAux': 'xAOD::EventShapeAuxInfo',
                                                   'Kt4UFOCSSKNeutEventShape': 'xAOD::EventShape',
                                                   'Kt4UFOCSSKNeutEventShapeAux': 'xAOD::EventShapeAuxInfo'})

    ### jets constituents
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addOriginCorrectedClustersToSlimmingTool
    addOriginCorrectedClustersToSlimmingTool(LLJ1SlimmingHelper,writeLC=True,writeEM=True)

    # FTAG Xbb extra content
    extraList = []
    for tagger in ["GN2Xv00", "GN2XWithMassv00"]:
        for score in ["phbb", "phcc", "ptop", "pqcd"]:
            extraList.append(f"{tagger}_{score}")
    LLJ1SlimmingHelper.ExtraVariables += ["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets." + ".".join(extraList)]
 
    # Truth extra content
    if flags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(LLJ1SlimmingHelper)
        LLJ1SlimmingHelper.AllVariables += ['TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm','TruthPileupParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']
        LLJ1SlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
                                              "Muons.TruthLink",
                                              "Photons.TruthLink"]
      
    # Trigger content
    LLJ1SlimmingHelper.IncludeTriggerNavigation = False
    LLJ1SlimmingHelper.IncludeJetTriggerContent = True
    LLJ1SlimmingHelper.IncludeMuonTriggerContent = False
    LLJ1SlimmingHelper.IncludeEGammaTriggerContent = False
    LLJ1SlimmingHelper.IncludeTauTriggerContent = False
    LLJ1SlimmingHelper.IncludeEtMissTriggerContent = False
    LLJ1SlimmingHelper.IncludeBJetTriggerContent = False
    LLJ1SlimmingHelper.IncludeBPhysTriggerContent = False
    LLJ1SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = LLJ1SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = LLJ1TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = LLJ1SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = LLJ1TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(LLJ1SlimmingHelper)

    # Output stream    
    LLJ1ItemList = LLJ1SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_LLJ1", ItemList=LLJ1ItemList, AcceptAlgs=["LLJ1Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_LLJ1", AcceptAlgs=["LLJ1Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

