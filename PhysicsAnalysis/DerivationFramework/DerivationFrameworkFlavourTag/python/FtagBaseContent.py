"""
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

FtagBaseContent.py
This module contains common configuration used by PHYSVAL, FTAG1 and FTAG2.
Most of the configuration of which variables to save is handled by the
smart slimming lists, whhich are defined in BTaggingContent.py. New variables
should be added there, not here.
"""

from DerivationFrameworkFlavourTag.FtagDerivationConfig import (
    ParentDecoratorCfg, trackTruthDecorator, truthVertexDecorator
)
from JetTagDerivationUtils.JetMatchingConfig import JetMatchingCfg

from DerivationFrameworkFlavourTag.FlowEnergyDecoratorConfig import FlowEnergyDecoratorCfg

## Common items used in PHYSVAL, FTAG1 and FTAG2
PHYSVAL_FTAG1_FTAG2_SmartCollections = [
    "Electrons",
    "Muons",
    "PrimaryVertices",
    "InDetTrackParticles",
    "AntiKt4EMPFlowJets",
    "AntiKt4TruthJets",
    "MET_Baseline_AntiKt4EMPFlow",
    "TauJets",
]

PHYSVAL_FTAG1_FTAG2_AllVariables = [
    "EventInfo",
    "PrimaryVertices",
    "InDetTrackParticles",
    "TruthBottom", "TruthElectrons","TruthMuons","TruthTaus",
]

PHYSVAL_FTAG1_FTAG2_mc_AppendToDictionary = {}

PHYSVAL_FTAG1_FTAG2_ExtraVariables = [
    "AntiKt10UFOCSSKJetsAux.GhostTrack",
    "Electrons.TruthLink",
    "Muons.TruthLink.segmentDeltaPhi.segmentDeltaEta.ParamEnergyLoss.ParamEnergyLossSigmaPlus.ParamEnergyLossSigmaMinus.MeasEnergyLoss.MeasEnergyLossSigma",
    "Photons.TruthLink",
    "AntiKt2PV0TrackJets.pt.eta.phi.m",
    "AntiKt4EMTopoJets.PartonTruthLabelID.GhostBHadronsFinalPt",
    "AntiKt4EMPFlowJets.DFCommonJets_fJvt.GhostBHadronsFinalPt.SumPtChargedPFOPt1000.SumPtTrkPt1000.TrackSumMass.TrackSumPt.TrackWidthPt500.TracksForBTagging.JetEMScaleMomentum_pt.JetEMScaleMomentum_eta.HECQuality.GhostHBosonsPt.GNNVerticesLink.InclusiveGNNVerticesLink",
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


_FTAG_JET_NUM_VERTEX_VARS = (
    ".ftagJetNumBVertices"
    ".ftagJetNumCVertices"
    ".ftagJetNumTauVertices"
    ".ftagJetNumStrangeVertices"
    ".ftagJetNumPionVertices"
    ".ftagJetNumMaterialIntVertices"
    ".ftagJetNumOtherVertices"
    ".ftagJetNumVertices"
)


def addTruthVertexDecorations(flags, cfg, helper, target="AntiKt4EMPFlowJets",
                              largeRJetCollection=None):
    """schedule truth-vertex decorators and add their slimming variables.

    Opt-in per derivation (e.g. FTAG1) rather than common, because not every
    derivation wants the truth-vertex content.

    Args:
        target: small-R jet collection (decorated with dR=0.4 matching).
        largeRJetCollection: optional large-R jet collection (decorated with
            dR=1.0 matching).
    """
    if not flags.Input.isMC:
        return

    jet_collections = [(target, 0.4)]
    if largeRJetCollection is not None:
        jet_collections.append((largeRJetCollection, 1.0))

    cfg.merge(truthVertexDecorator(flags, jetCollections=jet_collections))

    # TruthParticles is in AllVariables so its decorations are saved automatically
    helper.ExtraVariables += [
        "InDetTrackParticles.ftagTrackDecayVertexID"
        ".ftagTrackDecayVertexType"
        ".ftagTrackDecaySimpleVertexType"
        ".trackPDGID.trackParentPDGID",
    ]
    for jet_container, _ in jet_collections:
        helper.ExtraVariables += [jet_container + _FTAG_JET_NUM_VERTEX_VARS]


def addCommonAugmentation(flags, cfg, helper, target="AntiKt4EMPFlowJets"):
    """add content common to all ftag derivations"""

    cfg.merge(
        JetMatchingCfg(
            flags,
            target=target,
            ints_to_copy=_int_labels(flags),
        )
    )
    helper.ExtraVariables +=  [
        '.'.join([target] + _match_vars(flags, target))
    ]

    if not flags.Input.isMC:
        return

    # add track truth info
    cfg.merge(trackTruthDecorator(flags))

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

    helper.ExtraVariables += ['.'.join([target] + truth_labels)]

    if not flags.HeavyIon.isDerivation:
        # add flow energy decorator
        cfg.merge(
            FlowEnergyDecoratorCfg(
            )
        )
