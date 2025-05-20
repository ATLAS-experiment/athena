"""
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

FtagBaseContent.py
This module contains common configuration used by PHYSVAL, FTAG1 and FTAG2.
Most of the configuration of which variables to save is handled by the 
smart slimming lists, whhich are defined in BTaggingContent.py. New variables
should be added there, not here.
"""

from DerivationFrameworkFlavourTag.FtagDerivationConfig import (
    ParentDecoratorCfg
)
from JetTagDerivationUtils.JetMatchingConfig import JetMatchingCfg

## Common items used in PHYSVAL, FTAG1 and FTAG2
PHYSVAL_FTAG1_FTAG2_SmartCollections = [
    "Electrons",
    "Muons",
    "PrimaryVertices",
    "InDetTrackParticles",
    "AntiKt4EMPFlowJets",
    "AntiKt4TruthJets",
    "BTagging_AntiKt4EMPFlow",
    "MET_Baseline_AntiKt4EMPFlow",
    "TauJets",
]

PHYSVAL_FTAG1_FTAG2_AllVariables = [
    "EventInfo",
    "PrimaryVertices",
    "InDetTrackParticles",
    "BTagging_AntiKt4EMPFlow",
    "BTagging_AntiKt4EMPFlowJFVtx",
    "BTagging_AntiKt4EMPFlowSecVtx",
    "TruthBottom", "TruthElectrons","TruthMuons","TruthTaus",
]

PHYSVAL_FTAG1_FTAG2_mc_AppendToDictionary = {}

