# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

import os
import sys
from pathlib import Path
from unittest.mock import MagicMock, patch
sys.path.insert(0, os.path.dirname(__file__))

from testutils import _run_tests

import numpy as np
import awkward as ak
import pytest

from ColumnarToolWrapperPython.cutbookkeeper import read_cutbookkeepers


def _make_mock_file(cbk_entries, incomplete_count=0):
    """Return a mock uproot file object with MetaData tree populated from cbk_entries.

    cbk_entries: list of dicts with keys:
        name, cycle, inputStream, nAcceptedEvents,
        sumOfEventWeights, sumOfEventWeightsSquared
    incomplete_count: number of entries in IncompleteCutBookkeepers
    """
    names = ak.Array([[e["name"] for e in cbk_entries]])
    cycles = ak.Array([[e["cycle"] for e in cbk_entries]])
    streams = ak.Array([[e["inputStream"] for e in cbk_entries]])
    n_acc = ak.Array([[e["nAcceptedEvents"] for e in cbk_entries]])
    sow = ak.Array([[e["sumOfEventWeights"] for e in cbk_entries]])
    sowsq = ak.Array([[e["sumOfEventWeightsSquared"] for e in cbk_entries]])
    incomplete = ak.Array([[0] * incomplete_count])

    branch_data = {
        "CutBookkeepersAux./CutBookkeepersAux.name": names,
        "CutBookkeepersAux./CutBookkeepersAux.cycle": cycles,
        "CutBookkeepersAux./CutBookkeepersAux.inputStream": streams,
        "CutBookkeepersAux./CutBookkeepersAux.nAcceptedEvents": n_acc,
        "CutBookkeepersAux./CutBookkeepersAux.sumOfEventWeights": sow,
        "CutBookkeepersAux./CutBookkeepersAux.sumOfEventWeightsSquared": sowsq,
        "IncompleteCutBookkeepersAux./IncompleteCutBookkeepersAux.nAcceptedEvents": incomplete,
    }

    mock_branch = MagicMock()
    mock_branch.array.side_effect = lambda: None  # will be overridden per-branch

    mock_md = MagicMock()
    mock_md.__getitem__ = lambda self, key: _make_branch(branch_data[key])

    mock_file = MagicMock()
    mock_file.__getitem__ = lambda self, key: mock_md
    mock_file.__enter__ = lambda self: self
    mock_file.__exit__ = MagicMock(return_value=False)

    return mock_file


def _make_branch(data):
    """Return a mock branch whose .array() returns data."""
    b = MagicMock()
    b.array = MagicMock(return_value=data)
    return b


def _patch_uproot(cbk_entries, incomplete_count=0):
    """Context manager: patch uproot.open to return mock data for any path."""
    mock_file = _make_mock_file(cbk_entries, incomplete_count)
    return patch("ColumnarToolWrapperPython.cutbookkeeper.uproot.open", return_value=mock_file)


# ---------------------------------------------------------------------------
# Unit tests
# ---------------------------------------------------------------------------

class TestReadCutBookkeepersBasic:
    """Basic reading from a single file."""

    def test_reads_all_executed_events(self):
        """Finds AllExecutedEvents and extracts the payload correctly."""
        entries = [
            {"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamAOD",
             "nAcceptedEvents": 100, "sumOfEventWeights": 200.0, "sumOfEventWeightsSquared": 50000.0},
        ]
        with _patch_uproot(entries):
            result = read_cutbookkeepers("fake.root")
        assert len(result) == 1
        assert int(result[0].nEventsProcessed) == 100
        assert float(result[0].sumOfWeights) == pytest.approx(200.0)
        assert float(result[0].sumOfWeightsSquared) == pytest.approx(50000.0)
        assert int(result[0].nIncomplete) == 0

    def test_accepts_string_path(self):
        """Accepts a plain string path, not just a Path object."""
        entries = [
            {"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamAOD",
             "nAcceptedEvents": 42, "sumOfEventWeights": 84.0, "sumOfEventWeightsSquared": 168.0},
        ]
        with _patch_uproot(entries):
            result = read_cutbookkeepers("fake.root")
        assert int(result[0].nEventsProcessed) == 42

    def test_accepts_path_object(self):
        """Accepts a pathlib.Path object."""
        entries = [
            {"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamAOD",
             "nAcceptedEvents": 42, "sumOfEventWeights": 84.0, "sumOfEventWeightsSquared": 168.0},
        ]
        with _patch_uproot(entries):
            result = read_cutbookkeepers(Path("fake.root"))
        assert int(result[0].nEventsProcessed) == 42

    def test_incomplete_count(self):
        """nIncomplete reflects the number of entries in IncompleteCutBookkeepers."""
        entries = [
            {"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamAOD",
             "nAcceptedEvents": 50, "sumOfEventWeights": 100.0, "sumOfEventWeightsSquared": 200.0},
        ]
        with _patch_uproot(entries, incomplete_count=3):
            result = read_cutbookkeepers("fake.root")
        assert int(result[0].nIncomplete) == 3


