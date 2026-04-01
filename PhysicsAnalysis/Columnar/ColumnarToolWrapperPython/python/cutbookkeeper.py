# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# Read CutBookkeeper metadata from PHYSLITE (or other xAOD) ROOT files using
# uproot.  CutBookkeepers live in the MetaData TTree, not the CollectionTree,
# and carry the per-file event counts and sum-of-weights needed for MC
# normalisation.
#
# The selection logic mirrors CP::AsgCutBookkeeperAlg::fileExecute() and
# CutBookkeeperUtils::getOriginalAodCounts(): find the "AllExecutedEvents"
# entry (configurable) with the maximum cycle number from an allowed set of
# input streams.

from __future__ import annotations

from pathlib import Path
from typing import Union

import awkward as ak
import uproot

# Streams supported by the standard CP algorithm (AsgCutBookkeeperAlg).
_DEFAULT_STREAMS = ("StreamAOD", "StreamDAOD_PHYSLITE", "StreamEVGEN", "StreamEVNT")


def _read_one_file(
    path: Union[str, Path],
    allowed_streams: tuple[str, ...],
    bookkeeper_name: str,
) -> dict:
    """Return a dict of CutBookkeeper payload values for a single file."""
    with uproot.open(path) as f:
        md = f["MetaData"]

        # Each branch is a jagged array with one outer entry (the single TTree row)
        # and an inner vector of values — one per CutBookkeeper entry.
        names = md["CutBookkeepersAux./CutBookkeepersAux.name"].array()[0]
        cycles = md["CutBookkeepersAux./CutBookkeepersAux.cycle"].array()[0]
        streams = md["CutBookkeepersAux./CutBookkeepersAux.inputStream"].array()[0]
        n_acc = md["CutBookkeepersAux./CutBookkeepersAux.nAcceptedEvents"].array()[0]
        sow = md["CutBookkeepersAux./CutBookkeepersAux.sumOfEventWeights"].array()[0]
        sowsq = md["CutBookkeepersAux./CutBookkeepersAux.sumOfEventWeightsSquared"].array()[0]

        # Count incomplete bookkeeper entries (data-integrity indicator).
        n_incomplete = len(
            md["IncompleteCutBookkeepersAux./IncompleteCutBookkeepersAux.nAcceptedEvents"].array()[0]
        )

    # Find the matching entry with the maximum cycle, mirroring
    # AsgCutBookkeeperAlg::fileExecute() lines 102-114.
    best_idx = None
    best_cycle = -1
    for i, (name, cycle, stream) in enumerate(zip(names, cycles, streams)):
        if name == bookkeeper_name and stream in allowed_streams and cycle > best_cycle:
            best_idx = i
            best_cycle = cycle

    if best_idx is None:
        raise ValueError(
            f"No '{bookkeeper_name}' CutBookkeeper found in {path!r} "
            f"matching streams {allowed_streams}. "
            f"Available entries: {list(zip(ak.to_list(names), ak.to_list(streams)))}"
        )

    return {
        "nEventsProcessed": int(n_acc[best_idx]),
        "sumOfWeights": float(sow[best_idx]),
        "sumOfWeightsSquared": float(sowsq[best_idx]),
        "nIncomplete": n_incomplete,
    }


def read_cutbookkeepers(
    files: Union[str, Path, list],
    *,
    input_stream: Union[str, list[str], None] = None,
    bookkeeper_name: str = "AllExecutedEvents",
) -> ak.Array:
    """Read CutBookkeeper metadata from one or more xAOD ROOT files.

    Returns an awkward record array with one entry per file, with fields:
      - nEventsProcessed  (int)
      - sumOfWeights      (float)
      - sumOfWeightsSquared (float)
      - nIncomplete       (int)  -- entries in IncompleteCutBookkeepers

    Parameters
    ----------
    files:
        A single file path (str or Path) or a list of file paths.
    input_stream:
        Allowed input stream name(s).  Defaults to the streams supported by
        CP::AsgCutBookkeeperAlg: StreamAOD, StreamDAOD_PHYSLITE, StreamEVGEN,
        StreamEVNT.  Pass a single string or list to restrict the selection.
    bookkeeper_name:
        Name of the CutBookkeeper entry to look for.  Defaults to
        "AllExecutedEvents", the standard entry written by ATLAS production.
    """
    if isinstance(files, (str, Path)):
        files = [files]

    if input_stream is None:
        allowed_streams = _DEFAULT_STREAMS
    elif isinstance(input_stream, str):
        allowed_streams = (input_stream,)
    else:
        allowed_streams = tuple(input_stream)

    records = [
        _read_one_file(path, allowed_streams, bookkeeper_name)
        for path in files
    ]

    return ak.Array(records)
