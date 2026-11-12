# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# Tests for ElementLink column support in the Tool wrapper.
#
# The tools request the key and index columns of a link (m_persKey and
# m_persIndex) as they are found in the file, and the wrapper passes them
# on without converting them.

import os
import sys
sys.path.insert(0, os.path.dirname(__file__))

from testutils import _run_tests, raises, unique_name as _unique_name, physlite_test_file

import awkward as ak
import numpy as np

from ColumnarToolWrapperPython import (
    Tool,
    sg_key,
)
from ColumnarToolWrapperPython.buffers import (
    _branch_name_for_column,
    _key_index_buffers,
    extract_buffers,
    _fill_event_index_columns,
)

# CLIDs from the CLASS_DEF macros in the xAOD container headers
CLID_TRACK_PARTICLE_CONTAINER = 1287425431
CLID_CALO_CLUSTER_CONTAINER = 1219821989

NAME_MAP = {
    "EventInfo": "EventInfoAuxDyn",
    "AnalysisMuons": "AnalysisMuonsAuxDyn",
    "CombinedMuonTrackParticles": "CombinedMuonTrackParticlesAuxDyn",
}

VARIANT_NAME_MAP = {
    "EventInfo": "EventInfoAuxDyn",
    "AnalysisMuons": "AnalysisMuonsAuxDyn",
    "InDetTrackParticles": "InDetTrackParticlesAuxDyn",
    "InDetForwardTrackParticles": "InDetForwardTrackParticlesAuxDyn",
}


class StubColumnInfo:
    """Minimal stand-in for ColumnInfo in helper-level tests."""

    def __init__(self, name, offset_name, is_optional=False):
        self.name = name
        self.offset_name = offset_name
        self.is_optional = is_optional


# ---------------------------------------------------------------------------
# Column metadata
# ---------------------------------------------------------------------------

def test_name_for_links():
    """Link target names are set and reported by the current column name."""
    def make_tool(**kwargs):
        return Tool(f"columnar_array::LinkColumnExampleTool/{_unique_name()}",
                    rename_containers=NAME_MAP, **kwargs)

    def name_for_links(tool):
        (col,) = [c for c in tool.columns if c.name == "CombinedMuonTrackParticlesAuxDyn"]
        return col.name_for_links

    assert name_for_links(make_tool()) == "CombinedMuonTrackParticles"
    tool = make_tool(link_target_names={"CombinedMuonTrackParticlesAuxDyn": "OtherTracks"})
    assert name_for_links(tool) == "OtherTracks"
    with raises(RuntimeError):
        make_tool(link_target_names={"CombinedMuonTrackParticles": "OtherTracks"})


def _make_link_tool_array():
    return Tool(
        f"columnar_array::LinkColumnExampleTool/{_unique_name()}",
        rename_containers=NAME_MAP,
    )


def _make_variant_link_tool():
    return Tool(
        f"columnar_array::VariantLinkColumnExampleTool/{_unique_name()}",
        rename_containers=VARIANT_NAME_MAP,
    )


def test_required_input_branch_names_variant_link():
    """A variant link needs the link columns and all containers it can point to."""
    assert sorted(_make_variant_link_tool().required_input_branch_names) == [
        "AnalysisMuonsAuxDyn.inDetTrackParticleLink.m_persIndex",
        "AnalysisMuonsAuxDyn.inDetTrackParticleLink.m_persKey",
        "InDetForwardTrackParticlesAuxDyn.qOverP",
        "InDetTrackParticlesAuxDyn.qOverP",
    ]


def test_required_input_branch_names_key_index():
    """One link per object resolves to the key and index sub-branches."""
    assert sorted(_make_link_tool_array().required_input_branch_names) == [
        "AnalysisMuonsAuxDyn.combinedTrackParticleLink.m_persIndex",
        "AnalysisMuonsAuxDyn.combinedTrackParticleLink.m_persKey",
        "CombinedMuonTrackParticlesAuxDyn.qOverP",
    ]


