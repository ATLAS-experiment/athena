# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# Tests for ElementLink column support in the Tool wrapper.
#
# Link columns are identified by ColumnInfo.sole_link_target_name. On the
# array side a link is a uint64 global offset into the flattened target
# container (columnar::ColumnarModeArray), with invalid (null) links set to
# invalid_link_value. PHYSLITE files store per-event indices (m_persIndex)
# and target container hashes (m_persKey), so the wrapper converts:
#
#   global = target_offsets[event] + m_persIndex
#   (m_persKey == 0 and m_persIndex == 0)  ->  invalid_link_value
#
# mirroring LinkColumnVector::addSplitLink in
# ColumnarTestFixtures/Root/ColumnarPhysliteTest.cxx.

import os
import sys
sys.path.insert(0, os.path.dirname(__file__))

from testutils import _run_tests, raises, unique_name as _unique_name

import awkward as ak
import numpy as np

from ColumnarToolWrapperPython import (
    Tool,
    expected_link_key,
    invalid_link_value,
    link_key_map,
    sg_key,
)
from ColumnarToolWrapperPython.buffers import (
    _convert_scalar_link_column,
    _convert_vector_link_column,
)

PHYSLITE_PATH = "/data/krumnack/DAOD_PHYSLITE_DEV_V3.root"

# CLIDs from the CLASS_DEF macros in the xAOD container headers
CLID_TRACK_PARTICLE_CONTAINER = 1287425431
CLID_CALO_CLUSTER_CONTAINER = 1219821989

NAME_MAP = {
    "EventInfo": "EventInfoAuxDyn",
    "AnalysisMuons": "AnalysisMuonsAuxDyn",
    "InDetTrackParticles": "InDetTrackParticlesAuxDyn",
}


class StubColumnInfo:
    """Minimal stand-in for ColumnInfo in helper-level tests."""

    def __init__(self, name, offset_name, sole_link_target_name,
                 is_variant_link=False, is_optional=False,
                 sole_link_target_clid=0):
        self.name = name
        self.offset_name = offset_name
        self.sole_link_target_name = sole_link_target_name
        self.is_variant_link = is_variant_link
        self.is_optional = is_optional
        self.sole_link_target_clid = sole_link_target_clid


def _make_link_tool():
    return Tool(
        f"columnar::LinkColumnExampleTool/{_unique_name()}",
        rename_containers=NAME_MAP,
    )


# ---------------------------------------------------------------------------
# Column metadata
# ---------------------------------------------------------------------------

def test_link_column_metadata():
    """The link column reports its target container via sole_link_target_name."""
    tool = _make_link_tool()
    (link_col,) = [c for c in tool.input_columns if c.sole_link_target_name]
    assert link_col.name == "AnalysisMuonsAuxDyn.inDetTrackParticleLink"
    assert link_col.sole_link_target_name == "InDetTrackParticlesAuxDyn"
    assert link_col.dtype == "uint64"
    # the CLID of the xAOD container the link targets
    assert link_col.sole_link_target_clid == CLID_TRACK_PARTICLE_CONTAINER
    assert link_col.to_dict()["sole_link_target_clid"] == CLID_TRACK_PARTICLE_CONTAINER


def test_expected_link_key():
    """expected_link_key hashes the SG name (branch prefix sans AuxDyn/Aux)."""
    assert expected_link_key(
        StubColumnInfo("Muons.trackLink", "Muons", "InDetTrackParticlesAuxDyn",
                       sole_link_target_clid=CLID_TRACK_PARTICLE_CONTAINER)
    ) == sg_key("InDetTrackParticles", CLID_TRACK_PARTICLE_CONTAINER)
    # unrenamed canonical container names are already the SG name
    assert expected_link_key(
        StubColumnInfo("Muons.trackLink", "Muons", "InDetTrackParticles",
                       sole_link_target_clid=CLID_TRACK_PARTICLE_CONTAINER)
    ) == sg_key("InDetTrackParticles", CLID_TRACK_PARTICLE_CONTAINER)
    # static-aux branch prefixes drop the 'Aux' suffix
    assert expected_link_key(
        StubColumnInfo("Muons.trackLink", "Muons", "InDetTrackParticlesAux",
                       sole_link_target_clid=CLID_TRACK_PARTICLE_CONTAINER)
    ) == sg_key("InDetTrackParticles", CLID_TRACK_PARTICLE_CONTAINER)
    # without a CLID the key cannot be computed
    assert expected_link_key(
        StubColumnInfo("Muons.trackLink", "Muons", "InDetTrackParticlesAuxDyn")
    ) is None
    # non-link columns have no key
    assert expected_link_key(StubColumnInfo("Muons.pt", "Muons", "")) is None


