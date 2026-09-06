# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# Tests for the high-level Tool wrapper and buffers utilities.

import os
import sys
sys.path.insert(0, os.path.dirname(__file__))

from testutils import _run_tests, approx, unique_name, xfail

import numpy as np
import awkward as ak

import ColumnarToolWrapperPython
from ColumnarToolWrapperPython import Tool
from ColumnarToolWrapperPython.buffers import (
    _inner_most_list_offset_array,
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
    assert sf_values[0][0] == approx(0.99569094181060791)
    assert valid_values[0][0] == 1


def test_tool_call_masked_events():
    """Tool.__call__ works when events array is masked (e.g. events[selection])."""
    tool = make_muon_eff_tool()
    # Two events: select only the first
    two_events = ak.Array(
        {
            "EventInfo.runNumber": np.array([284500, 284500], dtype=np.uint32),
            "EventInfo.RandomRunNumber": np.array([284500, 284500], dtype=np.uint32),
            "EventInfo.eventTypeBitmask": np.array([1, 1], dtype=np.uint32),
            "Muons.pt": ak.values_astype(ak.Array([[10e5], [10e5]]), np.float32),
            "Muons.eta": ak.values_astype(ak.Array([[1.0], [1.0]]), np.float32),
            "Muons.phi": ak.values_astype(ak.Array([[1.0], [1.0]]), np.float32),
            "Muons.muonType": ak.values_astype(ak.Array([[0], [0]]), np.uint16),
        }
    )
    selection = np.array([True, False])
    events_filtered = two_events[selection]

    # This used to crash with KeyError: 'EventInfoAuxDyn1-data'
    result = tool(events_filtered)

    sf_values = result["Muons.sfOut"].to_list()
    assert len(sf_values) == 1
    assert len(sf_values[0]) == 1
    assert sf_values[0][0] == approx(0.99509060382843018)


def test_tool_systematics():
    """Applying a systematic variation changes the output SF."""
    tool = make_muon_eff_tool()
    events = make_muon_events()

    nominal = tool(events)["Muons.sfOut"].to_list()[0][0]

    tool.apply_systematic_variation("MUON_EFF_RECO_SYS__1up")
    varied = tool(events)["Muons.sfOut"].to_list()[0][0]

    assert nominal != approx(varied)


def test_tool_call_systematic_kwarg():
    """Tool.__call__(events, systematic=...) applies variation inline and resets to nominal."""
    tool = make_muon_eff_tool()
    events = make_muon_events()

    nominal = tool(events)["Muons.sfOut"].to_list()[0][0]
    varied = tool(events, systematic="MUON_EFF_RECO_SYS__1up")["Muons.sfOut"].to_list()[0][0]

    # Variation must differ from nominal
    assert nominal != approx(varied)

    # After the inline call, tool must have reset to nominal automatically
    after = tool(events)["Muons.sfOut"].to_list()[0][0]
    assert after == approx(nominal)


def test_extract_buffers_nested_vector_synthetic():
    """extract_buffers populates Particles, .offset, .data, and Particles.pt buffers."""
    pt = ak.values_astype(ak.Array([[10e5, 10e5, 1e3]]), np.float32)
    numtrk = ak.values_astype(ak.Array([[[3, 1, 0], [1], [0]]]), np.int32)
    sumpt = ak.values_astype(ak.Array([[[3000.0], [500.0], [0.0]]]), np.float32)
    events = ak.Array(
        {
            "Particles.pt": pt,
            "Particles.NumTrkPt500": numtrk,
            "Particles.SumPtTrkPt500": sumpt,
        }
    )

    handle = ColumnarToolWrapperPython.PythonToolHandle()
    handle.set_type_and_name(f"columnar::VectorExampleTool/{unique_name()}")
    handle.initialize()

    classified = classify_columns(handle.columns)
    classified = resolve_optional_columns(classified, events)

    buf = extract_buffers(events, classified)

    np.testing.assert_array_equal(buf["Particles"], [0, 3])
    np.testing.assert_array_equal(buf["Particles.pt"], [10e5, 10e5, 1e3])
    np.testing.assert_array_equal(buf["Particles.NumTrkPt500.offset"], [0, 3, 4, 5])
    np.testing.assert_array_equal(buf["Particles.NumTrkPt500.data"], [3, 1, 0, 1, 0])
    np.testing.assert_array_equal(buf["Particles.SumPtTrkPt500.offset"], [0, 1, 2, 3])
    np.testing.assert_allclose(buf["Particles.SumPtTrkPt500.data"], [3000.0, 500.0, 0.0])

    assert buf["Particles.NumTrkPt500.offset"].dtype == np.uint64
    assert buf["Particles.SumPtTrkPt500.offset"].dtype == np.uint64
    np.testing.assert_array_equal(buf["EventInfo"], [0, 1])


def test_tool_vector_example():
    """VectorExampleTool initializes and exposes nested vector columns."""
    tool = Tool("columnar::VectorExampleTool/vecEx")
    col_names = [c.name for c in tool.columns]
    # VectorExampleTool should have NumTrkPt500 as a nested vector column
    assert any("NumTrkPt500" in n for n in col_names)
    assert any("selection" in n for n in col_names)


def test_tool_vector_example_call_synthetic():
    """Tool.__call__ runs VectorExampleTool on a synthetic var * var * T awkward array."""
    tool = Tool(f"columnar::VectorExampleTool/{unique_name()}")

    pt = ak.values_astype(ak.Array([[10e5, 10e5, 1e3]]), np.float32)
    numtrk = ak.values_astype(ak.Array([[[3, 1, 0], [1], [0]]]), np.int32)
    sumpt = ak.values_astype(ak.Array([[[3000.0], [500.0], [0.0]]]), np.float32)
    events = ak.Array(
        {
            "Particles.pt": pt,
            "Particles.NumTrkPt500": numtrk,
            "Particles.SumPtTrkPt500": sumpt,
        }
    )

    result = tool(events)

    selection = result["Particles.selection"].to_list()
    # particle 0: pt>10e3 && trknum[0]=3>2 && trksumpt[0]=3000>2e3 → 1
    # particle 1: trknum[0]=1, fails > 2 → 0
    # particle 2: pt=1e3 < 10e3 → 0
    assert selection == [[1, 0, 0]]


def test_classify_columns_nested_vector_inputs_routed():
    """classify_columns groups VectorExampleTool nested-vector data columns under their nested offset."""
    handle = ColumnarToolWrapperPython.PythonToolHandle()
    handle.set_type_and_name(f"columnar::VectorExampleTool/{unique_name()}")
    handle.initialize()

    classified = classify_columns(handle.columns)

    assert "Particles" in classified
    nested = classified["Particles"]["nested_offsets"]

    expected_offsets = {"Particles.NumTrkPt500.offset", "Particles.SumPtTrkPt500.offset"}
    assert set(nested.keys()) == expected_offsets

    for offset_name in expected_offsets:
        entry = nested[offset_name]
        assert entry["offset"].name == offset_name
        assert entry["offset"].is_offset is True
        data_names = [c.name for c in entry["inputs"]]
        assert data_names == [offset_name.rsplit(".", 1)[0] + ".data"]
        assert entry["outputs"] == []

    flat_input_names = [c.name for c in classified["Particles"]["inputs"]]
    assert "Particles.pt" in flat_input_names
    assert "Particles.NumTrkPt500.data" not in flat_input_names
    assert "Particles.SumPtTrkPt500.data" not in flat_input_names

    flat_output_names = [c.name for c in classified["Particles"]["outputs"]]
    assert "Particles.selection" in flat_output_names


def test_tool_vector_example_call_physlite():
    """Tool.__call__ runs VectorExampleTool on real PHYSLITE data via uproot."""
    physlite_path = "/data/krumnack/DAOD_PHYSLITE_DEV_V3.root"
    if not os.path.exists(physlite_path):
        print(f"SKIP: {physlite_path} not present")
        return

    import uproot

    tool = Tool(
        f"columnar::VectorExampleTool/{unique_name()}",
        rename_containers={"Particles": "AnalysisJetsAuxDyn"},
    )

    branches = tool.required_input_branch_names
    with uproot.open(physlite_path) as f:
        events = f["CollectionTree"].arrays(branches, entry_stop=10)

    result = tool(events)
    selection = result["AnalysisJetsAuxDyn.selection"]

    assert len(selection) == len(events)
    n_per_event_in = ak.num(events["AnalysisJetsAuxDyn.pt"], axis=1).to_list()
    n_per_event_out = ak.num(selection, axis=1).to_list()
    assert n_per_event_in == n_per_event_out
    flat = ak.flatten(selection).to_list()
    assert all(v in (0, 1) for v in flat)


def test_required_input_branch_names_strips_data_suffix():
    """required_input_branch_names dedupes and strips the .data suffix on nested-vector inputs."""
    tool = Tool(f"columnar::VectorExampleTool/{unique_name()}")
    branches = tool.required_input_branch_names
    branches_set = set(branches)
    assert "Particles.pt" in branches_set
    assert "Particles.NumTrkPt500" in branches_set
    assert "Particles.SumPtTrkPt500" in branches_set
    # No .data leaks out
    assert not any(b.endswith(".data") for b in branches_set)
    # No offset columns and no output columns
    assert "Particles" not in branches_set
    assert "Particles.selection" not in branches_set
    # No duplicates
    assert len(branches) == len(branches_set)


def test_required_input_branch_names_muon_eff_sf_unchanged():
    """For tools without nested vectors, required_input_branch_names matches non-optional inputs."""
    tool = make_muon_eff_tool()
    branches = set(tool.required_input_branch_names)
    expected = {c.name for c in tool.input_columns if not c.is_optional}
    assert branches >= expected


def test_inner_most_list_offset_array_var_var_int():
    """_inner_most_list_offset_array returns a singly-jagged view of a var*var*int."""
    arr = ak.values_astype(ak.Array([[[3, 1, 0], [1]], [[2, 4]]]), np.int32)
    inner = _inner_most_list_offset_array(arr)
    # Inner offsets cumulative across all particles in the event range:
    # particle 0: 3 entries, particle 1: 1 entry, particle 2: 2 entries → [0, 3, 4, 6]
    np.testing.assert_array_equal(np.asarray(inner.layout.offsets.data), [0, 3, 4, 6])
    np.testing.assert_array_equal(np.asarray(inner.layout.content.data), [3, 1, 0, 1, 2, 4])


def test_inner_most_list_offset_array_var_int_passthrough():
    """A singly-jagged var*int input is returned unchanged in shape."""
    arr = ak.values_astype(ak.Array([[3, 1, 0], [2, 4]]), np.int32)
    inner = _inner_most_list_offset_array(arr)
    np.testing.assert_array_equal(np.asarray(inner.layout.offsets.data), [0, 3, 5])
    np.testing.assert_array_equal(np.asarray(inner.layout.content.data), [3, 1, 0, 2, 4])


if __name__ == "__main__":
    _run_tests(sys.modules[__name__])
