# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# Tests for the atlascp syntactic sugar module.

import os
import sys
sys.path.insert(0, os.path.dirname(__file__))

from testutils import _run_tests, approx, raises

import numpy as np
import awkward as ak

from ColumnarToolWrapperPython import Tool, atlascp


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def make_muon_events():
    """Build a minimal synthetic awkward array mimicking PHYSLITE muon columns."""
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


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

def test_creates_tool_instance():
    """atlascp constructor returns a Tool instance."""
    tool = atlascp.MuonEfficiencyScaleFactors("unique0")
    assert isinstance(tool, Tool)


def test_dynamic_subclass_type():
    """The returned instance's class is named after the tool type."""
    tool = atlascp.MuonEfficiencyScaleFactors("unique0")
    assert type(tool).__name__ == "MuonEfficiencyScaleFactors"
    assert Tool in type(tool).__mro__


def test_class_cache_consistency():
    """Two atlascp calls for the same type return instances of the same class."""
    tool_a = atlascp.MuonEfficiencyScaleFactors("unique0")
    tool_b = atlascp.MuonEfficiencyScaleFactors("unique1")
    assert type(tool_a) is type(tool_b)


def test_custom_namespace():
    """namespace keyword overrides the default 'CP' prefix."""
    tool = atlascp.VectorExampleTool("vecEx", namespace="columnar")
    assert isinstance(tool, Tool)
    assert type(tool).__name__ == "VectorExampleTool"


def test_properties_passed_through():
    """Properties dict is forwarded to the tool handle."""
    tool = atlascp.MuonEfficiencyScaleFactors("unique0")
    events = make_muon_events()
    result = tool(events)
    sf_values = result["Muons.sfOut"].to_list()
    assert len(sf_values) == 1
    assert sf_values[0][0] == approx(0.99509060382843018)


def test_keyword_only_enforcement():
    """properties must be passed as keyword argument, not positional."""
    with raises(TypeError):
        atlascp.MuonEfficiencyScaleFactors("unique0", None)


def test_positional_only_enforcement():
    """instance_name is positional-only and cannot be passed as keyword."""
    with raises(TypeError):
        atlascp.MuonEfficiencyScaleFactors(instance_name="unique0")


def test_no_name_creates_tool():
    """atlascp constructor works without providing an instance name."""
    tool = atlascp.MuonEfficiencyScaleFactors()
    assert isinstance(tool, Tool)


def test_no_name_unique_per_call():
    """Two no-name calls produce tools with different C++ handles."""
    tool_a = atlascp.MuonEfficiencyScaleFactors()
    tool_b = atlascp.MuonEfficiencyScaleFactors()
    assert tool_a._handle is not tool_b._handle


if __name__ == "__main__":
    _run_tests(sys.modules[__name__])
