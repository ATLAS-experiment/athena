# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# Tests for the high-level Tool wrapper and buffers utilities.

import os
import sys
sys.path.insert(0, os.path.dirname(__file__))

from testutils import _run_tests, approx, xfail

import numpy as np
import awkward as ak

import ColumnarToolWrapperPython
from ColumnarToolWrapperPython import Tool
from ColumnarToolWrapperPython.buffers import (
    classify_columns,
    resolve_optional_columns,
    extract_buffers,
    allocate_outputs,
    reconstruct_output,
)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def make_muon_events():
    """Build a minimal synthetic awkward array mimicking PHYSLITE muon columns.

    One event, one muon.  Field names match those reported by
    MuonEfficiencyScaleFactors after rename_containers (i.e. no prefix).
    """
    return ak.Array(
        {
            "EventInfo.runNumber": np.array([284500], dtype=np.uint32),
            "EventInfo.RandomRunNumber": np.array([284500], dtype=np.uint32),
            "EventInfo.eventTypeBitmask": np.array([1], dtype=np.uint32),
            "Muons.pt": ak.values_astype(ak.Array([[10e5]]), np.float32),
            "Muons.eta": ak.values_astype(ak.Array([[1.0]]), np.float32),
            "Muons.phi": ak.values_astype(ak.Array([[1.0]]), np.float32),
            "Muons.muonType": ak.values_astype(ak.Array([[0]]), np.uint16),
        }
    )


def make_muon_eff_tool():
    """Return an initialized MuonEfficiencyScaleFactors Tool."""
    return Tool("CP::MuonEfficiencyScaleFactors/unique0")


# ---------------------------------------------------------------------------
# Unit tests
# ---------------------------------------------------------------------------

def test_tool_init():
    """Tool creation populates columns."""
    tool = make_muon_eff_tool()
    col_names = [c.name for c in tool.columns]
    assert "Muons.pt" in col_names
    assert "Muons.sfOut" in col_names
    assert "Muons.validOut" in col_names
    # Should have at least one input and one output column
    assert len(tool.input_columns) > 0
    assert len(tool.output_columns) > 0


def test_tool_recommended_systematics():
    """Tool exposes recommended systematics."""
    tool = make_muon_eff_tool()
    systs = tool.recommended_systematics
    assert "" in systs  # nominal
    assert any("MUON_EFF_RECO" in s for s in systs)


def test_classify_columns():
    """classify_columns groups ColumnInfo into containers correctly."""
    handle = ColumnarToolWrapperPython.PythonToolHandle()
    handle.set_type_and_name("CP::MuonEfficiencyScaleFactors/unique0")
    handle.initialize()

    classified = classify_columns(handle.columns)

    # Should have EventInfo and Muons containers
    assert "EventInfo" in classified
    assert "Muons" in classified

    # EventInfo has no output columns, Muons has output columns
    assert len(classified["EventInfo"]["outputs"]) == 0
    assert len(classified["Muons"]["outputs"]) > 0

    # Offset columns themselves should not appear in inputs/outputs
    input_names = [c.name for c in classified["Muons"]["inputs"]]
    assert "Muons" not in input_names  # offset not in inputs
    assert "Muons.pt" in input_names
    output_names = [c.name for c in classified["Muons"]["outputs"]]
    assert "Muons.sfOut" in output_names
    assert "Muons.validOut" in output_names


def test_resolve_optional_columns_present():
    """Optional columns present in events are kept."""
    handle = ColumnarToolWrapperPython.PythonToolHandle()
    handle.set_type_and_name("CP::MuonEfficiencyScaleFactors/unique0")
    handle.initialize()
    classified = classify_columns(handle.columns)

    # Build events that include isLRT
    events = make_muon_events()
    # Add isLRT so it looks present
    fields = {f: events[f] for f in ak.fields(events)}
    fields["Muons.isLRT"] = ak.values_astype(ak.Array([[True]]), np.int8)
    events_with_lrt = ak.Array(fields)

    effective = resolve_optional_columns(classified, events_with_lrt)
    muon_input_names = [c.name for c in effective["Muons"]["inputs"]]
    assert "Muons.isLRT" in muon_input_names


