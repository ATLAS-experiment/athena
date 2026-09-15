# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# DAOD_FTAGTRIGCAL.py
#
# PHYS-like DAOD format with a two-lepton skim and the b-jet trigger content
# needed for the b-jet trigger calibration.
# Requires the FTAGTRIGCAL flag in Derivation_tf.py
# ====================================================================

from __future__ import annotations

from typing import Any, TYPE_CHECKING

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
from DerivationFrameworkFlavourTag.FtagBaseContent import (
    add_truth_to_slimming_helper,
    trigger_matching,
)
from DerivationFrameworkFlavourTag.FtagDerivationConfig import (
    HLTJetFTagDecorationCfg,
)
from DerivationFrameworkPhys.PHYS import PHYSCoreCfg, PHYSKernelCfg
from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
    xAODStringSkimmingToolCfg,
)
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg

if TYPE_CHECKING:
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def _get_lepton_skimming_expression() -> str:
    """Return the FTAGTRIGCAL dilepton skimming expression."""
    muon_quality = "(0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType)"
    electron_quality = "((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))"

    return (
        f"count( (Muons.pt > 18*GeV) && {muon_quality} ) "
        f"+ count(( Electrons.pt > 18*GeV) && {electron_quality}) >= 2 "
        "&& "
        f"count( (Muons.pt > 25*GeV) && {muon_quality} ) "
        f"+ count(( Electrons.pt > 25*GeV) && {electron_quality}) >= 1"
    )


def FTAGTRIGCALKernelCfg(
    flags: AthConfigFlags,
    name: str = "FTAGTRIGCALKernel",
    **kwargs: Any,
) -> ComponentAccumulator:
    """Configure the derivation kernel for FTAGTRIGCAL."""
    acc = ComponentAccumulator()

    acc.merge(
        PHYSKernelCfg(
            flags=flags,
            name=name,
            StreamName=kwargs["StreamName"],
            TriggerListsHelper=kwargs["TriggerListsHelper"],
        )
    )

    lepton_skimming_tool = acc.getPrimaryAndMerge(
        xAODStringSkimmingToolCfg(
            flags=flags,
            name="FTAGTRIGCALLeptonSkimmingTool",
            expression=_get_lepton_skimming_expression(),
        )
    )

    acc.addEventAlgo(
        CompFactory.DerivationFramework.DerivationKernel(
            name=name,
            AugmentationTools=[],
            SkimmingTools=[lepton_skimming_tool],
            ThinningTools=[],
        )
    )

    return acc


def FTAGTRIGCALExtraContentCfg(
    flags: AthConfigFlags,
    name_tag: str = "FTAGTRIGCAL",
    trigger_lists_helper: TriggerListsHelper | None = None,
) -> ComponentAccumulator:
    """Configure FTAGTRIGCAL-specific output additions."""
    acc = ComponentAccumulator()

    if trigger_lists_helper is None:
        trigger_lists_helper = TriggerListsHelper(flags)

    slimming_helper = SlimmingHelper(
        inputName=name_tag + "SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags,
    )

    # Add truth containers
    if flags.Input.isMC:
        add_truth_to_slimming_helper(slimming_helper=slimming_helper)
        if flags.Trigger.EDMVersion == 3:
            # Add truth labels to Run 3 trigger jets and trigger b-jets.
            acc.merge(HLTJetFTagDecorationCfg(flags))
            acc.merge(
                HLTJetFTagDecorationCfg(
                    flags=flags,
                    name="hltBJetLabelingAlg",
                    jet_container="HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJets",
                )
            )

    # Trigger content, as stored in FTAG2 before the FTAG derivation rework
    slimming_helper.IncludeTriggerNavigation = True
    slimming_helper.IncludeMuonTriggerContent = True
    slimming_helper.IncludeEGammaTriggerContent = True
    slimming_helper.IncludeBJetTriggerContent = True
    slimming_helper.IncludeBPhysTriggerContent = True

    # Preselection taggers, the TLA and precision taggers come with the b-jet content.
    # The fastftag jets are a shallow copy and take their kinematics from the parent.
    fastftag_vars = [
        f"{tagger}_{prob}"
        for tagger in ["fastDips", "fastGN220240122"]
        for prob in ["pb", "pc", "pu"]
    ]
    slimming_helper.ExtraVariables += [
        "HLT_AntiKt4EMTopoJets_subjesIS.pt.eta.phi.m",
        ".".join(["HLT_AntiKt4EMTopoJets_subjesIS_fastftag"] + fastftag_vars),
    ]

    trigger_matching(
        flags=flags,
        slimming_helper=slimming_helper,
        trigger_lists_helper=trigger_lists_helper,
    )

    # Output stream additions
    # The jet aux exclusions clash with the positive HLT b-jet selection from PHYS
    bjet_aux = "#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJetsAux."
    item_list = [
        item[: item.find(bjet_aux) + len(bjet_aux)] if bjet_aux in item else item
        for item in slimming_helper.GetItemList()
    ]
    acc.merge(
        OutputStreamCfg(
            flags=flags,
            streamName="DAOD_" + name_tag,
            ItemList=item_list,
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


def FTAGTRIGCALCfg(
    flags: AthConfigFlags,
    name_tag: str = "FTAGTRIGCAL",
) -> ComponentAccumulator:
    """Configure the full FTAGTRIGCAL derivation."""
    acc = ComponentAccumulator()

    trigger_lists_helper = TriggerListsHelper(flags)
    stream_name = "StreamDAOD_" + name_tag

    acc.merge(
        FTAGTRIGCALKernelCfg(
            flags=flags,
            name=name_tag + "Kernel",
            StreamName=stream_name,
            TriggerListsHelper=trigger_lists_helper,
        )
    )

    # PHYS content
    acc.merge(
        PHYSCoreCfg(
            flags=flags,
            name_tag=name_tag,
            StreamName=stream_name,
            TriggerListsHelper=trigger_lists_helper,
        )
    )

    acc.merge(
        FTAGTRIGCALExtraContentCfg(
            flags=flags,
            name_tag=name_tag,
            trigger_lists_helper=trigger_lists_helper,
        )
    )

    return acc