def test_required_input_branch_names_key_index_vector():
    """A vector of links resolves to the branch of the link itself."""
    tool = Tool(
        f"CP_array::EgammaCalibrationAndSmearingTool/{_unique_name()}",
        properties={
            "ESModel": "es2022_R22_PRE",
            "decorrelationModel": "1NP_v1",
            "useFastSim": 0,
            "doScaleCorrection": 0,
            "useMVACalibration": 0,
            "useLayerCorrection": 0,
            "onlyElectrons": 1,
        },
        rename_containers={
            "EventInfo": "EventInfoAuxDyn",
            "egammaClusters": "egammaClustersAuxDyn",
            "EGamma": "AnalysisElectronsAuxDyn",
            "Electrons": "AnalysisElectronsAuxDyn",
        },
    )
    branches = tool.required_input_branch_names
    assert "AnalysisElectronsAuxDyn.caloClusterLinks" in branches
    assert not [name for name in branches if name.endswith(".m_persKey")]


def test_required_input_branch_names_iterate():
    """Integration: uproot.iterate can read the branches of a key+index link."""
    path = os.environ.get("ASG_TEST_FILE_LITE_MC", "")
    if not os.path.exists(path):
        print(f"skipping: {path} not available")
        return
    import uproot

    tool = _make_link_tool_array()
    events = next(uproot.iterate({path: "CollectionTree"},
                                 tool.required_input_branch_names, step_size=100))
    result = tool(events)
    assert ak.sum(ak.num(result["AnalysisMuonsAuxDyn.selection"], axis=1)) > 0


# ---------------------------------------------------------------------------
# Key+index link columns
# ---------------------------------------------------------------------------

def _key_index_column(name, offset_name):
    col = StubColumnInfo(name, offset_name)
    col.dtype = "uint32"
    return col


def _uint32(values):
    return ak.values_astype(ak.Array(values), np.uint32)


def test_key_index_scalar_link():
    """The columns are the data in the file: no offsetting, no null conversion."""
    events = ak.Array(
        {
            "Muons.trackLink.m_persKey": _uint32([[1234], [0, 1234]]),
            "Muons.trackLink.m_persIndex": _uint32([[1], [0, 7]]),
        }
    )
    key_offsets, keys = _key_index_buffers(
        events, _key_index_column("Muons.trackLink.m_persKey", "Muons"))
    index_offsets, indices = _key_index_buffers(
        events, _key_index_column("Muons.trackLink.m_persIndex", "Muons"))
    assert key_offsets is None and index_offsets is None
    assert keys.tolist() == [1234, 0, 1234]
    assert indices.tolist() == [1, 0, 7]
    assert keys.dtype == np.uint32


def test_key_index_vector_link():
    """Links in a vector also provide the offset map of that vector."""
    events = ak.Array(
        {"Muons.links.m_persIndex": _uint32([[[1, 2], []], [[3]]])}
    )
    events = ak.with_field(events, _uint32([[[9, 9], []], [[9]]]), "Muons.links.m_persKey")
    offsets, indices = _key_index_buffers(
        events, _key_index_column("Muons.links.m_persIndex", "Muons.links.offset"))
    assert offsets.tolist() == [0, 2, 2, 3]
    assert offsets.dtype == np.uint64
    assert indices.tolist() == [1, 2, 3]


def test_key_index_record_layout():
    """Reading the parent branch gives the columns as a record."""
    events = ak.Array({"Muons.links": ak.zip(
        {"m_persKey": _uint32([[9], [9, 9]]), "m_persIndex": _uint32([[1], [2, 3]])})})
    _, indices = _key_index_buffers(
        events, _key_index_column("Muons.links.m_persIndex", "Muons"))
    assert indices.tolist() == [1, 2, 3]


def test_key_index_no_copy():
    """Without an event selection the column is a view on the input buffer."""
    index = _uint32([[1], [0, 7]])
    events = ak.Array(
        {"Muons.trackLink.m_persKey": index, "Muons.trackLink.m_persIndex": index}
    )
    _, indices = _key_index_buffers(
        events, _key_index_column("Muons.trackLink.m_persIndex", "Muons"))
    assert np.shares_memory(indices, np.asarray(index.layout.content.data))