def test_resolve_optional_columns_absent():
    """Optional columns absent from events are removed."""
    handle = ColumnarToolWrapperPython.PythonToolHandle()
    handle.set_type_and_name("CP::MuonEfficiencyScaleFactors/unique0")
    handle.initialize()
    classified = classify_columns(handle.columns)

    # make_muon_events() does NOT include isLRT
    events = make_muon_events()
    effective = resolve_optional_columns(classified, events)
    muon_input_names = [c.name for c in effective["Muons"]["inputs"]]
    assert "Muons.isLRT" not in muon_input_names


def test_tool_call_synthetic():
    """Tool.__call__ returns correct SF for 1 event, 1 muon."""
    tool = make_muon_eff_tool()
    events = make_muon_events()
    result = tool(events)

    sf_values = result["Muons.sfOut"].to_list()
    valid_values = result["Muons.validOut"].to_list()

    # One event, one muon
    assert len(sf_values) == 1
    assert len(sf_values[0]) == 1
    assert sf_values[0][0] == approx(0.99509060382843018)
    assert valid_values[0][0] == 1


def test_tool_systematics():
    """Applying a systematic variation changes the output SF."""
    tool = make_muon_eff_tool()
    events = make_muon_events()

    nominal = tool(events)["Muons.sfOut"].to_list()[0][0]

    tool.apply_systematic_variation("MUON_EFF_RECO_SYS__1up")
    varied = tool(events)["Muons.sfOut"].to_list()[0][0]

    assert nominal != approx(varied)



@xfail(reason="nested vectors not yet implemented in extract_buffers")
def test_extract_buffers_nested_vector():
    """extract_buffers handles var * var * int32 (nested vector) correctly."""
    # Build a synthetic var * var * int32 awkward array mimicking NumTrkPt500
    # Two events: first has 2 muons with [3, 1] tracks, second has 1 muon with [2] tracks
    inner = ak.values_astype(ak.Array([[[3, 1, 0], [1]], [[2, 4]]]), np.int32)
    events = ak.Array({"Particles.NumTrkPt500": inner})

    # Manually build a minimal classified structure for this field
    # We simulate what classify_columns would produce for VectorExampleTool
    # by building a fake classified dict
    from unittest.mock import MagicMock
    from ColumnarToolWrapperPython import ColumnAccessMode

    container_offset = MagicMock()
    container_offset.name = "Particles"
    container_offset.is_offset = True
    container_offset.offset_name = ""
    container_offset.is_optional = False

    nested_offset = MagicMock()
    nested_offset.name = "Particles.NumTrkPt500.offset"
    nested_offset.is_offset = True
    nested_offset.offset_name = "Particles"
    nested_offset.is_optional = False

    data_col = MagicMock()
    data_col.name = "Particles.NumTrkPt500"
    data_col.is_offset = False
    data_col.offset_name = "Particles.NumTrkPt500.offset"
    data_col.access_mode = ColumnAccessMode.input
    data_col.is_optional = False

    classified = {
        "Particles": {
            "offset": container_offset,
            "inputs": [data_col],
            "outputs": [],
            "nested_offsets": {
                "Particles.NumTrkPt500.offset": nested_offset,
            },
        }
    }

    buf = extract_buffers(events, classified)
    assert "Particles" in buf
    assert "Particles.NumTrkPt500.offset" in buf
    assert "Particles.NumTrkPt500" in buf


def test_tool_vector_example():
    """VectorExampleTool initializes and exposes nested vector columns."""
    tool = Tool("columnar::VectorExampleTool/vecEx")
    col_names = [c.name for c in tool.columns]
    # VectorExampleTool should have NumTrkPt500 as a nested vector column
    assert any("NumTrkPt500" in n for n in col_names)
    assert any("selection" in n for n in col_names)


if __name__ == "__main__":
    _run_tests(sys.modules[__name__])