class TestCycleSelection:
    """Max-cycle selection logic."""

    def test_picks_max_cycle(self):
        """When multiple AllExecutedEvents entries exist, picks the one with the highest cycle."""
        entries = [
            {"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamAOD",
             "nAcceptedEvents": 100, "sumOfEventWeights": 100.0, "sumOfEventWeightsSquared": 100.0},
            {"name": "AllExecutedEvents", "cycle": 3, "inputStream": "StreamAOD",
             "nAcceptedEvents": 300, "sumOfEventWeights": 300.0, "sumOfEventWeightsSquared": 300.0},
            {"name": "AllExecutedEvents", "cycle": 2, "inputStream": "StreamAOD",
             "nAcceptedEvents": 200, "sumOfEventWeights": 200.0, "sumOfEventWeightsSquared": 200.0},
        ]
        with _patch_uproot(entries):
            result = read_cutbookkeepers("fake.root")
        assert int(result[0].nEventsProcessed) == 300

    def test_other_entries_ignored(self):
        """Non-AllExecutedEvents entries (e.g. VertexSelection) are ignored."""
        entries = [
            {"name": "VertexSelection", "cycle": 5, "inputStream": "StreamAOD",
             "nAcceptedEvents": 999, "sumOfEventWeights": 999.0, "sumOfEventWeightsSquared": 999.0},
            {"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamAOD",
             "nAcceptedEvents": 10, "sumOfEventWeights": 20.0, "sumOfEventWeightsSquared": 40.0},
        ]
        with _patch_uproot(entries):
            result = read_cutbookkeepers("fake.root")
        assert int(result[0].nEventsProcessed) == 10


class TestStreamFiltering:
    """Input stream filtering."""

    def test_physlite_stream_accepted(self):
        """StreamDAOD_PHYSLITE is accepted by default (matches AsgCutBookkeeperAlg)."""
        entries = [
            {"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamDAOD_PHYSLITE",
             "nAcceptedEvents": 77, "sumOfEventWeights": 77.0, "sumOfEventWeightsSquared": 77.0},
        ]
        with _patch_uproot(entries):
            result = read_cutbookkeepers("fake.root")
        assert int(result[0].nEventsProcessed) == 77

    def test_evgen_stream_accepted(self):
        """StreamEVGEN is accepted by default."""
        entries = [
            {"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamEVGEN",
             "nAcceptedEvents": 55, "sumOfEventWeights": 55.0, "sumOfEventWeightsSquared": 55.0},
        ]
        with _patch_uproot(entries):
            result = read_cutbookkeepers("fake.root")
        assert int(result[0].nEventsProcessed) == 55

    def test_unknown_stream_rejected(self):
        """An entry with an unrecognized stream is not selected."""
        entries = [
            {"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamSOMETHING_UNKNOWN",
             "nAcceptedEvents": 99, "sumOfEventWeights": 99.0, "sumOfEventWeightsSquared": 99.0},
        ]
        with _patch_uproot(entries):
            with pytest.raises(ValueError, match="AllExecutedEvents"):
                read_cutbookkeepers("fake.root")

    def test_custom_stream_override(self):
        """The input_stream parameter restricts which streams are accepted."""
        entries = [
            {"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamAOD",
             "nAcceptedEvents": 10, "sumOfEventWeights": 10.0, "sumOfEventWeightsSquared": 10.0},
            {"name": "AllExecutedEvents", "cycle": 2, "inputStream": "StreamDAOD_PHYSLITE",
             "nAcceptedEvents": 20, "sumOfEventWeights": 20.0, "sumOfEventWeightsSquared": 20.0},
        ]
        with _patch_uproot(entries):
            result = read_cutbookkeepers("fake.root", input_stream="StreamAOD")
        assert int(result[0].nEventsProcessed) == 10

    def test_custom_stream_list(self):
        """input_stream can be a list of strings."""
        entries = [
            {"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamAOD",
             "nAcceptedEvents": 10, "sumOfEventWeights": 10.0, "sumOfEventWeightsSquared": 10.0},
        ]
        with _patch_uproot(entries):
            result = read_cutbookkeepers("fake.root", input_stream=["StreamAOD", "StreamEVGEN"])
        assert int(result[0].nEventsProcessed) == 10

    def test_no_match_raises(self):
        """Raises ValueError when no matching entry is found."""
        entries = [
            {"name": "SomeOtherBookkeeper", "cycle": 1, "inputStream": "StreamAOD",
             "nAcceptedEvents": 1, "sumOfEventWeights": 1.0, "sumOfEventWeightsSquared": 1.0},
        ]
        with _patch_uproot(entries):
            with pytest.raises(ValueError, match="AllExecutedEvents"):
                read_cutbookkeepers("fake.root")


