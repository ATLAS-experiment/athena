# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# DAOD_HID1.py
#
# Unskimmed DAOD format for hadronic-identification studies on MC.
#
# HID1 is the union of the FTAG1 flavour-tagging content and the jet
# content of JETM2, so that a single format serves flavour tagging, jet
# and hadronic-tau studies. It is intended to supersede both of those
# formats.
# Requires the HID1 flag in Derivation_tf.py
# ====================================================================

from __future__ import annotations

from typing import TYPE_CHECKING, Any

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, MetadataCategory
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
from DerivationFrameworkEGamma.ElectronsCPDetailedContent import ElectronsCPDetailedContent
from DerivationFrameworkFlavourTag.FtagBaseContent import (
    add_baseline_slimming_allvariables,
    add_baseline_slimming_smartcollections,
    add_common_augmentation,
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
from DerivationFrameworkFlavourTag.FtagVRJetConfig import (
    VR_JET_APPEND_TO_DICTIONARY,
    VR_JET_TRUTH_VERTEX_DR,
    VR_JETS,
    VRFtagJetsCfg,
    add_vr_jet_truth_augmentation,
)
from DerivationFrameworkJetEtMiss.CommonJETMXContent import (
    ClusterVariables,
    ExtraJSSVariables,
    FELinks,
    FlowElementVariables,
    TrackingVariables,
    TrackingVariablesHGTD,
    UFOVariables,
)
from DerivationFrameworkJetEtMiss.JetCommonConfig import addOriginCorrectedClustersToSlimmingTool
from DerivationFrameworkMCTruth.MCTruthCommonConfig import (
    AddTopQuarkAndDownstreamParticlesCfg,
    AddTruthCollectionNavigationDecorationsCfg,
)
from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
from InDetConfig.InDetPoolReadConfig import InDetPoolReadCfg
from JetRecConfig.JetInputConfig import buildEventShapeAlg
from JetRecConfig.JetRecConfig import JetRecCfg, getConstitPJGAlg, getInputAlgs
from JetRecConfig.StandardJetConstits import stdConstitDic as cst
from JetRecConfig.StandardLargeRJets import AntiKt10TruthDressedWZSoftDrop
from JetRecConfig.StandardSmallRJets import AntiKt4LCTopo
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg

if TYPE_CHECKING:
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags


LARGE_R_JETS = "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"

GN3_LATENT_VARS = [
    "GN3EPCLV01_Latent",
    "GN3EPCLV01_Latent128",
    "GN3EPCLV01_Latent64",
    "GN3EPCLV01_Latent32",
    "GN3EPCLV01SimpleFlip_Latent",
    "GN3EPCLV01SimpleFlip_Latent128",
    "GN3EPCLV01SimpleFlip_Latent64",
    "GN3EPCLV01SimpleFlip_Latent32",
]


def _add_ftag_content(
    flags: AthConfigFlags,
    acc: ComponentAccumulator,
    slimming_helper: SlimmingHelper,
    keep_truth_collections: bool = True,
    tau_as_smart_collection: bool = False,
) -> None:
    """Add the flavour-tagging content and augmentations to the slimming helper."""

    # Baseline content
    add_baseline_slimming_smartcollections(slimming_helper=slimming_helper)
    add_baseline_slimming_allvariables(slimming_helper=slimming_helper)
    add_truth_to_slimming_helper(slimming_helper=slimming_helper)

    # Common FTAG augmentations
    add_common_augmentation(
        flags=flags,
        acc=acc,
        slimming_helper=slimming_helper,
    )

    # Truth vertex labeling for Maskformer (see https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/AnalysisCommon/ParticleJetTools/docs/TruthVertexLabelling.md?ref_type=heads)
    # The VR jets are summarised at their Rmax, which over-associates the high-pT jets whose
    # real radius has shrunk to Rmin; a shrinking-cone-aware summary is left to a follow-up.
    add_truth_vertex_decorations(
        flags=flags,
        acc=acc,
        slimming_helper=slimming_helper,
        large_r_jet_collection=LARGE_R_JETS,
        extra_jet_collections=[(VR_JETS, VR_JET_TRUTH_VERTEX_DR)],
    )

    add_vr_jet_truth_augmentation(
        flags=flags,
        acc=acc,
        slimming_helper=slimming_helper,
    )

    slimming_helper.SmartCollections += [
        LARGE_R_JETS,
        "AntiKt4LCTopoJets",
    ]

    slimming_helper.AllVariables += [
        "AntiKt4EMPFlowJets",
        VR_JETS,
        "AntiKt4LCTopoJets",
        "CaloCalFwdTopoTowers",
        LARGE_R_JETS,
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
        slimming_helper.SmartCollections += ["TauJets"]
    else:
        slimming_helper.AllVariables += [
            "TauJets",
            "TauNeutralParticleFlowObjects",
            "TauShotParticleFlowObjects",
            "TauTracks",
        ]
    if keep_truth_collections:
        slimming_helper.AllVariables += [
            "TruthEvents",
            "TruthParticles",
            "TruthVertices",
        ]

    # Extra variables from e/gamma and common FTAG content
    slimming_helper.ExtraVariables += ElectronsCPDetailedContent
    add_extra_variables_to_slimming_helper(
        flags=flags,
        slimming_helper=slimming_helper,
    )

    slimming_helper.ExtraVariables += [
        "AntiKt10TruthSoftDropBeta100Zcut10Jets.constituentLinks",
        "AntiKt4TruthDressedWZJets.constituentLinks",
        "AntiKt4TruthJets.constituentLinks",
    ]

    # Truth labelling for the topo-cluster jets, which are not smart-slimmed here
    for jet_collection in ["AntiKt4EMTopoJets", "AntiKt4LCTopoJets"]:
        slimming_helper.ExtraVariables.append(
            ".".join(
                [
                    jet_collection,
                    "HadronConeExclTruthLabelID",
                    "HadronGhostTruthLabelID",
                    "GhostBHadronsFinal",
                    "GhostCHadronsFinal",
                    "GhostTausFinal",
                    "ConeExclBHadronsFinal",
                    "ConeExclCHadronsFinal",
                    "ConeExclTausFinal",
                ]
            )
        )

    # Extra ITk, HGTD and EMTopo jet content available with Run-4 inputs
    if flags.GeoModel.Run >= LHCPeriod.Run4:
        slimming_helper.SmartCollections += ["MET_Baseline_AntiKt4EMTopo"]
        slimming_helper.AllVariables += [
            "AntiKt4EMTopoJets",
            "AntiKt4TruthJets",
            "ITkPixelMeasurements",
            "ITkStripMeasurements",
            "ITkPixelSpacePoints",
            "ITkStripSpacePoints",
            "ITkStripOverlapSpacePoints",
        ]
        slimming_helper.ExtraVariables += [
            ".".join(["InDetTrackParticles"] + TrackingVariablesHGTD)
        ]

        # Needed for ITk space points
        acc.merge(InDetPoolReadCfg(flags))

    # Optional pseudotrack content
    if flags.BTagging.Pseudotrack:
        slimming_helper.AllVariables += ["InDetPseudoTrackParticles"]

    # Run jet labelling for trigger jets
    if flags.Trigger.EDMVersion == 3 and flags.Input.isMC:
        acc.merge(HLTJetFTagDecorationCfg(flags))


def _add_jet_content(
    flags: AthConfigFlags,
    slimming_helper: SlimmingHelper,
) -> None:
    """Add the jet reconstruction and constituent content to the slimming helper."""

    slimming_helper.SmartCollections += [
        "EventInfo",
        "Photons",
        "AntiKt4EMTopoJets",
    ]

    slimming_helper.AllVariables += [
        "Kt4EMTopoOriginEventShape",
        "Kt4EMPFlowEventShape",
        "Kt4EMPFlowNeutEventShape",
        "Kt4UFOCSSKEventShape",
        "Kt4UFOCSSKNeutEventShape",
    ]

    # Low-level inputs
    slimming_helper.ExtraVariables += [
        ".".join(["CaloCalTopoClusters"] + ClusterVariables),
        ".".join(["EMOriginTopoClusters"] + ["calM"]),
        ".".join(
            ["GlobalChargedParticleFlowObjects"] + FlowElementVariables + ["otherObjectWeights"]
        ),
        ".".join(
            ["GlobalNeutralParticleFlowObjects"] + FlowElementVariables + ["otherObjectWeights"]
        ),
        ".".join(["UFO"] + UFOVariables),
        ".".join(["UFOCSSK"] + UFOVariables),
        ".".join(["InDetTrackParticles"] + TrackingVariables),
    ]

    # Links of physics objects to FlowElements
    slimming_helper.ExtraVariables += FELinks

    # Detailed substructure information
    slimming_helper.ExtraVariables += [".".join([LARGE_R_JETS] + ExtraJSSVariables)]

    slimming_helper.ExtraVariables += [
        ".".join(
            [
                "AntiKt4EMPFlowJets",
                "GhostTower",
                "IsoFixedCone5Pt",
                "IsoFixedCone5PtPUsub",
                "constituentLinks",
            ]
        ),
        "AntiKt4EMTopoJets.IsoFixedCone5Pt.IsoFixedCone5PtPUsub.constituentLinks",
        f"{LARGE_R_JETS}.SizeParameter.GhostTrack.constituentLinks",
        "GSFTrackParticles.particleHypothesis.vx.vy.vz",
        "PrimaryVertices.x.y.z.covariance.trackWeights",
        "TauJets.clusterLinks",
        ".".join(
            [
                "Muons",
                "energyLossType",
                "EnergyLoss",
                "ParamEnergyLoss",
                "MeasEnergyLoss",
                "EnergyLossSigma",
                "MeasEnergyLossSigma",
                "ParamEnergyLossSigmaPlus",
                "ParamEnergyLossSigmaMinus",
                "clusterLinks",
                "FSR_CandidateEnergy",
            ]
        ),
        "MuonSegments.x.y.z.px.py.pz",
    ]

    addOriginCorrectedClustersToSlimmingTool(slimming_helper, writeLC=True, writeEM=True)

    if flags.Input.isMC:
        slimming_helper.AllVariables += [
            "TruthTopQuarkWithDecayParticles",
            "TruthTopQuarkWithDecayVertices",
        ]

        slimming_helper.SmartCollections += [
            "AntiKt4TruthWZJets",
            "AntiKt10TruthJets",
            "AntiKt10TruthDressedWZSoftDropBeta100Zcut10Jets",
        ]

        slimming_helper.ExtraVariables += [
            "AntiKt10TruthSoftDropBeta100Zcut10Jets.SizeParameter.constituentLinks",
            "AntiKt10TruthDressedWZSoftDropBeta100Zcut10Jets.constituentLinks",
            "AntiKt10TruthDressedWZJets.constituentLinks",
            "AntiKt10TruthJets.constituentLinks",
            "AntiKt4TruthWZJets.IsoFixedCone5Pt.constituentLinks",
            "AntiKt4TruthDressedWZJets.IsoFixedCone5Pt.constituentLinks",
            ".".join(
                [
                    LARGE_R_JETS,
                    "GhostTQuarksFinalCount",
                    "GhostHBosonsCount",
                    "GhostZBosonsCount",
                    "GhostWBosonsCount",
                ]
            ),
            ".".join(
                [
                    LARGE_R_JETS,
                    "GhostTQuarksFinalPt",
                    "GhostHBosonsPt",
                    "GhostZBosonsPt",
                    "GhostWBosonsPt",
                ]
            ),
            f"{LARGE_R_JETS}.GhostBHadronsFinalPt.GhostCHadronsFinalPt",
        ]


def _drop_track_covariance_offdiag(item_list: list[str]) -> list[str]:
    """Drop the off-diagonal track covariance matrix from InDetTrackParticles."""
    cov_vars = "-definingParametersCovMatrixOffDiag"

    def drop_cov(item: str) -> str:
        if "#InDetTrackParticlesAux." not in item:
            return item
        sep = "" if item.endswith("Aux.") else "."
        return item + sep + cov_vars

    return [drop_cov(item) for item in item_list]


def HID1KernelCfg(
    flags: AthConfigFlags,
    name: str = "HID1Kernel",
    **kwargs: Any,
) -> ComponentAccumulator:
    """Configure the derivation kernel for HID1."""
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
    acc.merge(HID1ExtraContentCfg(flags))
    return acc


def HID1CoreCfg(
    flags: AthConfigFlags,
    name_tag: str = "HID1",
    extra_SmartCollections: list[str] | None = None,
    extra_AllVariables: list[str] | None = None,
    trigger_lists_helper: TriggerListsHelper | None = None,
    keep_truth_collections: bool = True,
    keep_track_covariance_offdiag: bool = True,
    tau_as_smart_collection: bool = False,
    compress_gn3_latent: bool = True,
) -> ComponentAccumulator:
    """Configure HID1 slimming and output content."""
    if extra_SmartCollections is None:
        extra_SmartCollections = []
    if extra_AllVariables is None:
        extra_AllVariables = []

    acc = ComponentAccumulator()

    slimming_helper = SlimmingHelper(
        inputName=name_tag + "SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags,
    )

    # Initialise explicit lists
    slimming_helper.SmartCollections = []
    slimming_helper.AllVariables = []
    slimming_helper.ExtraVariables = []

    _add_ftag_content(
        flags=flags,
        acc=acc,
        slimming_helper=slimming_helper,
        keep_truth_collections=keep_truth_collections,
        tau_as_smart_collection=tau_as_smart_collection,
    )

    _add_jet_content(
        flags=flags,
        slimming_helper=slimming_helper,
    )

    # User-provided extras
    for container in extra_SmartCollections:
        if container not in slimming_helper.SmartCollections:
            slimming_helper.SmartCollections.append(container)

    for container in extra_AllVariables:
        if container not in slimming_helper.AllVariables:
            slimming_helper.AllVariables.append(container)

    # Append-to-dictionary updates
    update_append_to_dictionary_in_slimming_helper(
        flags=flags,
        slimming_helper=slimming_helper,
        extra_append_to_dictionary=VR_JET_APPEND_TO_DICTIONARY,
    )

    # Trigger content
    trigger_setup(slimming_helper=slimming_helper)
    trigger_matching(
        flags=flags,
        slimming_helper=slimming_helper,
        trigger_lists_helper=trigger_lists_helper,
    )

    # Output stream
    item_list = slimming_helper.GetItemList()

    if not keep_track_covariance_offdiag:
        item_list = _drop_track_covariance_offdiag(item_list)

    gn3_latent_compression = []
    if compress_gn3_latent:
        gn3_latent_compression = [
            "xAOD::AuxContainerBase!#AntiKt4EMPFlowJetsAux." + ".".join(GN3_LATENT_VARS)
        ]

    acc.merge(
        OutputStreamCfg(
            flags=flags,
            streamName="DAOD_" + name_tag,
            ItemList=item_list,
            AcceptAlgs=[name_tag + "Kernel"],
            CompressionListHigh=gn3_latent_compression,
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


def HID1ExtraContentCfg(flags: AthConfigFlags) -> ComponentAccumulator:
    """Configure extra reconstructed jet content for HID1."""
    acc = ComponentAccumulator()

    # CHS R = 0.4 UFO jet inputs
    for alg in getInputAlgs(cst.UFO, flags=flags):
        if isinstance(alg, ComponentAccumulator):
            acc.merge(alg)
        else:
            acc.addEventAlgo(alg)

    jet_list = [AntiKt4LCTopo]
    if flags.Input.isMC:
        jet_list += [AntiKt10TruthDressedWZSoftDrop]

    for jet_def in jet_list:
        acc.merge(JetRecCfg(flags, jet_def))

    # Variable-R EMPFlow jets (low-pT-wide, shrinking to 0.4) + soft-lepton association
    acc.merge(VRFtagJetsCfg(flags))

    # UFO CSSK event shapes, for both all and neutral-only constituents
    acc.addEventAlgo(buildEventShapeAlg(cst.UFOCSSK, "", suffix=None))
    acc.addEventAlgo(getConstitPJGAlg(cst.UFOCSSK, suffix="Neut"))
    acc.addEventAlgo(buildEventShapeAlg(cst.UFOCSSK, "", suffix="Neut"))

    # More detailed truth information
    if flags.Input.isMC:
        acc.merge(AddTopQuarkAndDownstreamParticlesCfg(flags))
        acc.merge(
            AddTruthCollectionNavigationDecorationsCfg(
                flags,
                TruthCollections=[
                    "TruthTopQuarkWithDecayParticles",
                    "TruthBosonsWithDecayParticles",
                ],
                prefix="Top",
            )
        )

    return acc


def HID1Cfg(
    flags: AthConfigFlags,
    name_tag: str = "HID1",
) -> ComponentAccumulator:
    """Configure the full HID1 derivation."""
    acc = ComponentAccumulator()

    trigger_lists_helper = TriggerListsHelper(flags)

    acc.merge(
        HID1KernelCfg(
            flags=flags,
            name=name_tag + "Kernel",
            StreamName="StreamDAOD_" + name_tag,
            trigger_lists_helper=trigger_lists_helper,
        )
    )

    acc.merge(
        HID1CoreCfg(
            flags=flags,
            name_tag=name_tag,
            trigger_lists_helper=trigger_lists_helper,
        )
    )

    return acc