def test_link_key_map():
    """link_key_map reverse-resolves m_persKey values to target containers."""
    tool = _make_link_tool()
    keys = link_key_map(tool.columns)
    assert keys == {
        sg_key("InDetTrackParticles", CLID_TRACK_PARTICLE_CONTAINER):
            "InDetTrackParticlesAuxDyn"
    }
    # the actual value stored in PHYSLITE files
    assert keys[0x1D3890DB] == "InDetTrackParticlesAuxDyn"


def test_link_key_map_unrenamed():
    """Without rename_containers the canonical names are the SG names."""
    tool = Tool(f"columnar::LinkColumnExampleTool/{_unique_name()}")
    keys = link_key_map(tool.columns)
    assert keys == {
        sg_key("InDetTrackParticles", CLID_TRACK_PARTICLE_CONTAINER):
            "InDetTrackParticles"
    }


def test_required_input_branch_names():
    """Link columns resolve to readable uproot branch names."""
    tool = _make_link_tool()
    assert tool.required_input_branch_names == [
        "AnalysisMuonsAuxDyn.inDetTrackParticleLink",
        "InDetTrackParticlesAuxDyn.qOverP",
    ]


# ---------------------------------------------------------------------------
# Scalar link conversion (helper level)
# ---------------------------------------------------------------------------

def test_convert_scalar_link():
    """Per-event indices become global offsets; (0, 0) links become invalid."""
    events = ak.Array(
        {
            "Muons.trackLink.m_persKey": [[1234], [0, 1234]],
            "Muons.trackLink.m_persIndex": [[1], [0, 0]],
        }
    )
    col = StubColumnInfo("Muons.trackLink", "Muons", "Tracks")
    target_offsets = np.array([0, 2, 5], dtype=np.uint64)

    converted = _convert_scalar_link_column(events, col, target_offsets)

    assert converted.dtype == np.uint64
    assert list(converted) == [1, invalid_link_value, 2]


def test_convert_scalar_link_key_zero_index_nonzero_is_valid():
    """A key of 0 with a regular index does not mark a null link by itself."""
    events = ak.Array(
        {
            "Muons.trackLink.m_persKey": [[0]],
            "Muons.trackLink.m_persIndex": [[1]],
        }
    )
    col = StubColumnInfo("Muons.trackLink", "Muons", "Tracks")
    target_offsets = np.array([0, 2], dtype=np.uint64)

    converted = _convert_scalar_link_column(events, col, target_offsets)
    assert list(converted) == [1]


def test_convert_scalar_link_invalid_index_is_null():
    """m_persIndex == 0xFFFFFFFF (ElementLinkBase::INVALID) marks a null link."""
    events = ak.Array(
        {
            "Muons.trackLink.m_persKey": [[0, 1234]],
            "Muons.trackLink.m_persIndex": [[0xFFFFFFFF, 0xFFFFFFFF]],
        }
    )
    col = StubColumnInfo("Muons.trackLink", "Muons", "Tracks")
    target_offsets = np.array([0, 2], dtype=np.uint64)

    converted = _convert_scalar_link_column(events, col, target_offsets)
    assert list(converted) == [invalid_link_value, invalid_link_value]


def test_convert_scalar_link_out_of_range():
    """A link index beyond its event's target range raises."""
    events = ak.Array(
        {
            "Muons.trackLink.m_persKey": [[1234]],
            "Muons.trackLink.m_persIndex": [[5]],
        }
    )
    col = StubColumnInfo("Muons.trackLink", "Muons", "Tracks")
    target_offsets = np.array([0, 2], dtype=np.uint64)

    with raises(RuntimeError):
        _convert_scalar_link_column(events, col, target_offsets)


