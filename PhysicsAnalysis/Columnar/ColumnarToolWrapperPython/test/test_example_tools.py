# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# Python translations of the ColumnarMemoryTest tests from
# PhysicsAnalysis/Columnar/ColumnarExampleTools/test/gt_ColumnarToolTests.cxx
#
# Each test uses PythonToolHandle directly (low-level API) to mirror the
# C++ test pattern: set columns manually, call, check expectations.
#
# Tests skipped:
#   - gt_ColumnarToolWrapper.cxx: C++ internals not exposed to Python
#   - gt_MultiToolTest.cxx: PHYSLITE-only, requires real data file

import os
import sys
sys.path.insert(0, os.path.dirname(__file__))

from testutils import _run_tests, unique_name as _unique_name

import numpy as np

from ColumnarToolWrapperPython import PythonToolHandle


# ---------------------------------------------------------------------------
# SimpleSelectorExampleTool
# ---------------------------------------------------------------------------

def test_SimpleSelectorExampleTool():
    """Particles with pt >= 10e5 pass selection (int8 output)."""
    handle = PythonToolHandle()
    handle.set_type_and_name(f"columnar::SimpleSelectorExampleTool/{_unique_name()}")
    handle.initialize()

    event_info = np.array([0, 2], dtype=np.uint64)
    particles = np.array([0, 1, 3], dtype=np.uint64)
    particles_pt = np.array([10e5, 10e5, 1e3], dtype=np.float32)
    selection = np.array([0, 0, 0], dtype=np.int8)

    handle["EventInfo"] = event_info
    handle["Particles"] = particles
    handle["Particles.pt"] = particles_pt
    handle.set_column_void("Particles.selection", selection, False)

    handle.call()

    assert list(selection) == [1, 1, 0]


# ---------------------------------------------------------------------------
# OptionalColumnExampleTool
# ---------------------------------------------------------------------------

def test_OptionalColumnExampleTool_present():
    """When ptCorr is present, tool uses it for selection."""
    handle = PythonToolHandle()
    handle.set_type_and_name(f"columnar::OptionalColumnExampleTool/{_unique_name()}")
    handle.initialize()

    event_info = np.array([0, 2], dtype=np.uint64)
    particles = np.array([0, 1, 3], dtype=np.uint64)
    particles_pt = np.array([10e5, 10e5, 1e3], dtype=np.float32)
    particles_ptcorr = np.array([10e5, 1e3, 10e5], dtype=np.float32)
    selection = np.array([0, 0, 0], dtype=np.int8)

    handle["EventInfo"] = event_info
    handle["Particles"] = particles
    handle["Particles.pt"] = particles_pt
    handle["Particles.ptCorr"] = particles_ptcorr
    handle.set_column_void("Particles.selection", selection, False)

    handle.call()

    assert list(selection) == [1, 0, 1]


def test_OptionalColumnExampleTool_absent():
    """When ptCorr is absent, tool falls back to pt for selection."""
    handle = PythonToolHandle()
    handle.set_type_and_name(f"columnar::OptionalColumnExampleTool/{_unique_name()}")
    handle.initialize()

    event_info = np.array([0, 2], dtype=np.uint64)
    particles = np.array([0, 1, 3], dtype=np.uint64)
    particles_pt = np.array([10e5, 10e5, 1e3], dtype=np.float32)
    selection = np.array([0, 0, 0], dtype=np.int8)

    handle["EventInfo"] = event_info
    handle["Particles"] = particles
    handle["Particles.pt"] = particles_pt
    handle.set_column_void("Particles.selection", selection, False)

    handle.call()

    assert list(selection) == [1, 1, 0]


# ---------------------------------------------------------------------------
# ConfigurableColumnExampleTool
# ---------------------------------------------------------------------------

def test_ConfigurableColumnExampleTool():
    """With ptVar='ptCorr' property, tool reads ptCorr column."""
    handle = PythonToolHandle()
    handle.set_type_and_name(f"columnar::ConfigurableColumnExampleTool/{_unique_name()}")
    handle.set_property("ptVar", "ptCorr")
    handle.initialize()

    event_info = np.array([0, 2], dtype=np.uint64)
    particles = np.array([0, 1, 3], dtype=np.uint64)
    particles_ptcorr = np.array([10e5, 10e5, 1e3], dtype=np.float32)
    selection = np.array([0, 0, 0], dtype=np.int8)

    handle["EventInfo"] = event_info
    handle["Particles"] = particles
    handle["Particles.ptCorr"] = particles_ptcorr
    handle.set_column_void("Particles.selection", selection, False)

    handle.call()

    assert list(selection) == [1, 1, 0]


# ---------------------------------------------------------------------------
# MomentumAccessorExampleTool
# ---------------------------------------------------------------------------

