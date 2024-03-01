# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""Code to setup MET reconstruction during the end-of-event sequence"""

from typing import List

from AthenaCommon.CFElements import parOR
from AthenaCommon.Configurable import ConfigurableCABehavior
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from .ConfigHelpers import AlgConfig, stringToMETRecoDict
from ..CommonSequences.FullScanDefs import caloFSRoI, trkFSRoI


_algs_by_purpose = {
    "metcalo": ["cell", "tcpufit", "tcpufit_sig30"],
    "mettrk": [
        "pfopufit",
        "pfsum",
        "pfsum_vssk",
        "pfsum_cssk",
        "trkmht",
        "mhtpufit_pf",
        "mhtpufit_em",
        "pfopufit_sig30",
        "nn",
    ],
}


def getMETRecoSequences(flags, purposes: List[str]):
    """Create the full reconstruction sequences to run at the end of the event

    Parameters
    ----------
    purposes : List[str]
        The purpose keys given in the associated acceptedevts chain

    Returns
    -------
    An OR containing the configured algorithms
    A list of the required RoIs
    A list of the required streams
    """
    ef_reco_algs = []
    for purpose in purposes:
        ef_reco_algs += [
            alg for alg in _algs_by_purpose.get(purpose, []) if alg not in ef_reco_algs
        ]
    seqname = f"{''.join(purposes)}EndOfEventRecoSequence"
    with ConfigurableCABehavior():
        merged_ca = ComponentAccumulator()
        merged_ca.addSequence(parOR(seqname), primary=True)
        max_step_idx = 0
        for alg in ef_reco_algs:
            cfg = AlgConfig.fromRecoDict(**stringToMETRecoDict(alg))
            step_output = cfg.make_reco_algs(flags, **cfg.interpret_reco_dict())
            max_step_idx = max(max_step_idx, step_output.max_step_idx)
            for ca in step_output.steps:
                merged_ca.merge(ca, seqname)

    rois = [caloFSRoI]
    if max_step_idx > 1:
        # If we have more than one step then we also need the FS tracking RoI
        rois.append(trkFSRoI)

    streams = ["Main", "VBFDelayed"]
    return merged_ca, rois, streams