class TestMultiFile:
    """Multi-file aggregation."""

    def test_returns_one_record_per_file(self):
        """Result array has one entry per input file."""
        entries_a = [{"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamAOD",
                      "nAcceptedEvents": 100, "sumOfEventWeights": 100.0, "sumOfEventWeightsSquared": 100.0}]
        entries_b = [{"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamAOD",
                      "nAcceptedEvents": 200, "sumOfEventWeights": 200.0, "sumOfEventWeightsSquared": 200.0}]
        mock_a = _make_mock_file(entries_a)
        mock_b = _make_mock_file(entries_b)

        call_count = [0]
        def open_side_effect(path):
            i = call_count[0]
            call_count[0] += 1
            return [mock_a, mock_b][i]

        with patch("ColumnarToolWrapperPython.cutbookkeeper.uproot.open", side_effect=open_side_effect):
            result = read_cutbookkeepers(["a.root", "b.root"])
        assert len(result) == 2
        assert int(result[0].nEventsProcessed) == 100
        assert int(result[1].nEventsProcessed) == 200

    def test_ak_sum_works(self):
        """ak.sum() on the result gives the total across files."""
        counts = [100, 200, 300]
        mocks = [
            _make_mock_file([{"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamAOD",
                              "nAcceptedEvents": n, "sumOfEventWeights": float(n) * 2,
                              "sumOfEventWeightsSquared": float(n) * 3}])
            for n in counts
        ]
        call_count = [0]
        def open_side_effect(path):
            i = call_count[0]
            call_count[0] += 1
            return mocks[i]

        with patch("ColumnarToolWrapperPython.cutbookkeeper.uproot.open", side_effect=open_side_effect):
            result = read_cutbookkeepers(["a.root", "b.root", "c.root"])
        assert int(ak.sum(result.nEventsProcessed)) == 600
        assert float(ak.sum(result.sumOfWeights)) == pytest.approx(1200.0)
        assert float(ak.sum(result.sumOfWeightsSquared)) == pytest.approx(1800.0)

    def test_single_file_in_list(self):
        """A single-element list works the same as passing the path directly."""
        entries = [{"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamAOD",
                    "nAcceptedEvents": 55, "sumOfEventWeights": 110.0, "sumOfEventWeightsSquared": 220.0}]
        with _patch_uproot(entries):
            result_list = read_cutbookkeepers(["fake.root"])
        with _patch_uproot(entries):
            result_single = read_cutbookkeepers("fake.root")
        assert int(result_list[0].nEventsProcessed) == int(result_single[0].nEventsProcessed)


class TestCustomBookkeeperName:
    """bookkeeper_name parameter."""

    def test_custom_name(self):
        """Can select a different bookkeeper by name."""
        entries = [
            {"name": "AllExecutedEvents", "cycle": 1, "inputStream": "StreamAOD",
             "nAcceptedEvents": 100, "sumOfEventWeights": 100.0, "sumOfEventWeightsSquared": 100.0},
            {"name": "PHYSLITEKernel", "cycle": 1, "inputStream": "StreamAOD",
             "nAcceptedEvents": 80, "sumOfEventWeights": 80.0, "sumOfEventWeightsSquared": 80.0},
        ]
        with _patch_uproot(entries):
            result = read_cutbookkeepers("fake.root", bookkeeper_name="PHYSLITEKernel")
        assert int(result[0].nEventsProcessed) == 80


# ---------------------------------------------------------------------------
# Integration test (requires real PHYSLITE file)
# ---------------------------------------------------------------------------

PHYSLITE_PATH = Path("/data/krumnack/DAOD_PHYSLITE_DEV_V3.root")


@pytest.mark.skipif(not PHYSLITE_PATH.exists(), reason="real PHYSLITE file not available")
class TestRealPHYSLITEFile:
    """Integration tests against a real PHYSLITE file."""

    def test_reads_real_file(self):
        """Read CutBookkeepers from real PHYSLITE file and check plausible values."""
        result = read_cutbookkeepers(PHYSLITE_PATH)
        assert len(result) == 1
        assert int(result[0].nEventsProcessed) > 0
        assert float(result[0].sumOfWeights) > 0.0
        assert float(result[0].sumOfWeightsSquared) > 0.0

    def test_real_file_known_values(self):
        """Check the actual values observed when probing the file."""
        result = read_cutbookkeepers(PHYSLITE_PATH)
        assert int(result[0].nEventsProcessed) == 200
        assert float(result[0].sumOfWeights) == pytest.approx(145561.916, rel=1e-4)
        assert float(result[0].sumOfWeightsSquared) == pytest.approx(108092396.188, rel=1e-4)


if __name__ == "__main__":
    _run_tests(__file__)
