"""
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.

FtagBaseContent.py

Common configuration shared by PHYSVAL, FTAG1, and FTAG2.

Most of the configuration of which variables to save is handled by the
smart slimming lists defined in BTaggingContent.py. New variables should
generally be added there rather than in this module.
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from DerivationFrameworkEGamma.ElectronsCPDetailedContent import GSFTracksCPDetailedContent
from DerivationFrameworkFlavourTag.FlowEnergyDecoratorConfig import FlowEnergyDecoratorCfg
from DerivationFrameworkFlavourTag.FtagDerivationConfig import (
    ParentDecoratorCfg,
    trackTruthDecorator,
    truthVertexDecorator,
)
from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
from DerivationFrameworkPhys.TriggerMatchingCommonConfig import (
    AddRun2TriggerMatchingToSlimmingHelper,
)
from JetTagDerivationUtils.JetMatchingConfig import JetMatchingCfg
from ParticleJetTools.FtagLargeRJetTruthLabelConfig import FtagLargeRJetTruthLabelCfg
from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import (
    AddRun3TrigNavSlimmingCollectionsToSlimmingHelper,
)


if TYPE_CHECKING:
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper


def update_append_to_dictionary_in_slimming_helper(
    flags: AthConfigFlags,
    slimming_helper: SlimmingHelper,
    extra_append_to_dictionary: dict[str, str] | None = None,
) -> None:
    """Update ``AppendToDictionary`` in the slimming helper."""

    if extra_append_to_dictionary is None:
        extra_append_to_dictionary = {}

    if flags.BTagging.RunNewVrtSecInclusive:
        slimming_helper.AppendToDictionary.update(
            {
                "NVSI_SecVrt_Tight": "xAOD::VertexContainer",
                "NVSI_SecVrt_TightAux": "xAOD::VertexAuxContainer",
                "NVSI_SecVrt_Medium": "xAOD::VertexContainer",
                "NVSI_SecVrt_MediumAux": "xAOD::VertexAuxContainer",
                "NVSI_SecVrt_Loose": "xAOD::VertexContainer",
                "NVSI_SecVrt_LooseAux": "xAOD::VertexAuxContainer",
            }
        )

    if extra_append_to_dictionary:
        slimming_helper.AppendToDictionary.update(extra_append_to_dictionary)


def add_static_content_to_slimming_helper(
    flags: AthConfigFlags,
    slimming_helper: SlimmingHelper,
    extra_static_content: list[str] | None = None,
) -> None:
    """Add common ``StaticContent`` in the slimming helper."""

    if extra_static_content is None:
        extra_static_content = []

    excl_vertex_aux_data: str = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    all_static_content = [
        "xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices",
        (
            "xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux."
            f"{excl_vertex_aux_data}"
        ),
        "xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices",
        (
            "xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux."
            f"{excl_vertex_aux_data}"
        ),
        "xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices",
        (
            "xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux."
            f"{excl_vertex_aux_data}"
        ),
    ]

    if flags.BTagging.GNNVertexFitter:
        all_static_content += [
            "xAOD::VertexContainer#GNNVertices",
            f"xAOD::VertexAuxContainer#GNNVerticesAux.{excl_vertex_aux_data}",
            "xAOD::VertexContainer#InclusiveGNNVertices",
            ("xAOD::VertexAuxContainer#InclusiveGNNVerticesAux." f"{excl_vertex_aux_data}"),
        ]

    if flags.BTagging.RunNewVrtSecInclusive:
        all_static_content += [
            "xAOD::VertexContainer#NVSI_SecVrt_Loose",
            "xAOD::VertexContainer#NVSI_SecVrt_Medium",
            "xAOD::VertexContainer#NVSI_SecVrt_Tight",
            ("xAOD::VertexAuxContainer#NVSI_SecVrt_LooseAux." f"{excl_vertex_aux_data}"),
            ("xAOD::VertexAuxContainer#NVSI_SecVrt_MediumAux." f"{excl_vertex_aux_data}"),
            ("xAOD::VertexAuxContainer#NVSI_SecVrt_TightAux." f"{excl_vertex_aux_data}"),
        ]

    if extra_static_content:
        all_static_content += list(extra_static_content)

    slimming_helper.StaticContent = all_static_content


def add_truth_to_slimming_helper(slimming_helper: SlimmingHelper) -> None:
    """Add common truth content to the slimming helper."""

    # Get TRUTH3 content
    addTruth3ContentToSlimmerTool(slimming_helper)

    # Add all variabes for certain truth containers
    slimming_helper.AllVariables += [
        "TruthHFWithDecayParticles",
        "TruthHFWithDecayVertices",
        "TruthCharm",
        "TruthPileupParticles",
        "InTimeAntiKt4TruthJets",
        "OutOfTimeAntiKt4TruthJets",
    ]


def add_extra_variables_to_slimming_helper(
    flags: AthConfigFlags,
    slimming_helper: SlimmingHelper,
) -> None:
    """Add extra FTAG variables to the slimming helper."""

    slimming_helper.ExtraVariables += [
        "AntiKt10UFOCSSKJetsAux.GhostTrack",
        "Electrons.TruthLink",
        (
            "Muons.TruthLink.segmentDeltaPhi.segmentDeltaEta."
            "ParamEnergyLoss.ParamEnergyLossSigmaPlus.ParamEnergyLossSigmaMinus."
            "MeasEnergyLoss.MeasEnergyLossSigma"
        ),
        "Photons.TruthLink",
        "AntiKt2PV0TrackJets.pt.eta.phi.m",
        "AntiKt4EMTopoJets.PartonTruthLabelID.GhostBHadronsFinalPt",
        (
            "AntiKt4EMPFlowJets."
            "DFCommonJets_fJvt."
            "GhostBHadronsFinalPt."
            "SumPtChargedPFOPt1000."
            "SumPtTrkPt1000."
            "TrackSumMass."
            "TrackSumPt."
            "TrackWidthPt500."
            "TracksForBTagging."
            "JetEMScaleMomentum_pt."
            "JetEMScaleMomentum_eta."
            "HECQuality."
            "GhostHBosonsPt."
            "GNNVerticesLink."
            "InclusiveGNNVerticesLink"
        ),
        "TruthPrimaryVertices.t.x.y.z",
        "TauNeutralParticleFlowObjects.pt.eta.phi.m.bdtPi0Score.nPi0Proto",
        "TauChargedParticleFlowObjects.pt.eta.phi.m",
        "MET_Track.sumet",
    ]

    # Add GSF Track content
    slimming_helper.ExtraVariables += GSFTracksCPDetailedContent

    if flags.BTagging.GNNVertexFitter:
        slimming_helper.ExtraVariables += [
            "AntiKt4EMPFlowJets.GNNVerticesLink.InclusiveGNNVerticesLink"
        ]


def add_baseline_slimming_smartcollections(slimming_helper: SlimmingHelper) -> None:
    """Add baseline smart collections to the slimming helper."""

    slimming_helper.SmartCollections += [
        "Electrons",
        "Muons",
        "PrimaryVertices",
        "InDetTrackParticles",
        "AntiKt4EMPFlowJets",
        "AntiKt4TruthJets",
        "MET_Baseline_AntiKt4EMPFlow",
        "TauJets",
    ]


def add_baseline_slimming_allvariables(slimming_helper: SlimmingHelper) -> None:
    """Add baseline all-variable collections to the slimming helper."""

    slimming_helper.AllVariables += [
        "EventInfo",
        "PrimaryVertices",
        "InDetTrackParticles",
        "TruthBottom",
        "TruthElectrons",
        "TruthMuons",
        "TruthTaus",
    ]


def trigger_setup(slimming_helper: SlimmingHelper) -> None:
    """Configure trigger content flags for FTAG derivations."""

    # Deactivate trigger content
    slimming_helper.IncludeTriggerNavigation = False
    slimming_helper.IncludeJetTriggerContent = False
    slimming_helper.IncludeMuonTriggerContent = False
    slimming_helper.IncludeEGammaTriggerContent = False
    slimming_helper.IncludeTauTriggerContent = False
    slimming_helper.IncludeEtMissTriggerContent = False
    slimming_helper.IncludeBJetTriggerContent = False
    slimming_helper.IncludeBPhysTriggerContent = False
    slimming_helper.IncludeMinBiasTriggerContent = False

    # Activate only JetTriggerContent
    slimming_helper.IncludeJetTriggerContent = True


def trigger_matching(
    flags: AthConfigFlags,
    slimming_helper: SlimmingHelper,
    trigger_lists_helper: TriggerListsHelper,
) -> None:
    """Configure trigger matching and trigger-navigation slimming."""
    if flags.Trigger.EDMVersion == 2:
        AddRun2TriggerMatchingToSlimmingHelper(
            SlimmingHelper=slimming_helper,
            OutputContainerPrefix="TrigMatch_",
            TriggerList=trigger_lists_helper.Run2TriggerNamesTau,
        )
        AddRun2TriggerMatchingToSlimmingHelper(
            SlimmingHelper=slimming_helper,
            OutputContainerPrefix="TrigMatch_",
            TriggerList=trigger_lists_helper.Run2TriggerNamesNoTau,
        )

    if flags.Trigger.EDMVersion == 3 or (
        flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion
    ):
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(slimming_helper)


def _get_truth_label_names(flags: AthConfigFlags) -> list[str]:
    """Return internal truth-label names to copy for jet matching."""
    if not flags.Input.isMC:
        return []

    algorithms = ["HadronConeExcl", "HadronGhost"]
    suffixes = ["Extended", ""]
    return [f"{algorithm}{suffix}TruthLabelID" for algorithm in algorithms for suffix in suffixes]


def _get_matching_variable_names(flags: AthConfigFlags, source: str) -> list[str]:
    """Return jet-matching extra variable names for a given source."""
    labels = _get_truth_label_names(flags)
    variables = [f"{label}From{source}" for label in labels]
    variables += [f"delta{var}To{source}" for var in ["R", "Pt"]]
    return variables


def add_truth_vertex_decorations(
    flags: AthConfigFlags,
    acc: ComponentAccumulator,
    slimming_helper: SlimmingHelper,
    target: str = "AntiKt4EMPFlowJets",
    large_r_jet_collection: str | None = None,
) -> None:
    """Schedule truth-vertex decorators and add their slimming variables.

    Opt-in per derivation (e.g. FTAG1) rather than common, because not every
    derivation wants the truth-vertex content.
    """
    if not flags.Input.isMC:
        return

    jet_collections = [(target, 0.4)]
    if large_r_jet_collection is not None:
        jet_collections.append((large_r_jet_collection, 1.0))

    acc.merge(truthVertexDecorator(flags, jet_collections=jet_collections))

    # TruthParticles is in AllVariables so its decorations are saved automatically
    slimming_helper.ExtraVariables += [
        "InDetTrackParticles.ftagTrackDecayVertexID"
        ".ftagTrackDecayVertexType"
        ".ftagTrackDecaySimpleVertexType"
        ".trackPDGID.trackParentPDGID",
    ]
    for jet_container, _ in jet_collections:
        slimming_helper.ExtraVariables += [
            jet_container
            + (
                ".ftagJetNumBVertices"
                ".ftagJetNumCVertices"
                ".ftagJetNumTauVertices"
                ".ftagJetNumStrangeVertices"
                ".ftagJetNumPionVertices"
                ".ftagJetNumMaterialIntVertices"
                ".ftagJetNumOtherVertices"
                ".ftagJetNumVertices"
            )
        ]


def add_common_augmentation(
    flags: AthConfigFlags,
    acc: ComponentAccumulator,
    slimming_helper: SlimmingHelper,
    target: str = "AntiKt4EMPFlowJets",
) -> None:
    """Add augmentation common to all FTAG derivations."""
    acc.merge(
        JetMatchingCfg(
            flags,
            target=target,
            ints_to_copy=_get_truth_label_names(flags),
        )
    )

    slimming_helper.ExtraVariables += [
        ".".join([target] + _get_matching_variable_names(flags, target))
    ]

    if not flags.Input.isMC:
        return

    acc.merge(trackTruthDecorator(flags))
    acc.merge(
        ParentDecoratorCfg(
            flags,
            targetContainer=target,
            prefix="PFlow",
            matchDeltaR=0.3,
        )
    )

    # FTAG simplified large-R jet truth labelling (see https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/AnalysisCommon/ParticleJetTools/ParticleJetTools/FtagLargeRJetLabelEnum.h?ref_type=heads)
    acc.merge(
        FtagLargeRJetTruthLabelCfg(
            flags,
            jetCollection="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
        )
    )

    truth_labels = [
        *[f"nTopTo{particle}Children" for particle in "BW"],
        *[f"parent{particle}ParentsMask" for particle in ["Higgs", "Z", "Scalar", "Top"]],
    ]
    slimming_helper.ExtraVariables += [".".join([target] + truth_labels)]

    if not flags.HeavyIon.isDerivation:
        acc.merge(FlowEnergyDecoratorCfg())