PHYSVAL_FTAG1_FTAG2_ExtraVariables = [
    "AntiKt10UFOCSSKJetsAux.GhostTrack",
    "AntiKt10TruthTrimmedPtFrac5SmallR20Jets.Tau1_wta.Tau2_wta.Tau3_wta.D2.GhostBHadronsFinalCount",
    "Electrons.TruthLink",
    "Muons.TruthLink.segmentDeltaPhi.segmentDeltaEta.ParamEnergyLoss.ParamEnergyLossSigmaPlus.ParamEnergyLossSigmaMinus.MeasEnergyLoss.MeasEnergyLossSigma",
    "Photons.TruthLink",
    "AntiKt2PV0TrackJets.pt.eta.phi.m",
    "AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.PartonTruthLabelID.GhostBHadronsFinalPt",
    "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.DFCommonJets_fJvt.GhostBHadronsFinalPt.SumPtChargedPFOPt1000.SumPtTrkPt1000.TrackSumMass.TrackSumPt.TrackWidthPt500.TracksForBTagging.JetEMScaleMomentum_pt.JetEMScaleMomentum_eta.HECQuality.GhostHBosonsPt.GNNVerticesLink.InclusiveGNNVerticesLink",
    "TruthPrimaryVertices.t.x.y.z",
    "TauNeutralParticleFlowObjects.pt.eta.phi.m.bdtPi0Score.nPi0Proto",
    "TauChargedParticleFlowObjects.pt.eta.phi.m",
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
def update_AppendToDictionary_in_SlimmingHelper(SlimmingHelper, flags, extra_AppendToDictionary={}):
    if flags.BTagging.RunNewVrtSecInclusive:
        SlimmingHelper.AppendToDictionary.update({'NVSI_SecVrt_Tight' : 'xAOD::VertexContainer','NVSI_SecVrt_TightAux' : 'xAOD::VertexAuxContainer',
                                                       'NVSI_SecVrt_Medium' : 'xAOD::VertexContainer','NVSI_SecVrt_MediumAux' : 'xAOD::VertexAuxContainer',
                                                       'NVSI_SecVrt_Loose' : 'xAOD::VertexContainer','NVSI_SecVrt_LooseAux' : 'xAOD::VertexAuxContainer'})

    if len(extra_AppendToDictionary)>0:
        SlimmingHelper.AppendToDictionary.update(extra_AppendToDictionary)

def add_static_content_to_SlimmingHelper(SlimmingHelper, flags, extra_StaticContent=[]):
    all_StaticContent = PHYSVAL_FTAG1_FTAG2_StaticContent
    excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    if flags.BTagging.GNNVertexFitter:
        all_StaticContent += ["xAOD::VertexContainer#GNNVertices"]
        all_StaticContent += ["xAOD::VertexAuxContainer#GNNVerticesAux."+excludedVertexAuxData]
        all_StaticContent += ["xAOD::VertexContainer#InclusiveGNNVertices"]
        all_StaticContent += ["xAOD::VertexAuxContainer#InclusiveGNNVerticesAux."+excludedVertexAuxData]
    if flags.BTagging.RunNewVrtSecInclusive:
        excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
        all_StaticContent += ["xAOD::VertexContainer#NVSI_SecVrt_Loose", "xAOD::VertexContainer#NVSI_SecVrt_Medium", "xAOD::VertexContainer#NVSI_SecVrt_Tight"]
        all_StaticContent += ["xAOD::VertexAuxContainer#NVSI_SecVrt_LooseAux."+excludedVertexAuxData]
        all_StaticContent += ["xAOD::VertexAuxContainer#NVSI_SecVrt_MediumAux."+excludedVertexAuxData ]
        all_StaticContent += ["xAOD::VertexAuxContainer#NVSI_SecVrt_TightAux."+excludedVertexAuxData]
    if len(extra_StaticContent) > 0:
        all_StaticContent += extra_StaticContent
    SlimmingHelper.StaticContent = all_StaticContent

def add_truth_to_SlimmingHelper(SlimmingHelper):
    from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
    if len(PHYSVAL_FTAG1_FTAG2_mc_AppendToDictionary)>0:
        SlimmingHelper.AppendToDictionary.update(PHYSVAL_FTAG1_FTAG2_mc_AppendToDictionary)
    addTruth3ContentToSlimmerTool(SlimmingHelper)
    SlimmingHelper.AllVariables += ['TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm','TruthPileupParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']

def add_ExtraVariables_to_SlimmingHelper(SlimmingHelper, flags):
    SlimmingHelper.ExtraVariables += PHYSVAL_FTAG1_FTAG2_ExtraVariables
    from DerivationFrameworkEGamma.ElectronsCPDetailedContent import GSFTracksCPDetailedContent
    SlimmingHelper.ExtraVariables += GSFTracksCPDetailedContent
    if flags.BTagging.GNNVertexFitter:
        SlimmingHelper.ExtraVariables += ["AntiKt4EMPFlowJets.GNNVerticesLink.InclusiveGNNVerticesLink"]

## Common function used in FTAG1 and FTAG2
def trigger_setup(SlimmingHelper, option=''):
    SlimmingHelper.IncludeTriggerNavigation = False
    SlimmingHelper.IncludeJetTriggerContent = False
    SlimmingHelper.IncludeMuonTriggerContent = False
    SlimmingHelper.IncludeEGammaTriggerContent = False
    SlimmingHelper.IncludeTauTriggerContent = False
    SlimmingHelper.IncludeEtMissTriggerContent = False
    SlimmingHelper.IncludeBJetTriggerContent = False
    SlimmingHelper.IncludeBPhysTriggerContent = False
    SlimmingHelper.IncludeMinBiasTriggerContent = False
    if option == 'FTAG1':
        SlimmingHelper.IncludeJetTriggerContent = True
    if option == 'FTAG2':
        SlimmingHelper.IncludeTriggerNavigation = True
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
    if option == 'FTAG5':
        SlimmingHelper.IncludeTriggerNavigation = True
        SlimmingHelper.IncludeJetTriggerContent = True


def trigger_matching(SlimmingHelper, TriggerListsHelper, ConfigFlags):
    # Run 2
    if ConfigFlags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if ConfigFlags.Trigger.EDMVersion == 3 or (ConfigFlags.Trigger.EDMVersion == 2 and ConfigFlags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(SlimmingHelper)

def add_baseline_slimming_smartcollections(SlimmingHelper):
    SlimmingHelper.SmartCollections += PHYSVAL_FTAG1_FTAG2_SmartCollections

def add_baseline_slimming_allvariables(SlimmingHelper):
    SlimmingHelper.AllVariables += PHYSVAL_FTAG1_FTAG2_AllVariables


def _int_labels(flags):
    if not flags.Input.isMC:
        return []
    algs = ['HadronConeExcl', 'HadronGhost']
    types = ['Extended', '']
    return [f'{a}{e}TruthLabelID' for a in algs for e in types]


def _match_vars(flags, source):
    labels = _int_labels(flags)
    allvars = [f'{l}From{source}' for l in labels]
    allvars += [f'delta{v}To{source}' for v in ['R', 'Pt']]
    return allvars


def addCommonAugmentation(flags, cfg, helper):
    """add content common to all ftag derivations"""

    target = "AntiKt4EMPFlowJets"

    cfg.merge(
        JetMatchingCfg(
            flags,
            target=target,
            ints_to_copy=_int_labels(flags),
        )
    )
    helper.ExtraVariables +=  [
        '.'.join(['AntiKt4EMPFlowJets'] + _match_vars(flags, target))
    ]

    if not flags.Input.isMC:
        return

    # match jets to the parent particles
    cfg.merge(
        ParentDecoratorCfg(
            flags,
            targetContainer=target,
            prefix="PFlow",
            matchDeltaR=0.3
        )
    )
    # todo add large-R jets
    truth_labels = [
        *[f"nTopTo{p}Children" for p in "BW"],
        *[f"parent{p}ParentsMask" for p in ["Higgs", "Z", "Scalar", "Top"]],
    ]

    helper.ExtraVariables += ['.'.join(['AntiKt4EMPFlowJets'] + truth_labels)]

