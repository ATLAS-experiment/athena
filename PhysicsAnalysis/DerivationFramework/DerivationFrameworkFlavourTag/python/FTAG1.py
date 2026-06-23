# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# DAOD_FTAG1.py
#
# Unskimmed DAOD format for Run 2/3 flavour-tagging studies.
# Contains objects and variables needed for most FTAG studies.
# Requires the FTAG1 flag in Derivation_tf.py
# ====================================================================

from __future__ import annotations

from typing import Any, TYPE_CHECKING

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, MetadataCategory

from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
from DerivationFrameworkEGamma.ElectronsCPDetailedContent import ElectronsCPDetailedContent
from DerivationFrameworkFlavourTag.FtagBaseContent import (
    add_common_augmentation,
    add_baseline_slimming_allvariables,
    add_baseline_slimming_smartcollections,
    add_extra_variables_to_slimming_helper,
    add_truth_to_slimming_helper,
    add_truth_vertex_decorations,
    trigger_matching,
    trigger_setup,
    update_append_to_dictionary_in_slimming_helper,
)
from DerivationFrameworkFlavourTag.FtagDerivationConfig import (
    HLTJetFTagDecorationCfg,
)
from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
from InDetConfig.InDetPoolReadConfig import InDetPoolReadCfg
from JetRecConfig.JetRecConfig import JetRecCfg
from JetRecConfig.StandardSmallRJets import AntiKt4LCTopo
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg

if TYPE_CHECKING:
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def FTAG1KernelCfg(
    flags: AthConfigFlags,
    name: str = "FTAG1Kernel",
    **kwargs: Any,
) -> ComponentAccumulator:
    """Configure the derivation kernel for FTAG1."""
    acc = ComponentAccumulator()

    acc.merge(
        PhysCommonAugmentationsCfg(
            flags=flags,
            TriggerListsHelper=kwargs["trigger_lists_helper"],
        )
    )

    # Setup the derivation kernel
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name=name))

    # Configure extra reconstruction of jets
    acc.merge(FTAG1ExtraContentCfg(flags))
    return acc


