# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# DAOD_FTAGSSV.py
#
# PHYS-like DAOD format with NVSI tracking systematics added that are needed for the soft b-tagging calibration.
# Requires the FTAGSSV flag in Derivation_tf.py
# ====================================================================

from __future__ import annotations

from typing import Any, TYPE_CHECKING

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from DerivationFrameworkPhys.PHYS import PHYSCoreCfg, PHYSKernelCfg
from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from AsgAnalysisAlgorithms.PileupReweightingAlgConfig import PileupReweightingAlgCfg
from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import (
    InDetTrackBiasingCalibKwargs,
    TrackSmearingAlgCfg,
    TrackSystematicsAlgCfg,
)
from NewVrtSecInclusiveTool.NewVrtSecInclusiveAlgConfig import (
    NewVrtSecInclusiveAlgTightCfg
)

if TYPE_CHECKING:
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags

from AthenaCommon.Logging import logging
logFTAGSSV = logging.getLogger('FTAGSSV')

# helper to get tracking systematics
def _get_NVSI_track_systematics_list() -> list[str]:
    """Return the list of track systematics to be considered for NVSI"""
    track_systematics = [
        "_TRK_EFF_LOOSE_GLOBAL",
        "_TRK_BIAS_NOMINAL",
        "_TRK_BIAS_D0_WM",
        "_TRK_BIAS_QOVERP_SAGITTA_WM",
        "_TRK_BIAS_Z0_WM",
        "_TRK_EFF_LOOSE_IBL",
        "_TRK_EFF_LOOSE_PHYSMODEL",
        "_TRK_EFF_LOOSE_PP0",
        "_TRK_FAKE_RATE_LOOSE",
        "_TRK_RES_D0_MEAS",
        "_TRK_RES_Z0_MEAS",
    ]
    return track_systematics

def FTAGSSVKernelCfg(
    flags: AthConfigFlags,
    name: str = "FTAGSSVKernel",
    **kwargs: Any,
) -> ComponentAccumulator:
    """Configure the derivation kernel for FTAGSSV."""
    acc = ComponentAccumulator()

    # PHYS is the baseline
    acc.merge(
        PHYSKernelCfg(
            flags=flags,
            name=name,
            StreamName=kwargs["StreamName"],
            TriggerListsHelper=kwargs["TriggerListsHelper"],
        )
    )
    return acc

# Additional content for FTAGSSV
def FTAGSSVExtraContentCfg(
    flags: AthConfigFlags,
    name_tag: str = "FTAGSSV",
) -> ComponentAccumulator:
    """Configure FTAGSSV-specific output additions."""
    acc = ComponentAccumulator()

    slimming_helper = SlimmingHelper(
        inputName=name_tag + "SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags,
    )

    if flags.Input.isMC:
        track_collection = "InDetTrackParticles"
        track_systematics = _get_NVSI_track_systematics_list()

        # verify that tracking systematics tool works -> track systematics tool can not be scheduled for MC campaigns without existing tracking systematics recommendations
        track_sys_accs = []
        try:
            # the track biasing needs the RandomRunNumber from the pileup reweighting
            prw_acc = PileupReweightingAlgCfg(flags)
            for suffix in track_systematics:
                syst = suffix[1:]
                varied_track_container = f"{track_collection}{suffix}"
                # efficiency and fake rate systematics drop tracks, resolution and bias systematics smear or shift them
                if syst.startswith(("TRK_EFF_", "TRK_FAKE_RATE_")):
                    track_sys_acc = TrackSystematicsAlgCfg(
                        flags,
                        name=f"InDetTrackSystematicsAlg{suffix}",
                        InputTrackContainer=track_collection,
                        OutputTrackContainer=varied_track_container,
                        SystematicVariationSTD=syst,
                    )
                # all track biases applied, each TRK_BIAS_* variation removes one of them again
                elif suffix == "_TRK_BIAS_NOMINAL":
                    track_sys_acc = TrackSmearingAlgCfg(
                        flags,
                        syst="",
                        input_tracks=track_collection,
                        output_tracks=varied_track_container,
                        bias_kwargs=InDetTrackBiasingCalibKwargs(flags),
                    )
                else:
                    track_sys_acc = TrackSmearingAlgCfg(
                        flags,
                        syst=syst,
                        input_tracks=track_collection,
                        output_tracks=varied_track_container,
                        strict_mode=True,
                    )
                # build list of tuples of (suffix, varied track container name, track systematics alg to be run)
                track_sys_accs.append((suffix, varied_track_container, track_sys_acc))
        # the tracking systematics tool can not be configured -> skip the tracking systematics
        except ValueError as err:
            logFTAGSSV.info(
                "Skipping FTAGSSV NVSI tracking systematics: %s",
                err,
            )
        # the tracking systematics tool can be configured -> proceed as planned
        else:
            acc.merge(prw_acc)
            #retrieve the tuples
            for suffix, varied_track_container, track_sys_acc in track_sys_accs:
                # apply the tracking systematics              
                acc.merge(track_sys_acc)
                # run the NewVrtSecInclusive with the varied input tracks
                acc.merge(
                    NewVrtSecInclusiveAlgTightCfg(
                        flags,
                        algname=f"NewVrtSecInclusive{suffix}",
                        AugmentingVersionString=suffix,
                        BVertexContainerName=f"NVSI_SecVrt_Tight{suffix}",
                        TrackParticleContainer=varied_track_container,
                    )
                )

            nvsi_containers = []
            for suffix in track_systematics:
                nvsi_containers.append(f"NVSI_SecVrt_Tight{suffix}")

            excl_vertex_aux_data = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV.-trackParticleLinks.-trackWeights.-neutralParticleLinks.-neutralWeights"
            # add NVSI_SecVrt_Tight tracking systematics to the slimming_helper
            for container in nvsi_containers:
                slimming_helper.AppendToDictionary.update({
                    container: "xAOD::VertexContainer",
                    container + "Aux": "xAOD::VertexAuxContainer",
                })

                slimming_helper.StaticContent += [
                    f"xAOD::VertexContainer#{container}",
                    (
                        f"xAOD::VertexAuxContainer#{container}Aux."
                        f"{excl_vertex_aux_data}"
                    ),
                ]

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
    return acc

def FTAGSSVCfg(
    flags: AthConfigFlags,
    name_tag: str = "FTAGSSV",
) -> ComponentAccumulator:
    """Configure the full FTAGSSV derivation."""
    acc = ComponentAccumulator()

    trigger_lists_helper = TriggerListsHelper(flags)
    stream_name = "StreamDAOD_" + name_tag

    acc.merge(
        FTAGSSVKernelCfg(
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

    # FTAGSSV extra content -> the NVSI track systematics containers
    acc.merge(
        FTAGSSVExtraContentCfg(
            flags=flags,
            name_tag=name_tag,
        )
    )
    return acc
