# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_JETM2.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory, LHCPeriod

# Main algorithm config
def JETM2KernelCfg(flags, name='JETM2Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM2"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    if flags.Input.isMC:
        # thinning tools: 
        truthThinningTool = CompFactory.DerivationFramework.MenuTruthThinning(name               = "JETM2TruthThinning",
                                                                              StreamName         = kwargs['StreamName'],
                                                                              WriteAllStable     = True,
                                                                              WritePartons       = False,
                                                                              WriteHadrons       = False,
                                                                              WriteBHadrons      = True,
                                                                              WriteCHadrons      = False,
                                                                              WriteGeant         = False,
                                                                              WriteFirstN        = 10)

        acc.addPublicTool(truthThinningTool)

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, ThinningTools = [truthThinningTool] if flags.Input.isMC else []))

    # Extra jet content:
    acc.merge(JETM2ExtraContentCfg(flags))

    return acc


def JETM2ExtraContentCfg(flags):

    acc = ComponentAccumulator()

    from JetRecConfig.JetRecConfig import JetRecCfg, getInputAlgs, getConstitPJGAlg
    from JetRecConfig.JetInputConfig import buildEventShapeAlg
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    from JetRecConfig.StandardSmallRJets import AntiKt4UFOCSSKNoPtCut, AntiKt4EMPFlowNoPtCut, AntiKt4EMTopoNoPtCut
    from JetRecConfig.StandardLargeRJets import AntiKt10TruthDressedWZSoftDrop

    #=======================================
    # CHS R = 0.4 UFO jets
    #=======================================

    algs = getInputAlgs(cst.UFO, flags=flags)
    for alg in algs:
        if isinstance(alg, ComponentAccumulator):
            acc.merge(alg)
        else:
            acc.addEventAlgo(alg)

    #=======================================
    # NoPtCut containers
    #=======================================
    jetList = [AntiKt4UFOCSSKNoPtCut,AntiKt4EMPFlowNoPtCut,AntiKt4EMTopoNoPtCut]

    #=======================================
    # R = 1.0 truth WZ Dressed jets
    #=======================================
    if flags.Input.isMC:
        jetList += [AntiKt10TruthDressedWZSoftDrop]


    for jd in jetList:
        acc.merge(JetRecCfg(flags,jd))

    #=======================================
    # UFO CSSK event shape 
    #=======================================

    acc.addEventAlgo(buildEventShapeAlg(cst.UFOCSSK,'', suffix=None))
    acc.addEventAlgo(getConstitPJGAlg(cst.UFOCSSK, suffix='Neut'))
    acc.addEventAlgo(buildEventShapeAlg(cst.UFOCSSK,'', suffix='Neut'))

    #=======================================
    # More detailed truth information
    #=======================================

    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTopQuarkAndDownstreamParticlesCfg, AddTruthCollectionNavigationDecorationsCfg
        acc.merge(AddTopQuarkAndDownstreamParticlesCfg(flags))
        acc.merge(AddTruthCollectionNavigationDecorationsCfg(flags, TruthCollections=["TruthTopQuarkWithDecayParticles","TruthBosonsWithDecayParticles"],prefix='Top'))


    return acc