def test_convert_scalar_link_missing_fields():
    """Missing m_persKey/m_persIndex fields produce a clear error."""
    events = ak.Array({"Muons.trackLink.m_persIndex": [[1]]})
    col = StubColumnInfo("Muons.trackLink", "Muons", "Tracks")
    target_offsets = np.array([0, 2], dtype=np.uint64)

    with raises(RuntimeError):
        _convert_scalar_link_column(events, col, target_offsets)


# ---------------------------------------------------------------------------
# Vector link conversion (helper level)
# ---------------------------------------------------------------------------

def test_convert_vector_link():
    """vector<ElementLink> columns produce nested offsets plus converted data."""
    # 2 events; event 0 has one object with 1 link, event 1 has two objects
    # with 2 and 0 links respectively
    links = ak.Array(
        [
            [[{"m_persKey": 99, "m_persIndex": 1}]],
            [
                [
                    {"m_persKey": 99, "m_persIndex": 0},
                    {"m_persKey": 0, "m_persIndex": 0},
                ],
                [],
            ],
        ]
    )
    events = ak.Array({"Photons.clusterLinks": links})
    col = StubColumnInfo(
        "Photons.clusterLinks.data", "Photons.clusterLinks.offset", "Clusters"
    )
    target_offsets = np.array([0, 3, 5], dtype=np.uint64)

    offsets, data = _convert_vector_link_column(events, col, target_offsets)

    assert offsets.dtype == np.uint64
    assert data.dtype == np.uint64
    # one offset entry per object plus one
    assert list(offsets) == [0, 1, 3, 3]
    assert list(data) == [1, 3, invalid_link_value]


def test_convert_vector_link_out_of_range():
    """A vector link index beyond its event's target range raises."""
    links = ak.Array([[[{"m_persKey": 99, "m_persIndex": 7}]]])
    events = ak.Array({"Photons.clusterLinks": links})
    col = StubColumnInfo(
        "Photons.clusterLinks.data", "Photons.clusterLinks.offset", "Clusters"
    )
    target_offsets = np.array([0, 3], dtype=np.uint64)

    with raises(RuntimeError):
        _convert_vector_link_column(events, col, target_offsets)


# ---------------------------------------------------------------------------
# Tool-level behavior (synthetic events)
# ---------------------------------------------------------------------------

def test_tool_call_with_links():
    """End to end with LinkColumnExampleTool: selection via linked-track q/p.

    The tool selects muons whose linked track has |qOverP| < 1/ptCut
    (default ptCut = 10e3, i.e. threshold 1e-4); muons with a null link
    fail the selection.
    """
    events = ak.Array(
        {
            "AnalysisMuonsAuxDyn.inDetTrackParticleLink.m_persKey": [[1234], [0, 1234]],
            "AnalysisMuonsAuxDyn.inDetTrackParticleLink.m_persIndex": [[1], [0, 0]],
            "InDetTrackParticlesAuxDyn.qOverP": ak.values_astype(
                ak.Array([[2e-4, 4e-5], [3e-4, 5e-5]]), np.float32
            ),
        }
    )
    tool = _make_link_tool()
    result = tool(events)

    # event 0: muon -> track 1 (|4e-5| < 1e-4 -> pass)
    # event 1: muon 0 -> null link (fail); muon 1 -> track 0 (|3e-4| -> fail)
    assert result["AnalysisMuonsAuxDyn.selection"].to_list() == [[1], [0, 0]]


def test_tool_call_with_links_event_selection():
    """Link conversion respects prior event selections (masked arrays)."""
    events = ak.Array(
        {
            "AnalysisMuonsAuxDyn.inDetTrackParticleLink.m_persKey": [[1234], [0, 1234], [1234]],
            "AnalysisMuonsAuxDyn.inDetTrackParticleLink.m_persIndex": [[1], [0, 0], [0]],
            "InDetTrackParticlesAuxDyn.qOverP": ak.values_astype(
                ak.Array([[2e-4, 4e-5], [3e-4, 5e-5], [9e-5]]), np.float32
            ),
        }
    )
    tool = _make_link_tool()
    result = tool(events[[True, False, True]])

    assert result["AnalysisMuonsAuxDyn.selection"].to_list() == [[1], [1]]


# ---------------------------------------------------------------------------
# Integration: real PHYSLITE file
# ---------------------------------------------------------------------------

