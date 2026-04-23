# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# DAOD_FTAG4.py
#
# PHYS-like DAOD format with a one-lepton skim for calibration studies.
# Requires the FTAG4 flag in Derivation_tf.py
# ====================================================================

from __future__ import annotations

from typing import Any, TYPE_CHECKING

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from DerivationFrameworkPhys.PHYS import PHYSCoreCfg, PHYSKernelCfg
from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
    xAODStringSkimmingToolCfg,
)

if TYPE_CHECKING:
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def _get_lepton_skimming_expression() -> str:
    """Return the FTAG4 one-lepton skimming expression."""
    muon_quality = "(0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType)"
    electron_quality = "((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))"

    return (
        f"count( (Muons.pt > 25*GeV) && {muon_quality} ) "
        f"+ count(( Electrons.pt > 25*GeV) && {electron_quality}) >= 1"
    )


def FTAG4KernelCfg(
    flags: AthConfigFlags,
    name: str = "FTAG4Kernel",
    **kwargs: Any,
) -> ComponentAccumulator:
    """Configure the derivation kernel for FTAG4."""
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
            name="FTAG4LeptonSkimmingTool",
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


def FTAG4Cfg(
    flags: AthConfigFlags,
    name_tag: str = "FTAG4",
) -> ComponentAccumulator:
    """Configure the full FTAG4 derivation."""
    acc = ComponentAccumulator()

    trigger_lists_helper = TriggerListsHelper(flags)
    stream_name = "StreamDAOD_" + name_tag

    acc.merge(
        FTAG4KernelCfg(
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

    return acc