def test_key_index_event_selection():
    index = _uint32([[1], [0, 7], [5]])
    events = ak.Array(
        {"Muons.trackLink.m_persKey": index, "Muons.trackLink.m_persIndex": index}
    )
    _, indices = _key_index_buffers(
        events[np.array([False, True, True])],
        _key_index_column("Muons.trackLink.m_persIndex", "Muons"))
    assert indices.tolist() == [0, 7, 5]


def test_key_index_wrong_type():
    """A wrong data type is an error, not something that gets converted."""
    index = ak.values_astype(ak.Array([[1]]), np.uint64)
    events = ak.Array(
        {"Muons.trackLink.m_persKey": index, "Muons.trackLink.m_persIndex": index}
    )
    with raises(RuntimeError):
        _key_index_buffers(
            events, _key_index_column("Muons.trackLink.m_persIndex", "Muons"))


def test_key_index_branch_name():
    assert _branch_name_for_column("Muons.trackLink.m_persKey") == "Muons.trackLink"
    assert _branch_name_for_column("Muons.trackLink.m_persIndex") == "Muons.trackLink"
    assert _branch_name_for_column("Muons.trackLink.data") == "Muons.trackLink"
    assert _branch_name_for_column("Muons.pt") == "Muons.pt"


# ---------------------------------------------------------------------------
# Offset maps of containers without input columns
# ---------------------------------------------------------------------------

def _classified_without_inputs():
    def entry(name, offset_name):
        return {
            "offset": StubColumnInfo(name, offset_name),
            "inputs": [],
            "outputs": [],
            "nested_offsets": {},
            "event_indices": [],
        }
    return {
        "EventInfo": entry("EventInfo", ""),
        "Tracks": entry("Tracks", "EventInfo"),
    }


def test_container_offsets_read_from_fields():
    events = ak.Array({"Tracks.pt": [[1.0, 2.0], [], [3.0]]})
    buffers = extract_buffers(events, _classified_without_inputs())
    assert buffers["EventInfo"].tolist() == [0, 3]
    assert buffers["Tracks"].tolist() == [0, 2, 2, 3]


def test_container_offsets_missing_raises():
    """An offset map that is not in the input is an error, not made up."""
    events = ak.Array({"Muons.pt": [[1.0, 2.0], [], [3.0]]})
    with raises(RuntimeError):
        extract_buffers(events, _classified_without_inputs())


# ---------------------------------------------------------------------------
# Tool-level behavior (synthetic events)
# ---------------------------------------------------------------------------

def test_fill_event_index_columns():
    # 3 events with 2, 0, 1 muons; the muons have 1, 2, 0 vector entries
    classified = {
        "EventInfo": {
            "offset": StubColumnInfo("EventInfo", ""),
            "nested_offsets": {},
            "event_indices": [],
        },
        "AnalysisMuons": {
            "offset": StubColumnInfo("AnalysisMuons", "EventInfo"),
            "nested_offsets": {
                "AnalysisMuons.links.offset": {
                    "event_indices": [StubColumnInfo(
                        "AnalysisMuons.links.eventIndex",
                        "AnalysisMuons.links.offset")],
                },
            },
            "event_indices": [StubColumnInfo(
                "AnalysisMuons.eventIndex", "AnalysisMuons")],
        },
    }
    buffers = {
        "EventInfo": np.array([0, 3], dtype=np.uint64),
        "AnalysisMuons": np.array([0, 2, 2, 3], dtype=np.uint64),
        "AnalysisMuons.links.offset": np.array([0, 1, 3, 3], dtype=np.uint64),
    }
    _fill_event_index_columns(classified, buffers, 3)
    assert buffers["AnalysisMuons.eventIndex"].tolist() == [0, 0, 2]
    assert buffers["AnalysisMuons.eventIndex"].dtype == np.uint64
    assert buffers["AnalysisMuons.links.eventIndex"].tolist() == [0, 0, 0]


# ---------------------------------------------------------------------------
# Integration: real PHYSLITE file
# ---------------------------------------------------------------------------