def test_tool_call_with_links_physlite():
    """Integration: LinkColumnExampleTool on a real PHYSLITE file.

    Cross-checks the tool's selection against a manual recomputation from
    the m_persIndex/m_persKey and qOverP branches.
    """
    if not os.path.exists(PHYSLITE_PATH):
        print(f"skipping: {PHYSLITE_PATH} not available")
        return
    import uproot

    tool = _make_link_tool()
    with uproot.open(PHYSLITE_PATH) as f:
        tree = f["CollectionTree"]
        events = tree.arrays(tool.required_input_branch_names, entry_stop=100)
        # read the sub-branches separately for the manual cross-check
        manual = tree.arrays(
            [
                "AnalysisMuonsAuxDyn.inDetTrackParticleLink.m_persKey",
                "AnalysisMuonsAuxDyn.inDetTrackParticleLink.m_persIndex",
            ],
            entry_stop=100,
        )

    result = tool(events)
    selection = result["AnalysisMuonsAuxDyn.selection"]

    keys = manual["AnalysisMuonsAuxDyn.inDetTrackParticleLink.m_persKey"]
    indices = manual["AnalysisMuonsAuxDyn.inDetTrackParticleLink.m_persIndex"]
    qoverp = events["InDetTrackParticlesAuxDyn.qOverP"]

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


def test_egamma_tool_with_vector_links_physlite():
    """Integration: EgammaCalibrationAndSmearingTool reads caloClusterLinks.

    Uses the tested configuration from
    PhysicsAnalysis/ElectronPhotonID/ElectronPhotonFourMomentumCorrection/
    test/gt_ColumnarToolTests.cxx. Exercises the vector-of-links path
    (AnalysisElectronsAuxDyn.caloClusterLinks.data) end to end.
    """
    if not os.path.exists(PHYSLITE_PATH):
        print(f"skipping: {PHYSLITE_PATH} not available")
        return
    import uproot

    tool = Tool(
        f"CP::EgammaCalibrationAndSmearingTool/{_unique_name()}",
        properties={
            "ESModel": "es2022_R22_PRE",
            "decorrelationModel": "1NP_v1",
            "useFastSim": 0,
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

    # the vector link column carries the calo-cluster container CLID, so the
    # m_persKey values stored in the file can be reverse-resolved
    (link_col,) = [c for c in tool.input_columns if c.sole_link_target_name]
    assert link_col.sole_link_target_clid == CLID_CALO_CLUSTER_CONTAINER
    keys = link_key_map(tool.columns)
    assert keys[sg_key("egammaClusters", CLID_CALO_CLUSTER_CONTAINER)] == "egammaClustersAuxDyn"

    with uproot.open(PHYSLITE_PATH) as f:
        tree = f["CollectionTree"]
        events = tree.arrays(tool.required_input_branch_names, entry_stop=50)

    # every key stored in the file resolves to the expected target
    file_keys = set(
        ak.flatten(
            events["AnalysisElectronsAuxDyn.caloClusterLinks"]["m_persKey"], axis=None
        ).to_list()
    )
    file_keys.discard(0)
    assert {keys[k] for k in file_keys} == {"egammaClustersAuxDyn"}

    result = tool(events)
    pt_out = result["AnalysisElectronsAuxDyn.ptOut"]
    pt_in = events["AnalysisElectronsAuxDyn.pt"]

    # same jagged structure as the input electrons
    assert ak.num(pt_out, axis=1).to_list() == ak.num(pt_in, axis=1).to_list()
    assert ak.sum(ak.num(pt_out, axis=1)) > 0
    # the calibrated pt is a (smallish) correction on the input pt
    ratios = ak.flatten(pt_out / pt_in, axis=None).to_list()
    assert all(0.5 < r < 1.5 for r in ratios)

    # a systematic variation produces a different output (the AF3
    # variations are no-ops for full sim, so use the scale variation)
    assert "EG_SCALE_ALL__1up" in tool.recommended_systematics
    varied = tool(events, systematic="EG_SCALE_ALL__1up")["AnalysisElectronsAuxDyn.ptOut"]
    assert varied.to_list() != pt_out.to_list()


if __name__ == "__main__":
    _run_tests(sys.modules[__name__])
