"""
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

FtagBaseContent.py
This module contains common configuration used by PHYSVAL, FTAG1 and FTAG2.
Most of the configuration of which variables to save is handled by the 
smart slimming lists, whhich are defined in BTaggingContent.py. New variables
should be added there, not here.
"""


## Common items used in PHYSVAL, FTAG1 and FTAG2
PHYSVAL_FTAG1_FTAG2_mc_AppendToDictionary = {}

PHYSVAL_FTAG1_FTAG2_ExtraVariables = [
    "AntiKt10TruthTrimmedPtFrac5SmallR20Jets.Tau1_wta.Tau2_wta.Tau3_wta.D2.GhostBHadronsFinalCount",
    "Electrons.TruthLink",
    "Muons.TruthLink.segmentDeltaPhi.segmentDeltaEta.ParamEnergyLoss.ParamEnergyLossSigmaPlus.ParamEnergyLossSigmaMinus.MeasEnergyLoss.MeasEnergyLossSigma",
    "Photons.TruthLink",
    "AntiKt2PV0TrackJets.pt.eta.phi.m",
    "AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.PartonTruthLabelID.GhostBHadronsFinalPt",
    "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.DFCommonJets_fJvt.GhostBHadronsFinalPt.SumPtChargedPFOPt1000.SumPtTrkPt1000.TrackSumMass.TrackSumPt.TrackWidthPt500.TracksForBTagging.TruthLabelDeltaR_B.TruthLabelDeltaR_C.TruthLabelDeltaR_T.JetEMScaleMomentum_pt.JetEMScaleMomentum_eta.HECQuality.GhostHBosonsPt",
    "TruthPrimaryVertices.t.x.y.z",
    "TauNeutralParticleFlowObjects.pt.eta.phi.m.bdtPi0Score.nPi0Proto",
    "TauChargedParticleFlowObjects.pt.eta.phi.m.bdtPi0Score",
    "MET_Track.sumet",
]

excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
PHYSVAL_FTAG1_FTAG2_StaticContent = []
PHYSVAL_FTAG1_FTAG2_StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
PHYSVAL_FTAG1_FTAG2_StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
PHYSVAL_FTAG1_FTAG2_StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
PHYSVAL_FTAG1_FTAG2_StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
PHYSVAL_FTAG1_FTAG2_StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
PHYSVAL_FTAG1_FTAG2_StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData]
PHYSVAL_FTAG1_FTAG2_StaticContent += ["xAOD::VertexAuxContainer#BTagging_AntiKt4EMPFlowSecVtxAux.-vxTrackAtVertex"]


## Common functions used in PHYSVAL, FTAG1 and FTAG2
def add_static_content_to_SlimmingHelper(SlimmingHelper, extra_StaticContent=[]):
    all_StaticContent = PHYSVAL_FTAG1_FTAG2_StaticContent
    if len(extra_StaticContent) > 0:
        all_StaticContent += extra_StaticContent
    SlimmingHelper.StaticContent = all_StaticContent

def add_truth_to_SlimmingHelper(SlimmingHelper):
    from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
    if len(PHYSVAL_FTAG1_FTAG2_mc_AppendToDictionary)>0:
        SlimmingHelper.AppendToDictionary.update(PHYSVAL_FTAG1_FTAG2_mc_AppendToDictionary)
    addTruth3ContentToSlimmerTool(SlimmingHelper)
    SlimmingHelper.AllVariables += ['TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm','TruthPileupParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']

def add_ExtraVariables_to_SlimmingHelper(SlimmingHelper):
    SlimmingHelper.ExtraVariables += PHYSVAL_FTAG1_FTAG2_ExtraVariables
    from DerivationFrameworkEGamma.ElectronsCPDetailedContent import GSFTracksCPDetailedContent
    SlimmingHelper.ExtraVariables += GSFTracksCPDetailedContent

## Common function used in FTAG1 and FTAG2
def trigger_setup(SlimmingHelper, option=''):
    SlimmingHelper.IncludeTriggerNavigation = False
    SlimmingHelper.IncludeJetTriggerContent = False
    SlimmingHelper.IncludeMuonTriggerContent = False
    SlimmingHelper.IncludeEGammaTriggerContent = False
    SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    SlimmingHelper.IncludeTauTriggerContent = False
    SlimmingHelper.IncludeEtMissTriggerContent = False
    SlimmingHelper.IncludeBJetTriggerContent = False
    SlimmingHelper.IncludeBPhysTriggerContent = False
    SlimmingHelper.IncludeMinBiasTriggerContent = False
    if option == 'FTAG2':
        SlimmingHelper.IncludeMuonTriggerContent = True
        SlimmingHelper.IncludeEGammaTriggerContent = True
        SlimmingHelper.IncludeBJetTriggerContent = True
        SlimmingHelper.IncludeBPhysTriggerContent = True
    if option == 'FTAG3':
        SlimmingHelper.IncludeJetTriggerContent = True
        SlimmingHelper.FinalItemList.append('xAOD::JetContainer#HLT_xAOD__JetContainer_a4tcemsubjesFS')
        SlimmingHelper.FinalItemList.append('xAOD::JetTrigAuxContainer#HLT_xAOD__JetContainer_a4tcemsubjesFSAux.')
        SlimmingHelper.FinalItemList.append('xAOD::JetContainer#HLT_xAOD__JetContainer_a4tcemsubjesISFS')
        SlimmingHelper.FinalItemList.append('xAOD::JetTrigAuxContainer#HLT_xAOD__JetContainer_a4tcemsubjesISFSAux.')
        SlimmingHelper.FinalItemList.append('xAOD::JetContainer#HLT_xAOD__JetContainer_a10tclcwsubjesFS')
        SlimmingHelper.FinalItemList.append('xAOD::JetTrigAuxContainer#HLT_xAOD__JetContainer_a10tclcwsubjesFSAux.')
        SlimmingHelper.FinalItemList.append('xAOD::JetContainer#HLT_xAOD__JetContainer_a10ttclcwjesFS')
        SlimmingHelper.FinalItemList.append('xAOD::JetTrigAuxContainer#HLT_xAOD__JetContainer_a10ttclcwjesFSAux.')


def trigger_matching(SlimmingHelper, TriggerListsHelper, ConfigFlags):
    # Trigger matching
    # Run 2
    if ConfigFlags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = SlimmingHelper,
                OutputContainerPrefix = "TrigMatch_",
                TriggerList = TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = SlimmingHelper,
                OutputContainerPrefix = "TrigMatch_",
                TriggerList = TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3
    if ConfigFlags.Trigger.EDMVersion == 3:
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(SlimmingHelper)
        # Run 2 is added here temporarily to allow testing/comparison/debugging
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = SlimmingHelper,
                OutputContainerPrefix = "TrigMatch_",
                TriggerList = TriggerListsHelper.Run3TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = SlimmingHelper,
                OutputContainerPrefix = "TrigMatch_",
                TriggerList = TriggerListsHelper.Run3TriggerNamesNoTau)

