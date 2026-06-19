# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# Tests for the StoreGate key (m_persKey) hash bindings.
#
# ElementLink branches in Athena-written xAOD files store m_persKey values
# computed by SG::StringPool::stringToKey (Control/SGTools/src/StringPool.cxx):
#
#   crc = CxxUtils::crc64(name)
#   if (clid) crc = CxxUtils::crc64addint(crc, clid)
#   key = crc & ((1 << 30) - 1)
#
# The CLID is the container class ID from the CLASS_DEF macro in the xAOD
# container headers (e.g. xAODTracking/TrackParticleContainer.h). The
# expected values below are the hard-coded knownKeys from
# ColumnarTestFixtures/Root/PhysliteTest.cxx, which were read directly from
# a PHYSLITE file.

import os
import sys
sys.path.insert(0, os.path.dirname(__file__))

from testutils import _run_tests

from ColumnarToolWrapperPython import crc64, crc64addint, invalid_link_value, sg_key

# CLIDs from the CLASS_DEF macros in the xAOD container headers
CLID_MUON_CONTAINER = 1178459224
CLID_ELECTRON_CONTAINER = 1087532415
CLID_PHOTON_CONTAINER = 1105575213
CLID_JET_CONTAINER = 1244316195
CLID_CALO_CLUSTER_CONTAINER = 1219821989
CLID_TRACK_PARTICLE_CONTAINER = 1287425431
CLID_VERTEX_CONTAINER = 1092961325

# {container_name: (clid, expected_sgkey)} — expected values are the
# knownKeys hard-coded in ColumnarTestFixtures/Root/PhysliteTest.cxx
KNOWN_KEYS = {
    "AnalysisMuons": (CLID_MUON_CONTAINER, 0x3A6B126F),
    "AnalysisElectrons": (CLID_ELECTRON_CONTAINER, 0x3902FEC0),
    "AnalysisPhotons": (CLID_PHOTON_CONTAINER, 0x35D1472F),
    "AnalysisJets": (CLID_JET_CONTAINER, 0x1AFD1919),
    "egammaClusters": (CLID_CALO_CLUSTER_CONTAINER, 0x15788D1F),
    "GSFConversionVertices": (CLID_VERTEX_CONTAINER, 0x1F3E85C9),
    "InDetTrackParticles": (CLID_TRACK_PARTICLE_CONTAINER, 0x1D3890DB),
    "CombinedMuonTrackParticles": (CLID_TRACK_PARTICLE_CONTAINER, 0x340D9196),
    "ExtrapolatedMuonTrackParticles": (CLID_TRACK_PARTICLE_CONTAINER, 0x14E35E9F),
    "GSFTrackParticles": (CLID_TRACK_PARTICLE_CONTAINER, 0x2E42DB0B),
    "InDetForwardTrackParticles": (CLID_TRACK_PARTICLE_CONTAINER, 0x143C6846),
    "MuonSpectrometerTrackParticles": (CLID_TRACK_PARTICLE_CONTAINER, 0x3993C8F3),
}

PHYSLITE_PATH = "/data/krumnack/DAOD_PHYSLITE_DEV_V3.root"


def test_sg_key_known_keys():
    """sg_key reproduces every hard-coded key from PhysliteTest.cxx."""
    for name, (clid, expected) in KNOWN_KEYS.items():
        assert sg_key(name, clid) == expected, name


def test_sg_key_requires_clid():
    """Without the CLID the hash differs (the CLID is mixed into the key)."""
    for name, (clid, expected) in KNOWN_KEYS.items():
        assert sg_key(name) != expected, name


def test_sg_key_is_30_bits():
    """Keys are masked to 30 bits (SG::StringPool::sgkey_t_nbits)."""
    for name, (clid, _expected) in KNOWN_KEYS.items():
        assert sg_key(name, clid) < (1 << 30)
        assert sg_key(name) < (1 << 30)


def test_crc64_empty_string():
    """crc64 of an empty string is the initial CRC value."""
    assert crc64("") == 0xFFFFFFFFFFFFFFFF


def test_crc64_composes_to_sg_key():
    """sg_key is crc64 + crc64addint masked to 30 bits."""
    name, clid = "InDetTrackParticles", CLID_TRACK_PARTICLE_CONTAINER
    crc = crc64addint(crc64(name), clid)
    assert crc & ((1 << 30) - 1) == sg_key(name, clid)
    # without a CLID, no crc64addint step
    assert crc64("AnalysisMuons") & ((1 << 30) - 1) == sg_key("AnalysisMuons")


def test_invalid_link_value():
    """invalid_link_value matches columnar::ColumnarModeArray::invalidLinkValue."""
    assert invalid_link_value == 0xFFFFFFFFFFFFFFFF


def test_sg_key_matches_physlite_file():
    """Integration: sg_key reproduces the m_persKey values stored in a real PHYSLITE file."""
    if not os.path.exists(PHYSLITE_PATH):
        print(f"skipping: {PHYSLITE_PATH} not available")
        return
    import awkward as ak
    import uproot

    with uproot.open(PHYSLITE_PATH) as f:
        tree = f["CollectionTree"]
        events = tree.arrays(
            [
                "AnalysisMuonsAuxDyn.inDetTrackParticleLink.m_persKey",
                "AnalysisPhotonsAuxDyn.caloClusterLinks",
            ],
            entry_stop=50,
        )

    muon_keys = set(
        ak.flatten(
            events["AnalysisMuonsAuxDyn.inDetTrackParticleLink.m_persKey"], axis=None
        ).to_list()
    )
    # only the non-null links carry the target key
    muon_keys.discard(0)
    assert muon_keys == {sg_key("InDetTrackParticles", CLID_TRACK_PARTICLE_CONTAINER)}

    cluster_keys = set(
        ak.flatten(
            events["AnalysisPhotonsAuxDyn.caloClusterLinks"]["m_persKey"], axis=None
        ).to_list()
    )
    cluster_keys.discard(0)
    assert cluster_keys == {sg_key("egammaClusters", CLID_CALO_CLUSTER_CONTAINER)}


if __name__ == "__main__":
    _run_tests(sys.modules[__name__])
