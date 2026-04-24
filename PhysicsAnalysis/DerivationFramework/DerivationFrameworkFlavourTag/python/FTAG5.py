# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# DAOD_FTAG5.py
#
# PHYS-like DAOD format with a two-lepton skim for calibration studies.
# Requires the FTAG5 flag in Derivation_tf.py
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
    trigger_setup,
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
    """Return the FTAG5 dilepton skimming expression."""
    muon_quality = "(0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType)"
    electron_quality = "((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))"

    return (
        f"count( (Muons.pt > 18*GeV) && {muon_quality} ) "
        f"+ count(( Electrons.pt > 18*GeV) && {electron_quality}) >= 2 "
        "&& "
        f"count( (Muons.pt > 25*GeV) && {muon_quality} ) "
        f"+ count(( Electrons.pt > 25*GeV) && {electron_quality}) >= 1"
    )


def FTAG5KernelCfg(
    flags: AthConfigFlags,
    name: str = "FTAG5Kernel",
    **kwargs: Any,
) -> ComponentAccumulator:
    """Configure the derivation kernel for FTAG5."""
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
            name="FTAG5LeptonSkimmingTool",
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


def FTAG5ExtraContentCfg(
    flags: AthConfigFlags,
    name_tag: str = "FTAG5",
    trigger_lists_helper: TriggerListsHelper | None = None,
) -> ComponentAccumulator:
    """Configure FTAG5-specific output additions."""
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
            # Add truth labels to Run 3 trigger jets.
            acc.merge(HLTJetFTagDecorationCfg(flags))

    # Trigger content
    trigger_setup(slimming_helper=slimming_helper)
    slimming_helper.IncludeTriggerNavigation = True
    trigger_matching(
        flags=flags,
        slimming_helper=slimming_helper,
        trigger_lists_helper=trigger_lists_helper,
    )

    # Output stream additions
    item_list = slimming_helper.GetItemList()
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


def FTAG5Cfg(
    flags: AthConfigFlags,
    name_tag: str = "FTAG5",
) -> ComponentAccumulator:
    """Configure the full FTAG5 derivation."""
    acc = ComponentAccumulator()

    trigger_lists_helper = TriggerListsHelper(flags)
    stream_name = "StreamDAOD_" + name_tag

    acc.merge(
        FTAG5KernelCfg(
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
        FTAG5ExtraContentCfg(
            flags=flags,
            name_tag=name_tag,
            trigger_lists_helper=trigger_lists_helper,
        )
    )

    return acc
