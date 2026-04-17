"""
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.

Define sets of standard variables to save in output files.

The variable lists returned by these functions are used by the smart slimming
service to determine which variables to save in derivations.
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from AthenaConfiguration.Enums import LHCPeriod

if TYPE_CHECKING:
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def _is_run4(flags: AthConfigFlags) -> bool:
    """Check whether the current geometry corresponds to Run 4 or later."""
    return flags is not None and flags.GeoModel.Run >= LHCPeriod.Run4


def _get_variable_list(collection: str, aux_list: list[str]) -> list[str]:
    """Build the output-variable list for a collection and its aux store."""
    return [collection] + [".".join([collection + "Aux"] + aux_list)]


def _get_small_r_discriminant_vars(
    name: str,
    extra_flavours: list[str] | None = None,
    flip_modes: list[str] | None = None,
) -> list[str]:
    """Build small-R discriminant output variable names."""
    if extra_flavours is None:
        extra_flavours = []
    if flip_modes is None:
        flip_modes = [""]

    flavours = list("cub") + extra_flavours
    variants = [""] + flip_modes
    return [f"{name}{variant}_p{flavour}" for variant in variants for flavour in flavours]


def _get_large_r_discriminant_vars(
    name: str,
    extra_flavours: list[str] | None = None,
) -> list[str]:
    """Build large-R discriminant output variable names."""
    if extra_flavours is None:
        extra_flavours = []

    flavours = ["hbb", "hcc", "top", "qcd"] + extra_flavours
    return [f"{name}_p{flavour}" for flavour in flavours]


def _get_standard_small_r_vars() -> list[str]:
    """Return the standard truth and reco variables for small-R jets."""
    label_values = ["ID", "Pt", "Lxy", "DR", "PdgId", "Barcode"]
    algorithms = ["HadronConeExcl", "HadronGhost"]

    truth_vars = [
        f"{algorithm}TruthLabel{value}" for value in label_values for algorithm in algorithms
    ]
    truth_vars += [f"{algorithm}ExtendedTruthLabelID" for algorithm in algorithms]

    reco_vars = [
        "jetFoldHash",
        "jetFoldHash_noHits",
        "pt",
        "eta",
        "GhostTrack",
        "jetRank",
        "ConeExclBHadronsFinal",
        "ConeExclCHadronsFinal",
        "PartonTruthLabelID",
    ]

    return truth_vars + reco_vars


def BTaggingLargeRContent(flags: AthConfigFlags, jetcol: str) -> list[str]:
    """Return standard large-R jet and Xbb-tagging content."""
    large_r_jet_truth_aux = [
        "R10TruthLabel_R22v1",
        "R10TruthLabel_R22v1_TruthJetMass",
        "R10TruthLabel_R22v1_TruthJetPt",
        "HadronGhostExtendedTruthLabelID",
        "HadronGhostTruthLabelPt",
    ]
    jet_content = _get_variable_list(collection=jetcol, aux_list=large_r_jet_truth_aux)

    xbb_aux_vars: list[str] = []
    xbb_aux_vars += _get_large_r_discriminant_vars(name="GN2Xv01", extra_flavours=[])
    xbb_aux_vars += _get_large_r_discriminant_vars(name="GN2Xv02", extra_flavours=[])
    xbb_aux_vars += _get_large_r_discriminant_vars(
        name="GN2XTauV00",
        extra_flavours=["htautauhad"],
    )
    xbb_aux_vars += _get_large_r_discriminant_vars(
        name="GN3XPV01",
        extra_flavours=["htautauhad", "qcdbb", "qcdbx", "qcdcx", "qcdll", "Wqq"],
    )

    btag_content = _get_variable_list(collection=jetcol, aux_list=xbb_aux_vars)
    return jet_content + btag_content


def BTaggingStandardContent(flags: AthConfigFlags, jetcol: str) -> list[str]:
    """Return standard small-R jet and b-tagging content."""
    jet_basic_content = _get_variable_list(collection=jetcol, aux_list=_get_standard_small_r_vars())

    btagging_aux: list[str] = []
    btagging_aux += _get_small_r_discriminant_vars(
        name=flags.BTagging.AK4TaggerName,
        extra_flavours=["tau"],
        flip_modes=["SimpleFlip"],
    )
    btagging_aux += ["SV1_NGTinSvx", "SV1_masssvx"]

    if not _is_run4(flags):
        btagging_aux += _get_small_r_discriminant_vars(
            name="GN3V00",
            extra_flavours=["tau"],
            flip_modes=["SimpleFlip"],
        )
        btagging_aux += _get_small_r_discriminant_vars(
            name="GN3PflowMuonsV00",
            extra_flavours=["tau", "ud", "g", "s", "quark"],
            flip_modes=["SimpleFlip"],
        )
        btagging_aux += _get_small_r_discriminant_vars(
            name="GN3EPCLV01",
            extra_flavours=[
                "tau",
                "ud",
                "g",
                "s",
                "bquark",
                "antibquark",
                "cquark",
                "anticquark",
                "other",
            ],
            flip_modes=["SimpleFlip"],
        )
        btagging_aux += ["GN3PflowMuonsV00_ptFromTruthDressedWZJet"]
        btagging_aux += ["GN3EPCLV01_ptFromTruthDressedWZJet"]

    btag_content = _get_variable_list(collection=jetcol, aux_list=btagging_aux)
    return btag_content + jet_basic_content


def BTaggingExpertContent(flags: AthConfigFlags, jetcol: str) -> list[str]:
    """Return expert-level small-R jet and b-tagging content."""
    standard_content = BTaggingStandardContent(flags=flags, jetcol=jetcol)

    jet_extended_aux = [
        "GhostBHadronsFinalCount",
        "GhostBHadronsFinalPt",
        "GhostCHadronsFinalCount",
        "GhostCHadronsFinalPt",
        "GhostTausFinalCount",
        "GhostTausFinalPt",
        "PartonTruthLabelEnergy",
    ]
    extended_content = _get_variable_list(collection=jetcol, aux_list=jet_extended_aux)

    # See https://ftag.docs.cern.ch/reco_algs/taggers/overview/ for additional
    # expert-level variables that are intentionally omitted here.
    return standard_content + extended_content