def test_MomentumAccessorExampleTool():
    """Jet ObjectType=2: selects on pt and eta cuts."""
    handle = PythonToolHandle()
    handle.set_type_and_name(f"columnar::MomentumAccessorExampleTool/{_unique_name()}")
    # ObjectType 2 corresponds to xAODType::Jet
    handle.set_property("ObjectType", 2)
    handle.initialize()

    event_info = np.array([0, 2], dtype=np.uint64)
    particles = np.array([0, 1, 3], dtype=np.uint64)
    particles_pt = np.array([10e5, 10e5, 1e3], dtype=np.float32)
    particles_eta = np.array([0, 3, 0], dtype=np.float32)
    particles_phi = np.array([0, 0, 0], dtype=np.float32)
    particles_m = np.array([0, 0, 0], dtype=np.float32)
    selection = np.array([0, 0, 0], dtype=np.int8)

    handle["EventInfo"] = event_info
    handle["Particles"] = particles
    handle["Particles.pt"] = particles_pt
    handle["Particles.eta"] = particles_eta
    handle["Particles.phi"] = particles_phi
    handle["Particles.m"] = particles_m
    handle.set_column_void("Particles.selection", selection, False)

    handle.call()

    assert list(selection) == [1, 1, 0]


# ---------------------------------------------------------------------------
# ModularExampleTool
# ---------------------------------------------------------------------------

def test_ModularExampleTool():
    """pt cut and |eta| < 2.5 cut applied: first passes both, second fails eta."""
    handle = PythonToolHandle()
    handle.set_type_and_name(f"columnar::ModularExampleTool/{_unique_name()}")
    handle.initialize()

    event_info = np.array([0, 2], dtype=np.uint64)
    particles = np.array([0, 1, 3], dtype=np.uint64)
    particles_pt = np.array([10e5, 10e5, 1e3], dtype=np.float32)
    particles_eta = np.array([0, 3, 0], dtype=np.float32)
    selection = np.array([0, 0, 0], dtype=np.int8)

    handle["EventInfo"] = event_info
    handle["Particles"] = particles
    handle["Particles.pt"] = particles_pt
    handle["Particles.eta"] = particles_eta
    handle.set_column_void("Particles.selection", selection, False)

    handle.call()

    assert list(selection) == [1, 0, 0]


# ---------------------------------------------------------------------------
# StringExampleTool
# ---------------------------------------------------------------------------

def test_StringExampleTool():
    """String column with nested offsets: 'Final' matches, 'Invisible' does not."""
    handle = PythonToolHandle()
    handle.set_type_and_name(f"columnar::StringExampleTool/{_unique_name()}")
    handle.initialize()

    event_info = np.array([0, 1], dtype=np.uint64)
    met = np.array([0, 2], dtype=np.uint64)
    # Met.name.offset is a nested-vector offset (dtype uint64)
    met_name_offset = np.array([0, 9, 14], dtype=np.uint64)
    # Met.name.data is int8 (char) data
    # 'Invisible' = [73,110,118,105,115,105,98,108,101]
    # 'Final'     = [70,105,110,97,108]
    met_name_data = np.array([ord(c) for c in "InvisibleFinal"], dtype=np.int8)
    selection = np.array([0, 0], dtype=np.int8)

    handle["EventInfo"] = event_info
    handle["Met"] = met
    handle["Met.name.offset"] = met_name_offset
    handle["Met.name.data"] = met_name_data
    handle.set_column_void("Met.selection", selection, False)

    handle.call()

    assert list(selection) == [0, 1]


# ---------------------------------------------------------------------------
# VariantExampleTool
# ---------------------------------------------------------------------------

def test_VariantExampleTool():
    """Cross-container pt/eta ranking across electrons and muons."""
    handle = PythonToolHandle()
    handle.set_type_and_name(f"columnar::VariantExampleTool/{_unique_name()}")
    handle.initialize()

    event_info = np.array([0, 1], dtype=np.uint64)
    electrons = np.array([0, 2], dtype=np.uint64)
    electrons_pt = np.array([10e3, 50e3], dtype=np.float32)
    electrons_eta = np.array([0, -1], dtype=np.float32)
    e_pt_rank = np.array([0, 0], dtype=np.uint16)
    e_eta_rank = np.array([0, 0], dtype=np.uint16)
    muons = np.array([0, 1], dtype=np.uint64)
    muons_pt = np.array([30e3], dtype=np.float32)
    muons_eta = np.array([0.5], dtype=np.float32)
    m_pt_rank = np.array([0], dtype=np.uint16)

    handle["EventInfo"] = event_info
    handle["AnalysisElectrons"] = electrons
    handle["AnalysisElectrons.pt"] = electrons_pt
    handle["AnalysisElectrons.eta"] = electrons_eta
    handle.set_column_void("AnalysisElectrons.ptRank", e_pt_rank, False)
    handle.set_column_void("AnalysisElectrons.etaRank", e_eta_rank, False)
    handle["AnalysisMuons"] = muons
    handle["AnalysisMuons.pt"] = muons_pt
    handle["AnalysisMuons.eta"] = muons_eta
    handle.set_column_void("AnalysisMuons.ptRank", m_pt_rank, False)

    handle.call()

    assert list(e_pt_rank) == [2, 0]
    assert list(e_eta_rank) == [0, 2]
    assert list(m_pt_rank) == [1]


if __name__ == "__main__":
    _run_tests(sys.modules[__name__])