def JETM2Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM2TriggerListsHelper = TriggerListsHelper(flags)

    # Skimming, thinning, augmentation, extra content
    acc.merge(JETM2KernelCfg(flags, name="JETM2Kernel", StreamName = 'StreamDAOD_JETM2', TriggerListsHelper = JETM2TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM2SlimmingHelper = SlimmingHelper("JETM2SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    JETM2SlimmingHelper.SmartCollections = ["EventInfo","InDetTrackParticles", "PrimaryVertices",
                                            "Electrons", "Photons", "Muons", "TauJets",
                                            "MET_Baseline_AntiKt4EMPFlow",
                                            "AntiKt4EMTopoNoPtCutJets","AntiKt4EMPFlowJets",
                                            "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                            "BTagging_AntiKt4EMPFlow",
                                            "BTagging_AntiKtVR30Rmax4Rmin02Track"]

    JETM2SlimmingHelper.AllVariables = ["CaloCalFwdTopoTowers",
                                        "CHSGChargedParticleFlowObjects","CHSGNeutralParticleFlowObjects",
                                        "CSSKGChargedParticleFlowObjects","CSSKGNeutralParticleFlowObjects",
                                        "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape",
                                        "Kt4EMPFlowNeutEventShape","Kt4UFOCSSKEventShape","Kt4UFOCSSKNeutEventShape"]

    # Low-level inputs
    from DerivationFrameworkJetEtMiss.CommonJETMXContent import ClusterVariables, FlowElementVariables, UFOVariables, TrackingVariables,  TrackingVariablesHGTD, FELinks
    JETM2SlimmingHelper.ExtraVariables += [".".join(["CaloCalTopoClusters"] + ClusterVariables)]
    JETM2SlimmingHelper.ExtraVariables += [".".join(["GlobalChargedParticleFlowObjects"] + FlowElementVariables)]
    JETM2SlimmingHelper.ExtraVariables += [".".join(["GlobalNeutralParticleFlowObjects"] + FlowElementVariables)]
    JETM2SlimmingHelper.ExtraVariables += [".".join(["UFO"] + UFOVariables)]
    JETM2SlimmingHelper.ExtraVariables += [".".join(["UFOCSSK"] + UFOVariables)]
    JETM2SlimmingHelper.ExtraVariables += [".".join(["InDetTrackParticles"] + TrackingVariables)]
    if flags.GeoModel.Run >= LHCPeriod.Run4:
        JETM2SlimmingHelper.ExtraVariables += [".".join(["InDetTrackParticles"] + TrackingVariablesHGTD)]

    # Links of physics objects to FlowElements
    JETM2SlimmingHelper.ExtraVariables += FELinks

    # Detailed substructure information
    from DerivationFrameworkJetEtMiss.CommonJETMXContent import ExtraJSSVariables
    JETM2SlimmingHelper.ExtraVariables += [".".join(["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"] + ExtraJSSVariables)]

    # sub-structure taggers score
    from DerivationFrameworkJetEtMiss.CommonJETMXContent import JSSTaggersVariables
    JETM2SlimmingHelper.ExtraVariables += JSSTaggersVariables

    JETM2SlimmingHelper.ExtraVariables += ["AntiKt4EMPFlowJets.GhostTower.IsoFixedCone5Pt.IsoFixedCone5PtPUsub",
                                           "AntiKt4EMTopoNoPtCutJets.IsoFixedCone5Pt.IsoFixedCone5PtPUsub",
                                           "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.SizeParameter.GhostTrack",
                                           "GSFTrackParticles.particleHypothesis.vx.vy.vz",
                                           "PrimaryVertices.x.y.z.covariance.trackWeights",
                                           "TauJets.clusterLinks",
                                           "Muons.energyLossType.EnergyLoss.ParamEnergyLoss.MeasEnergyLoss.EnergyLossSigma.MeasEnergyLossSigma.ParamEnergyLossSigmaPlus.ParamEnergyLossSigmaMinus.clusterLinks.FSR_CandidateEnergy",
                                           "MuonSegments.x.y.z.px.py.pz",
                                           "BTagging_AntiKt4EMPFlow.jetLink",
                                           "BTagging_AntiKtVR30Rmax4Rmin02Track.jetLink"]

    JETM2SlimmingHelper.AppendToDictionary.update({'CSSKGNeutralParticleFlowObjects': 'xAOD::FlowElementContainer',
                                                   'CSSKGNeutralParticleFlowObjectsAux': 'xAOD::ShallowAuxContainer',
                                                   'CSSKGChargedParticleFlowObjects': 'xAOD::FlowElementContainer',
                                                   'CSSKGChargedParticleFlowObjectsAux': 'xAOD::ShallowAuxContainer',
                                                   'UFO': 'xAOD::FlowElementContainer',
                                                   'UFOAux': 'xAOD::FlowElementAuxContainer',
                                                   'Kt4UFOCSSKEventShape': 'xAOD::EventShape',
                                                   'Kt4UFOCSSKEventShapeAux': 'xAOD::EventShapeAuxInfo',
                                                   'Kt4UFOCSSKNeutEventShape': 'xAOD::EventShape',
                                                   'Kt4UFOCSSKNeutEventShapeAux': 'xAOD::EventShapeAuxInfo',
                                                   'AntiKt4EMTopoNoPtCutJets': 'xAOD::JetContainer',
                                                   'AntiKt4EMTopoNoPtCutJetsAux': 'xAOD::JetAuxContainer'})

    from DerivationFrameworkJetEtMiss.JetCommonConfig import addOriginCorrectedClustersToSlimmingTool
    addOriginCorrectedClustersToSlimmingTool(JETM2SlimmingHelper,writeLC=True,writeEM=True)

    # Truth containers
    if flags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(JETM2SlimmingHelper)

        JETM2SlimmingHelper.AppendToDictionary.update({'TruthParticles': 'xAOD::TruthParticleContainer',
                                                       'TruthParticlesAux': 'xAOD::TruthParticleAuxContainer'})
        
        JETM2SlimmingHelper.AllVariables += ["TruthTopQuarkWithDecayParticles","TruthTopQuarkWithDecayVertices","TruthHFWithDecayParticles",
                                             "AntiKt4TruthJets", "InTimeAntiKt4TruthJets", "OutOfTimeAntiKt4TruthJets",
                                             "TruthParticles", "TruthVertices","TruthEvents"]
        JETM2SlimmingHelper.ExtraVariables += ["AntiKt10TruthSoftDropBeta100Zcut10Jets.SizeParameter"]
        JETM2SlimmingHelper.SmartCollections += ["AntiKt4TruthJets","AntiKt10TruthJets","AntiKt4TruthWZJets","AntiKt10TruthDressedWZSoftDropBeta100Zcut10Jets"]

        JETM2SlimmingHelper.ExtraVariables += ["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.GhostTQuarksFinalCount.GhostHBosonsCount.GhostZBosonsCount.GhostWBosonsCount",
                                               "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.GhostTQuarksFinalPt.GhostHBosonsPt.GhostZBosonsPt.GhostWBosonsPt",
                                               "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.GhostBHadronsFinalPt.GhostCHadronsFinalPt",
                                               "AntiKt4TruthWZJets.IsoFixedCone5Pt",
                                               "AntiKt4TruthDressedWZJets.IsoFixedCone5Pt"]

    # Trigger content
    JETM2SlimmingHelper.IncludeTriggerNavigation = False
    JETM2SlimmingHelper.IncludeJetTriggerContent = False
    JETM2SlimmingHelper.IncludeMuonTriggerContent = False
    JETM2SlimmingHelper.IncludeEGammaTriggerContent = False
    JETM2SlimmingHelper.IncludeTauTriggerContent = False
    JETM2SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM2SlimmingHelper.IncludeBJetTriggerContent = False
    JETM2SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM2SlimmingHelper.IncludeMinBiasTriggerContent = False

    jetOutputList = ["AntiKt4UFOCSSKNoPtCutJets","AntiKt4EMPFlowNoPtCutJets"]
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addJetsToSlimmingTool
    addJetsToSlimmingTool(JETM2SlimmingHelper, jetOutputList, JETM2SlimmingHelper.SmartCollections)

    # Output stream
    JETM2ItemList = JETM2SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_JETM2", ItemList=JETM2ItemList, AcceptAlgs=["JETM2Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_JETM2", AcceptAlgs=["JETM2Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