def test_tool_call_with_links_physlite():
    """Integration: LinkColumnExampleTool on a real PHYSLITE file.

    Cross-checks the tool's selection against a manual recomputation from
    the m_persIndex/m_persKey and qOverP branches.
    """
    physlite_path = physlite_test_file()
    if physlite_path is None:
        print("skipping: the PHYSLITE test file is not available")
        return
    import uproot

    tool = _make_link_tool_array()
    with uproot.open(physlite_path) as f:
        tree = f["CollectionTree"]
        events = tree.arrays(tool.required_input_branch_names, entry_stop=100)
        # read the sub-branches separately for the manual cross-check
        manual = tree.arrays(
            [
                "AnalysisMuonsAuxDyn.combinedTrackParticleLink.m_persKey",
                "AnalysisMuonsAuxDyn.combinedTrackParticleLink.m_persIndex",
            ],
            entry_stop=100,
        )

    result = tool(events)
    selection = result["AnalysisMuonsAuxDyn.selection"]

    keys = manual["AnalysisMuonsAuxDyn.combinedTrackParticleLink.m_persKey"]
    indices = manual["AnalysisMuonsAuxDyn.combinedTrackParticleLink.m_persIndex"]
    qoverp = events["CombinedMuonTrackParticlesAuxDyn.qOverP"]

    n_muons = 0
    n_selected = 0
    for event in range(len(events)):
        for muon in range(len(indices[event])):
            n_muons += 1
            key = keys[event][muon]
            index = indices[event][muon]
            if (key == 0 and index == 0) or index == 0xFFFFFFFF:
                expected = 0
            else:
                expected = int(abs(qoverp[event][index]) < 1.0 / 10e3)
            assert selection[event][muon] == expected, (event, muon)
            n_selected += expected
    assert n_muons > 0
    assert 0 < n_selected <= n_muons


def test_variant_tool_call_with_links_physlite():
    """Integration: VariantLinkColumnExampleTool on a real PHYSLITE file.

    The inner detector track link of a muon points into
    InDetTrackParticles, or for forward muons into
    InDetForwardTrackParticles. This cross-checks the tool's selection
    against a manual recomputation that picks the container from the
    link key.
    """
    physlite_path = physlite_test_file()
    if physlite_path is None:
        print("skipping: the PHYSLITE test file is not available")
        return
    import uproot

    tool = _make_variant_link_tool()
    with uproot.open(physlite_path) as f:
        tree = f["CollectionTree"]
        events = tree.arrays(tool.required_input_branch_names, entry_stop=100)

    result = tool(events)
    selection = result["AnalysisMuonsAuxDyn.selection"]

    keys = events["AnalysisMuonsAuxDyn.inDetTrackParticleLink.m_persKey"]
    indices = events["AnalysisMuonsAuxDyn.inDetTrackParticleLink.m_persIndex"]
    qoverp = {
        sg_key("InDetTrackParticles", CLID_TRACK_PARTICLE_CONTAINER):
            events["InDetTrackParticlesAuxDyn.qOverP"],
        sg_key("InDetForwardTrackParticles", CLID_TRACK_PARTICLE_CONTAINER):
            events["InDetForwardTrackParticlesAuxDyn.qOverP"],
    }

    n_muons = 0
    n_selected = 0
    n_forward = 0
    for event in range(len(events)):
        for muon in range(len(indices[event])):
            n_muons += 1
            key = keys[event][muon]
            index = indices[event][muon]
            if (key == 0 and index == 0) or index == 0xFFFFFFFF:
                expected = 0
            else:
                expected = int(abs(qoverp[key][event][index]) < 1.0 / 10e3)
                n_forward += key == sg_key("InDetForwardTrackParticles", CLID_TRACK_PARTICLE_CONTAINER)
            assert selection[event][muon] == expected, (event, muon)
            n_selected += expected
    assert n_muons > 0
    assert n_forward > 0  # make sure the second container is exercised
    assert 0 < n_selected <= n_muons


if __name__ == "__main__":
    _run_tests(sys.modules[__name__])
