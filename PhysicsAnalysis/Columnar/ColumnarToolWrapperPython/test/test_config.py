# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

import os
import sys
import tempfile
from pathlib import Path
sys.path.insert(0, os.path.dirname(__file__))

from testutils import _run_tests

import numpy as np
import awkward as ak

from ColumnarToolWrapperPython import atlascp
from ColumnarToolWrapperPython.config import load_config, merge_tool_config


def test_load_config_global_only():
    """load_config returns global section."""
    toml_content = '[global.rename_containers]\nEventInfo = "EventInfoAuxDyn"\nMuons = "AnalysisMuonsAuxDyn"\n'
    with tempfile.NamedTemporaryFile(mode="w", suffix=".toml", delete=False) as f:
        f.write(toml_content)
        path = Path(f.name)
    try:
        cfg = load_config(path)
        assert cfg["global"]["rename_containers"]["EventInfo"] == "EventInfoAuxDyn"
    finally:
        path.unlink()


def test_load_config_multiple_same_type():
    """load_config supports multiple [[tool.TypeName]] entries."""
    toml_content = (
        '[global.rename_containers]\nEventInfo = "EventInfoAuxDyn"\nMuons = "AnalysisMuonsAuxDyn"\n'
        '\n[[tool.MuonEfficiencyScaleFactors]]\nproperties.WorkingPoint = "Tight"\n'
        '\n[[tool.MuonEfficiencyScaleFactors]]\nproperties.WorkingPoint = "Medium"\n'
    )
    with tempfile.NamedTemporaryFile(mode="w", suffix=".toml", delete=False) as f:
        f.write(toml_content)
        path = Path(f.name)
    try:
        cfg = load_config(path)
        tools = cfg["tool"]["MuonEfficiencyScaleFactors"]
        assert len(tools) == 2
        assert tools[0]["properties"]["WorkingPoint"] == "Tight"
        assert tools[1]["properties"]["WorkingPoint"] == "Medium"
    finally:
        path.unlink()


def test_merge_tool_config_global_wins_when_no_tool_section():
    """merge_tool_config returns global values when tool entry has no overrides."""
    global_cfg = {
        "rename_containers": {"EventInfo": "EventInfoAuxDyn", "Muons": "AnalysisMuonsAuxDyn"},
        "properties": {},
    }
    tool_entry = {}
    merged = merge_tool_config(global_cfg, tool_entry)
    assert merged["rename_containers"] == {"EventInfo": "EventInfoAuxDyn", "Muons": "AnalysisMuonsAuxDyn"}
    assert merged["properties"] == {}


def test_merge_tool_config_tool_overrides_global():
    """merge_tool_config: tool-level keys override global keys."""
    global_cfg = {
        "rename_containers": {"EventInfo": "EventInfoAuxDyn", "Muons": "AnalysisMuonsAuxDyn"},
        "properties": {},
    }
    tool_entry = {
        "rename_containers": {"Muons": "SelectedMuonsAuxDyn"},
        "properties": {"WorkingPoint": "Tight"},
    }
    merged = merge_tool_config(global_cfg, tool_entry)
    assert merged["rename_containers"]["Muons"] == "SelectedMuonsAuxDyn"
    assert merged["rename_containers"]["EventInfo"] == "EventInfoAuxDyn"
    assert merged["properties"]["WorkingPoint"] == "Tight"


def make_muon_events():
    return ak.Array(
        {
            "EventInfoAuxDyn.runNumber": np.array([284500], dtype=np.uint32),
            "EventInfoAuxDyn.RandomRunNumber": np.array([284500], dtype=np.uint32),
            "EventInfoAuxDyn.eventTypeBitmask": np.array([1], dtype=np.uint32),
            "AnalysisMuonsAuxDyn.pt": ak.values_astype(ak.Array([[10e5]]), np.float32),
            "AnalysisMuonsAuxDyn.eta": ak.values_astype(ak.Array([[1.0]]), np.float32),
            "AnalysisMuonsAuxDyn.phi": ak.values_astype(ak.Array([[1.0]]), np.float32),
            "AnalysisMuonsAuxDyn.muonType": ak.values_astype(ak.Array([[0]]), np.uint16),
        }
    )


def test_configure_creates_corrections():
    """atlascp.configure() returns a Corrections with one tool."""
    toml_content = (
        '[global.rename_containers]\nEventInfo = "EventInfoAuxDyn"\nMuons = "AnalysisMuonsAuxDyn"\n'
        '\n[[tool.MuonEfficiencyScaleFactors]]\nproperties.WorkingPoint = "Tight"\n'
    )
    with tempfile.NamedTemporaryFile(mode="w", suffix=".toml", delete=False) as f:
        f.write(toml_content)
        path = Path(f.name)
    try:
        corrections = atlascp.configure(path)
        names = list(corrections)
        assert names[0] == "MuonEfficiencyScaleFactors"
        from ColumnarToolWrapperPython import Tool
        assert isinstance(corrections[names[0]][0], Tool)
    finally:
        path.unlink()


def test_configure_multiple_same_type():
    """atlascp.configure() creates multiple instances of same tool type."""
    toml_content = (
        '[global.rename_containers]\nEventInfo = "EventInfoAuxDyn"\nMuons = "AnalysisMuonsAuxDyn"\n'
        '\n[[tool.MuonEfficiencyScaleFactors]]\nproperties.WorkingPoint = "Tight"\n'
        '\n[[tool.MuonEfficiencyScaleFactors]]\nproperties.WorkingPoint = "Medium"\n'
    )
    with tempfile.NamedTemporaryFile(mode="w", suffix=".toml", delete=False) as f:
        f.write(toml_content)
        path = Path(f.name)
    try:
        corrections = atlascp.configure(path)
        all_tools = list(corrections.items())
        assert len(all_tools) == 2
        assert all_tools[0][0] == "MuonEfficiencyScaleFactors"
        assert all_tools[1][0] == "MuonEfficiencyScaleFactors"
        assert all_tools[0][1] is not all_tools[1][1]
    finally:
        path.unlink()


def test_corrections_apply():
    """Corrections.apply(events) runs all tools and returns merged outputs."""
    toml_content = (
        '[global.rename_containers]\nEventInfo = "EventInfoAuxDyn"\nMuons = "AnalysisMuonsAuxDyn"\n'
        '\n[[tool.MuonEfficiencyScaleFactors]]\nproperties.WorkingPoint = "Tight"\n'
    )
    with tempfile.NamedTemporaryFile(mode="w", suffix=".toml", delete=False) as f:
        f.write(toml_content)
        path = Path(f.name)
    try:
        corrections = atlascp.configure(path)
        events = make_muon_events()
        result = corrections.apply(events)
        assert "AnalysisMuonsAuxDyn.sfOut" in ak.fields(result)
    finally:
        path.unlink()


if __name__ == "__main__":
    _run_tests(sys.modules[__name__])
