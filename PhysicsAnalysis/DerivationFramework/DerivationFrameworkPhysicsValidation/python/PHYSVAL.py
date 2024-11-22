# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_PHYSVAL.py
# This defines DAOD_PHYSVAL, an unskimmed DAOD format for
# Run 3 validation. It contains the variables and objects needed for
# most physics validation tasks in ATLAS.
# It requires the flag PHYSVAL in Derivation_tf.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from DerivationFrameworkEGamma.ElectronsCPDetailedContent import GSFTracksCPDetailedContent
from AthenaConfiguration.Enums import LHCPeriod, MetadataCategory

# Main algorithm config
def PHYSVALKernelCfg(flags, name='PHYSVALKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for PHYSVAL"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # LLP-specific configs
    if flags.Tracking.doLargeD0:
        from DerivationFrameworkLLP.PhysValLLPConfig import PhysValLLPCfg
        acc.merge(PhysValLLPCfg(flags))

        # LRT Egamma
        from DerivationFrameworkEGamma.EGammaLRTConfig import EGammaLRTCfg
        acc.merge(EGammaLRTCfg(flags))

        from DerivationFrameworkLLP.LLPToolsConfig import LRTElectronLHSelectorsCfg
        acc.merge(LRTElectronLHSelectorsCfg(flags))

        # LRT Muons
        from DerivationFrameworkMuons.MuonsCommonConfig import MuonsCommonCfg
        acc.merge(MuonsCommonCfg(flags,
                                suff="LRT"))

    # R = 0.4 LCTopo jets (for tau validation)
    from JetRecConfig.StandardSmallRJets import AntiKt4LCTopo
    from JetRecConfig.JetRecConfig import JetRecCfg

    acc.merge(JetRecCfg(flags,AntiKt4LCTopo))

    # Kernel algorithm
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name))
    return acc


def PHYSVALCfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    PHYSVALTriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(PHYSVALKernelCfg(flags, name="PHYSVALKernel", StreamName = 'StreamDAOD_PHYSVAL', TriggerListsHelper = PHYSVALTriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    PHYSVALSlimmingHelper = SlimmingHelper("PHYSVALSlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    PHYSVALSlimmingHelper.SmartCollections = ["EventInfo",
                                              "Electrons",
                                              "LRTElectrons",
                                              "Photons",
                                              "Muons",
                                              "MuonsLRT",
                                              "PrimaryVertices",
                                              "InDetTrackParticles",
                                              "InDetLargeD0TrackParticles",
                                              "AntiKt4EMTopoJets",
                                              "AntiKt4EMPFlowJets",
                                              "AntiKt4LCTopoJets",
                                              "BTagging_AntiKt4EMPFlow",
                                              "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                              "MET_Baseline_AntiKt4EMTopo",
                                              "MET_Baseline_AntiKt4EMPFlow",
                                              "TauJets",
                                              "DiTauJets",
                                              "DiTauJetsLowPt",
                                              "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                              "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                              "AntiKtVR30Rmax4Rmin02PV0TrackJets"]

    PHYSVALSlimmingHelper.AllVariables =  ["EventInfo",
                                           "Electrons", "ForwardElectrons","LRTElectrons",
                                           "Photons",
                                           "Muons", "CombinedMuonTrackParticles","ExtrapolatedMuonTrackParticles",
                                           "MuonsLRT",
                                           "MuonSpectrometerTrackParticles","MSOnlyExtrapolatedMuonTrackParticles","MuonSegments",
                                           "PrimaryVertices",
                                           "InDetTrackParticles","InDetForwardTrackParticles",
                                           "InDetLargeD0TrackParticles",
                                           "AntiKt4EMTopoJets",
                                           "AntiKt4EMPFlowJets",
                                           "AntiKt4LCTopoJets",
                                           "BTagging_AntiKt4EMPFlow",
                                           "BTagging_AntiKt4EMTopo",
                                           "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                           "BTagging_AntiKt4EMPFlowJFVtx",
                                           "BTagging_AntiKt4EMPFlowJFVtxFlip", #Flip version of JetFitter
                                           "BTagging_AntiKt4EMPFlowSecVtx",
                                           "BTagging_AntiKt4EMPFlowSecVtxFlip", #Flip version of SV1
                                           "TauJets",
                                           "TauTracks",
                                           "DiTauJets",
                                           "DiTauJetsLowPt",
                                           "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets","AntiKt10LCTopoJets","AntiKt4LCTopoJets","AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                           "TruthParticles", "TruthEvents", "TruthVertices", "MuonTruthParticles", "egammaTruthParticles",
                                           "MuonTruthSegments",
                                           "MET_Truth","MET_TruthRegions",
                                           "TruthElectrons","TruthMuons","TruthPhotons","TruthTaus","TruthNeutrinos","TruthBSM","TruthTop","TruthBoson",
                                           "CaloCalTopoClusters", "EMOriginTopoClusters","LCOriginTopoClusters",
                                           "JetETMissChargedParticleFlowObjects", "JetETMissNeutralParticleFlowObjects"]

    if flags.GeoModel.Run >= LHCPeriod.Run4:
        PHYSVALSlimmingHelper.AllVariables += ["BTagging_AntiKt4EMTopoJFVtx",
                                               "BTagging_AntiKt4EMTopoJFVtxFlip", #Flip version of JetFitter
                                               "BTagging_AntiKt4EMTopoSecVtx",
                                               "BTagging_AntiKt4EMTopoSecVtxFlip"] #Flip version of SV1

    # TODO: for now we don't have a ConfigFlag for this so it is set to False
    AddPseudoTracks = False
    if AddPseudoTracks:
        PseudoTrackContainers = [
            "InDetPseudoTrackParticles",
            "InDetReplacedWithPseudoTrackParticles",
            "InDetReplacedWithPseudoFromBTrackParticles",
            "InDetReplacedWithPseudoNotFromBTrackParticles",
            "InDetPlusPseudoTrackParticles",
            "InDetPlusPseudoFromBTrackParticles",
            "InDetPlusPseudoNotFromBTrackParticles",
            "InDetNoFakesTrackParticles",
            "InDetNoFakesFromBTrackParticles",
            "InDetNoFakesNotFromBTrackParticles",
            "InDetSiSPSeededTracksParticles"]
        PHYSVALSlimmingHelper.SmartCollections += PseudoTrackContainers
        PHYSVALSlimmingHelper.AllVariables += PseudoTrackContainers

    excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    StaticContent = []
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexAuxContainer#BTagging_AntiKt4EMPFlowSecVtxAux.-vxTrackAtVertex"]
    for wp in ["","_LeptonsMod_LRTR3_1p0"]:
        StaticContent += ["xAOD::VertexContainer#VrtSecInclusive_SecondaryVertices" + wp]
        StaticContent += ["xAOD::VertexAuxContainer#VrtSecInclusive_SecondaryVertices" + wp + "Aux."]

    if flags.BTagging.RunFlipTaggers is True:
        StaticContent += ["xAOD::VertexAuxContainer#BTagging_AntiKt4EMPFlowSecVtxFlipAux.-vxTrackAtVertex"]

    if flags.GeoModel.Run >= LHCPeriod.Run4:
        StaticContent += ["xAOD::VertexAuxContainer#BTagging_AntiKt4EMTopoSecVtxAux.-vxTrackAtVertex"]
        if flags.BTagging.RunFlipTaggers is True:
            StaticContent += ["xAOD::VertexAuxContainer#BTagging_AntiKt4EMTopoSecVtxFlipAux.-vxTrackAtVertex"]

 
    PHYSVALSlimmingHelper.StaticContent = StaticContent

    # Truth containers
    if flags.Input.isMC:
        PHYSVALSlimmingHelper.AppendToDictionary = {'TruthEvents':'xAOD::TruthEventContainer','TruthEventsAux':'xAOD::TruthEventAuxContainer',
                                                    'MET_Truth':'xAOD::MissingETContainer','MET_TruthAux':'xAOD::MissingETAuxContainer',
                                                    'TruthElectrons':'xAOD::TruthParticleContainer','TruthElectronsAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthMuons':'xAOD::TruthParticleContainer','TruthMuonsAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthPhotons':'xAOD::TruthParticleContainer','TruthPhotonsAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthTaus':'xAOD::TruthParticleContainer','TruthTausAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthNeutrinos':'xAOD::TruthParticleContainer','TruthNeutrinosAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthBSM':'xAOD::TruthParticleContainer','TruthBSMAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthBoson':'xAOD::TruthParticleContainer','TruthBosonAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthTop':'xAOD::TruthParticleContainer','TruthTopAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthForwardProtons':'xAOD::TruthParticleContainer','TruthForwardProtonsAux':'xAOD::TruthParticleAuxContainer',
                                                    'BornLeptons':'xAOD::TruthParticleContainer','BornLeptonsAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthBosonsWithDecayParticles':'xAOD::TruthParticleContainer','TruthBosonsWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthBosonsWithDecayVertices':'xAOD::TruthVertexContainer','TruthBosonsWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                                    'TruthBSMWithDecayParticles':'xAOD::TruthParticleContainer','TruthBSMWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthBSMWithDecayVertices':'xAOD::TruthVertexContainer','TruthBSMWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                                    'TruthHFWithDecayParticles':'xAOD::TruthParticleContainer','TruthHFWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthHFWithDecayVertices':'xAOD::TruthVertexContainer','TruthHFWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                                    'TruthCharm':'xAOD::TruthParticleContainer','TruthCharmAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthPrimaryVertices':'xAOD::TruthVertexContainer','TruthPrimaryVerticesAux':'xAOD::TruthVertexAuxContainer',
                                                    'AntiKt10TruthTrimmedPtFrac5SmallR20Jets':'xAOD::JetContainer', 'AntiKt10TruthTrimmedPtFrac5SmallR20JetsAux':'xAOD::JetAuxContainer',
                                                    'AntiKt10LCTopoJets':'xAOD::JetContainer', 'AntiKt10LCTopoJetsAux':'xAOD::JetAuxContainer',
                                                    'BTagging_AntiKtVR30Rmax4Rmin02Track':'xAOD::BTaggingContainer','BTagging_AntiKtVR30Rmax4Rmin02TrackAux':'xAOD::BTaggingAuxContainer',
                                                    'EMOriginTopoClusters':'xAOD::CaloClusterContainer', 'EMOriginTopoClustersAux':'xAOD::ShallowAuxContainer',
                                                    'LCOriginTopoClusters':'xAOD::CaloClusterContainer', 'LCOriginTopoClustersAux':'xAOD::ShallowAuxContainer',
                                                    'BTagging_AntiKt4EMPFlowJFVtx':'xAOD::BTagVertexContainer','BTagging_AntiKt4EMPFlowJFVtxAux':'xAOD::BTagVertexAuxContainer',
                                                    'BTagging_AntiKt4EMPFlowSecVtx':'xAOD::VertexContainer','BTagging_AntiKt4EMPFlowSecVtxAux':'xAOD::VertexAuxContainer',
                                                    'GlobalChargedParticleFlowObjects':'xAOD::FlowElementContainer','GlobalChargedParticleFlowObjectsAux':'xAOD::FlowElementAuxContainer',
                                                    'GlobalNeutralParticleFlowObjects':'xAOD::FlowElementContainer', 'GlobalNeutralParticleFlowObjectsAux':'xAOD::FlowElementAuxContainer',
                                                    'CHSGChargedParticleFlowObjects': 'xAOD::FlowElementContainer', 'CHSGChargedParticleFlowObjectsAux':'xAOD::ShallowAuxContainer',
                                                    'CHSGNeutralParticleFlowObjects': 'xAOD::FlowElementContainer', 'CHSGNeutralParticleFlowObjectsAux':'xAOD::ShallowAuxContainer',
                                                    'BTagging_AntiKt4EMPFlowJFVtxFlip':'xAOD::BTagVertexContainer','BTagging_AntiKt4EMPFlowJFVtxFlipAux':'xAOD::BTagVertexAuxContainer',#For Flip version of JetFitter
                                                    'BTagging_AntiKt4EMPFlowSecVtxFlip':'xAOD::VertexContainer','BTagging_AntiKt4EMPFlowSecVtxFlipAux':'xAOD::VertexAuxContainer'}

        if flags.GeoModel.Run >= LHCPeriod.Run4:
            PHYSVALSlimmingHelper.AppendToDictionary.update({'BTagging_AntiKt4EMTopoJFVtx':'xAOD::BTagVertexContainer','BTagging_AntiKt4EMTopoJFVtxAux':'xAOD::BTagVertexAuxContainer',
                                                             'BTagging_AntiKt4EMTopoSecVtx':'xAOD::VertexContainer','BTagging_AntiKt4EMTopoSecVtxAux':'xAOD::VertexAuxContainer',
                                                             'BTagging_AntiKt4EMTopoJFVtxFlip':'xAOD::BTagVertexContainer','BTagging_AntiKt4EMTopoJFVtxFlipAux':'xAOD::BTagVertexAuxContainer',#For Flip version of JetFitter
                                                             'BTagging_AntiKt4EMTopoSecVtxFlip':'xAOD::VertexContainer','BTagging_AntiKt4EMTopoSecVtxFlipAux':'xAOD::VertexAuxContainer'})

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(PHYSVALSlimmingHelper)
        PHYSVALSlimmingHelper.AllVariables += ['TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm','TruthPileupEvents','TruthPileupParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']
        PHYSVALSlimmingHelper.SmartCollections += ['AntiKt4TruthJets']

        if flags.Trigger.EDMVersion == 3:
            # Decorate Run 3 HLT jets with truth labels
            from DerivationFrameworkFlavourTag.FtagDerivationConfig import HLTJetFTagDecorationCfg
            acc.merge(HLTJetFTagDecorationCfg(flags))

        # End of isMC clause

    PHYSVALSlimmingHelper.ExtraVariables += ["AntiKt10TruthTrimmedPtFrac5SmallR20Jets.Tau1_wta.Tau2_wta.Tau3_wta.D2.GhostBHadronsFinalCount",
                                             "Electrons.TruthLink","LRTElectrons.TruthLink",
                                             "Muons.TruthLink","MuonsLRT.TruthLink",
                                             "Photons.TruthLink",
                                             "AntiKt2PV0TrackJets.pt.eta.phi.m",
                                             "AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.PartonTruthLabelID",
                                             "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.PartonTruthLabelID.DFCommonJets_fJvt",
                                             "TruthPrimaryVertices.t.x.y.z",
                                             "TauNeutralParticleFlowObjects.pt.eta.phi.m.bdtPi0Score.nPi0Proto",
                                             "TauChargedParticleFlowObjects.pt.eta.phi.m.bdtPi0Score",
                                             "MET_Track.sumet"]
    PHYSVALSlimmingHelper.ExtraVariables += GSFTracksCPDetailedContent

    VSITrackAuxVars = [
        "is_selected", "is_associated", "is_svtrk_final", "pt_wrtSV", "eta_wrtSV",
        "phi_wrtSV", "d0_wrtSV", "z0_wrtSV", "errP_wrtSV", "errd0_wrtSV",
        "errz0_wrtSV", "chi2_toSV"
    ]

    for suffix in ["","_LeptonsMod_LRTR3_1p0"]:
        PHYSVALSlimmingHelper.ExtraVariables += [ "InDetTrackParticles." + '.'.join( [ var + suffix for var in VSITrackAuxVars] ) ]
        PHYSVALSlimmingHelper.ExtraVariables += [ "InDetLargeD0TrackParticles." + '.'.join( [ var + suffix for var in VSITrackAuxVars] ) ]
        PHYSVALSlimmingHelper.ExtraVariables += [ "GSFTrackParticles." + '.'.join( [ var + suffix for var in VSITrackAuxVars] ) ]
        PHYSVALSlimmingHelper.ExtraVariables += [ "LRTGSFTrackParticles." + '.'.join( [ var + suffix for var in VSITrackAuxVars] ) ]


    # Trigger content
    PHYSVALSlimmingHelper.IncludeTriggerNavigation          = True
    PHYSVALSlimmingHelper.IncludeJetTriggerContent          = True
    PHYSVALSlimmingHelper.IncludeMuonTriggerContent         = True
    PHYSVALSlimmingHelper.IncludeEGammaTriggerContent       = True
    PHYSVALSlimmingHelper.IncludeTauTriggerContent          = True
    PHYSVALSlimmingHelper.IncludeEtMissTriggerContent       = True
    PHYSVALSlimmingHelper.IncludeBJetTriggerContent         = True
    PHYSVALSlimmingHelper.IncludeBPhysTriggerContent        = True
    PHYSVALSlimmingHelper.IncludeMinBiasTriggerContent      = True

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = PHYSVALSlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = PHYSVALTriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = PHYSVALSlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = PHYSVALTriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(PHYSVALSlimmingHelper)

    # Full trigger content (needed for T0-style monitoring)
    PHYSVALSlimmingHelper.IncludeFullTriggerEDMLevel = "ESD"

    # Output stream
    PHYSVALItemList = PHYSVALSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_PHYSVAL", ItemList=PHYSVALItemList, AcceptAlgs=["PHYSVALKernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_PHYSVAL", AcceptAlgs=["PHYSVALKernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