def FTAG1CoreCfg(
    flags: AthConfigFlags,
    name_tag: str = "FTAG1",
    extra_SmartCollections: list[str] | None = None,
    extra_AllVariables: list[str] | None = None,
    trigger_lists_helper: TriggerListsHelper | None = None,
    keep_truth_collections: bool = True,
    keep_track_covariance_offdiag: bool = True,
    tau_as_smart_collection: bool = False,
) -> ComponentAccumulator:
    """Configure FTAG1 slimming and output content."""
    if extra_SmartCollections is None:
        extra_SmartCollections = []
    if extra_AllVariables is None:
        extra_AllVariables = []

    acc = ComponentAccumulator()

    ftag1_slimming_helper = SlimmingHelper(
        inputName=name_tag + "SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags,
    )

    # Initialise explicit lists
    ftag1_slimming_helper.SmartCollections = []
    ftag1_slimming_helper.AllVariables = []
    ftag1_slimming_helper.ExtraVariables = []

    # Baseline content
    add_baseline_slimming_smartcollections(slimming_helper=ftag1_slimming_helper)
    add_baseline_slimming_allvariables(slimming_helper=ftag1_slimming_helper)
    add_truth_to_slimming_helper(slimming_helper=ftag1_slimming_helper)

    # Common FTAG augmentations
    add_common_augmentation(
        flags=flags,
        acc=acc,
        slimming_helper=ftag1_slimming_helper,
    )

    # Truth vertex labeling for Maskformer (see https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/AnalysisCommon/ParticleJetTools/docs/TruthVertexLabelling.md?ref_type=heads)
    add_truth_vertex_decorations(
        flags=flags,
        acc=acc,
        slimming_helper=ftag1_slimming_helper,
        large_r_jet_collection="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
    )

    # FTAG1-specific smart collections
    ftag1_slimming_helper.SmartCollections += [
        "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
        "AntiKt4LCTopoJets",
    ]

    # FTAG1-specific all-variable content
    ftag1_slimming_helper.AllVariables += [
        "AntiKt4EMPFlowJets",
        "AntiKt4LCTopoJets",
        "CaloCalFwdTopoTowers",
        "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
        "UFOCSSK",
        "GlobalChargedParticleFlowObjects",
        "GlobalNeutralParticleFlowObjects",
        "CHSGChargedParticleFlowObjects",
        "CHSGNeutralParticleFlowObjects",
        "CSSKGChargedParticleFlowObjects",
        "CSSKGNeutralParticleFlowObjects",
        "CaloCalTopoClusters",
        "JetAssociatedPixelClusters",
        "JetAssociatedSCTClusters",
        "PixelClusters",
        "SCT_Clusters",
    ]

    if tau_as_smart_collection:
        ftag1_slimming_helper.SmartCollections += ["TauJets"]
    else:
        ftag1_slimming_helper.AllVariables += [
            "TauJets",
            "TauNeutralParticleFlowObjects",
            "TauShotParticleFlowObjects",
            "TauTracks",
        ]
    if keep_truth_collections:
        ftag1_slimming_helper.AllVariables += [
            "TruthEvents",
            "TruthParticles",
            "TruthVertices",
        ]

    # Extra variables from e/gamma and common FTAG content
    ftag1_slimming_helper.ExtraVariables += ElectronsCPDetailedContent
    add_extra_variables_to_slimming_helper(
        flags=flags,
        slimming_helper=ftag1_slimming_helper,
    )

    # FTAG1-specific extra variables
    ftag1_slimming_helper.ExtraVariables += [
        "AntiKt10TruthSoftDropBeta100Zcut10Jets.constituentLinks",
        "AntiKt4TruthDressedWZJets.constituentLinks",
        "AntiKt4TruthJets.constituentLinks",
        (
            "AntiKt4EMTopoJets."
            "HadronConeExclTruthLabelID."
            "HadronGhostTruthLabelID."
            "GhostBHadronsFinal."
            "GhostCHadronsFinal."
            "GhostTausFinal."
            "ConeExclBHadronsFinal."
            "ConeExclCHadronsFinal."
            "ConeExclTausFinal"
        ),
        (
            "AntiKt4LCTopoJets."
            "HadronConeExclTruthLabelID."
            "HadronGhostTruthLabelID."
            "GhostBHadronsFinal."
            "GhostCHadronsFinal."
            "GhostTausFinal."
            "ConeExclBHadronsFinal."
            "ConeExclCHadronsFinal."
            "ConeExclTausFinal"
        ),
    ]

    # Run-4-specific additions
    if flags.GeoModel.Run >= LHCPeriod.Run4:
        ftag1_slimming_helper.SmartCollections += [
            "AntiKt4EMTopoJets",
            "MET_Baseline_AntiKt4EMTopo",
        ]
        ftag1_slimming_helper.AllVariables += [
            "AntiKt4EMTopoJets",
            "AntiKt4TruthJets",
            "ITkPixelMeasurements",
            "ITkStripMeasurements",
            "ITkPixelSpacePoints",
            "ITkStripSpacePoints",
            "ITkStripOverlapSpacePoints",
        ]

        # Needed for ITk space points
        acc.merge(InDetPoolReadCfg(flags))

    # User-provided extras
    for container in extra_SmartCollections:
        if container not in ftag1_slimming_helper.SmartCollections:
            ftag1_slimming_helper.SmartCollections.append(container)

    for container in extra_AllVariables:
        if container not in ftag1_slimming_helper.AllVariables:
            ftag1_slimming_helper.AllVariables.append(container)

    # Optional pseudotrack content
    if flags.BTagging.Pseudotrack:
        ftag1_slimming_helper.AllVariables += ["InDetPseudoTrackParticles"]

    # Append-to-dictionary updates
    update_append_to_dictionary_in_slimming_helper(
        flags=flags,
        slimming_helper=ftag1_slimming_helper,
    )

    # Trigger content
    trigger_setup(slimming_helper=ftag1_slimming_helper)
    trigger_matching(
        flags=flags,
        slimming_helper=ftag1_slimming_helper,
        trigger_lists_helper=trigger_lists_helper,
    )

    # Run jet labelling for trigger jets
    if flags.Trigger.EDMVersion == 3 and flags.Input.isMC:
        acc.merge(HLTJetFTagDecorationCfg(flags))

    # Output stream
    ftag1_item_list = ftag1_slimming_helper.GetItemList()

    # Drop the off-diagonal track covariance matrix from InDetTrackParticles.
    if not keep_track_covariance_offdiag:
        _cov_vars = "-definingParametersCovMatrixOffDiag"

        def _drop_cov(item: str) -> str:
            if "#InDetTrackParticlesAux." not in item:
                return item
            sep = "" if item.endswith("Aux.") else "."
            return item + sep + _cov_vars

        ftag1_item_list = [_drop_cov(item) for item in ftag1_item_list]

    acc.merge(
        OutputStreamCfg(
            flags=flags,
            streamName="DAOD_" + name_tag,
            ItemList=ftag1_item_list,
            AcceptAlgs=[name_tag + "Kernel"],
        )
    )

    acc.merge(
        SetupMetaDataForStreamCfg(
            flags=flags,
            streamName="DAOD_" + name_tag,
            AcceptAlgs=[name_tag + "Kernel"],
            createMetadata=[
                MetadataCategory.CutFlowMetaData,
                MetadataCategory.TruthMetaData,
            ],
        )
    )

    return acc


def FTAG1ExtraContentCfg(flags: AthConfigFlags) -> ComponentAccumulator:
    """Configure extra reconstructed jet content for FTAG1."""
    acc = ComponentAccumulator()

    jet_list = [AntiKt4LCTopo]
    for jet_def in jet_list:
        acc.merge(JetRecCfg(flags, jet_def))

    return acc


def FTAG1Cfg(
    flags: AthConfigFlags,
    name_tag: str = "FTAG1",
    keep_truth_collections: bool = True,
) -> ComponentAccumulator:
    """Configure the full FTAG1 derivation."""
    acc = ComponentAccumulator()

    ftag1_trigger_lists_helper = TriggerListsHelper(flags)

    acc.merge(
        FTAG1KernelCfg(
            flags=flags,
            name=name_tag + "Kernel",
            StreamName="StreamDAOD_" + name_tag,
            trigger_lists_helper=ftag1_trigger_lists_helper,
        )
    )

    acc.merge(
        FTAG1CoreCfg(
            flags=flags,
            name_tag=name_tag,
            trigger_lists_helper=ftag1_trigger_lists_helper,
        )
    )

    return acc
