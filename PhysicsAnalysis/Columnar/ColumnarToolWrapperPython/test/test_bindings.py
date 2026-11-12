# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Matthew Feickert
# @author Giordon Stark

# This follows the code Nils wrote in the ColumnarTests at:
# https://gitlab.cern.ch/atlas-asg/columnar-athena/-/blob/84feea5559c07a6a67233ab5465f90fb6f862509/PhysicsAnalysis/Columnar/ColumnarTests/test/gt_fullTools.cxx

import os
import sys
sys.path.insert(0, os.path.dirname(__file__))

from testutils import _run_tests, approx, MUON_CALIBRATION_RELEASE

import numpy as np

from ColumnarToolWrapperPython import PythonToolHandle, sg_key

# CLID of xAOD::TrackParticleContainer (from its CLASS_DEF)
CLID_TRACK_PARTICLE_CONTAINER = 1287425431


def test_properties():
    muon_eff_sf_tool_handle = PythonToolHandle()

    muon_eff_sf_tool_handle.set_type_and_name("CP_array::MuonEfficiencyScaleFactors/unique0")
    assert muon_eff_sf_tool_handle.type == "CP_array::MuonEfficiencyScaleFactors"
    assert muon_eff_sf_tool_handle.name == "unique0"


def test_accessors():
    muon_eff_sf_tool_handle = PythonToolHandle()

    muon_eff_sf_tool_handle.set_type_and_name("CP_array::MuonEfficiencyScaleFactors/unique0")
    muon_eff_sf_tool_handle.initialize()

    assert sorted([column.name for column in muon_eff_sf_tool_handle.columns]) == [
        "EventInfo",
        "EventInfo.RandomRunNumber",
        "EventInfo.eventTypeBitmask",
        "EventInfo.runNumber",
        "Muons",
        "Muons.eta",
        "Muons.isLRT",
        "Muons.muonType",
        "Muons.phi",
        "Muons.pt",
        "Muons.sfOut",
        "Muons.validOut",
    ]
    assert muon_eff_sf_tool_handle.get_recommended_systematics() == [
        "",
        "MUON_EFF_RECO_STAT__1down",
        "MUON_EFF_RECO_STAT__1up",
        "MUON_EFF_RECO_STAT_LOWPT__1down",
        "MUON_EFF_RECO_STAT_LOWPT__1up",
        "MUON_EFF_RECO_SYS__1down",
        "MUON_EFF_RECO_SYS__1up",
        "MUON_EFF_RECO_SYS_LOWPT__1down",
        "MUON_EFF_RECO_SYS_LOWPT__1up",
    ]


def test_call():
    muon_eff_sf_tool_handle = PythonToolHandle()

    muon_eff_sf_tool_handle.set_type_and_name("CP_array::MuonEfficiencyScaleFactors/unique0")
    muon_eff_sf_tool_handle.set_property("CalibrationRelease", MUON_CALIBRATION_RELEASE)
    muon_eff_sf_tool_handle.initialize()

    columns = {
        "EventInfo": np.array([0, 1], dtype=np.uint64),
        "EventInfo.runNumber": np.array([284500], dtype=np.uint32),
        "EventInfo.RandomRunNumber": np.array([284500], dtype=np.uint32),
        "EventInfo.eventTypeBitmask": np.array([1], dtype=np.uint32),
        "Muons": np.array([0, 1], dtype=np.uint64),
        "Muons.pt": np.array([10e5], dtype=np.float32),
        "Muons.eta": np.array([1], dtype=np.float32),
        "Muons.phi": np.array([1], dtype=np.float32),
        "Muons.muonType": np.array([0], dtype=np.uint16),
        "Muons.sfOut": np.array([0], dtype=np.float32),
        "Muons.validOut": np.array([0], dtype=np.int8),
    }

    for name, value in columns.items():
        muon_eff_sf_tool_handle.set_column(name, value)

    assert columns["Muons.sfOut"] == 0
    assert columns["Muons.validOut"] == 0

    muon_eff_sf_tool_handle.call()

    assert columns["Muons.sfOut"] == approx(0.99569094181060791)
    assert columns["Muons.validOut"] == 1


def test_dict():
    """dict(handle) reflects column state: empty before set, populated after set, empty after call()."""
    handle = PythonToolHandle()
    handle.set_type_and_name("CP_array::MuonEfficiencyScaleFactors/unique0")
    handle.initialize()

    # Before setting any columns: keys match tool columns, all values are empty
    result = dict(handle)
    assert set(result.keys()) == {col.name for col in handle.columns}
    assert all(len(v) == 0 for v in result.values())

    # After setting columns: set columns are populated, unset remain empty
    event_info = np.array([0, 1], dtype=np.uint64)
    event_run = np.array([284500], dtype=np.uint32)
    event_rand = np.array([284500], dtype=np.uint32)
    event_mask = np.array([1], dtype=np.uint32)
    muons = np.array([0, 1], dtype=np.uint64)
    muons_pt = np.array([10e5], dtype=np.float32)
    muons_eta = np.array([1], dtype=np.float32)
    muons_phi = np.array([1], dtype=np.float32)
    muons_type = np.array([0], dtype=np.uint16)
    muons_sf = np.array([0], dtype=np.float32)
    muons_valid = np.array([0], dtype=np.int8)
    handle["EventInfo"] = event_info
    handle["EventInfo.runNumber"] = event_run
    handle["EventInfo.RandomRunNumber"] = event_rand
    handle["EventInfo.eventTypeBitmask"] = event_mask
    handle["Muons"] = muons
    handle["Muons.pt"] = muons_pt
    handle["Muons.eta"] = muons_eta
    handle["Muons.phi"] = muons_phi
    handle["Muons.muonType"] = muons_type
    handle.set_column_void("Muons.sfOut", muons_sf, False)
    handle.set_column_void("Muons.validOut", muons_valid, False)

    result = dict(handle)
    assert len(result["Muons.pt"]) == 1
    assert result["Muons.pt"][0] == approx(10e5)
    assert len(result["Muons.isLRT"]) == 0  # optional, not set

    # After call(): all columns reset to empty
    handle.call()
    result = dict(handle)
    assert all(len(v) == 0 for v in result.values())


def test_set_property():
    muon_calib_tool_handle = PythonToolHandle()
    muon_calib_tool_handle.set_type_and_name("CP_array::MuonCalibTool/unique0")

    muon_calib_tool_handle.set_property("IsRun3Geo", False)
    muon_calib_tool_handle.set_property("calibMode", 0)
    muon_calib_tool_handle.set_property("ExcludeNSWFromPrecisionLayers", False)
    muon_calib_tool_handle.initialize()

    assert muon_calib_tool_handle

    egamma_calib_tool_handle = PythonToolHandle()

    egamma_calib_tool_handle.set_type_and_name(
        "CP_array::EgammaCalibrationAndSmearingTool/unique1"
    )
    egamma_calib_tool_handle.set_property("ESModel", "es2022_R22_PRE")
    egamma_calib_tool_handle.set_property("decorrelationModel", "1NP_v1")
    egamma_calib_tool_handle.set_property("useFastSim", 0)
    egamma_calib_tool_handle.set_property("useMVACalibration", 0)
    egamma_calib_tool_handle.set_property("useLayerCorrection", 0)
    egamma_calib_tool_handle.set_property("onlyElectrons", 1)
    egamma_calib_tool_handle.initialize()

    assert egamma_calib_tool_handle

    egamma_calib_tool_handle = PythonToolHandle()

    egamma_calib_tool_handle.set_type_and_name(
        "CP_array::EgammaCalibrationAndSmearingTool/unique2"
    )
    egamma_calib_tool_handle.ESModel = "es2022_R22_PRE"
    egamma_calib_tool_handle.decorrelationModel = "1NP_v1"
    egamma_calib_tool_handle.useFastSim = 0
    egamma_calib_tool_handle.useMVACalibration = 0
    egamma_calib_tool_handle.useLayerCorrection = 0
    egamma_calib_tool_handle.onlyElectrons = 1
    egamma_calib_tool_handle.initialize()

    assert egamma_calib_tool_handle


def test_muon_calib_tool():
    """MuonCalibTool with key+index links to the track particle containers.

    This is the python version of the MuonCalibTool memory test in
    MuonMomentumCorrections/test/gt_ColumnarToolTests.cxx.
    """
    handle = PythonToolHandle()
    handle.set_type_and_name("CP_array::MuonCalibTool/unique0")
    handle.set_property("IsRun3Geo", False)
    handle.set_property("calibMode", 0)
    handle.set_property("ExcludeNSWFromPrecisionLayers", False)
    handle.set_property("skipResolutionCategory", True)
    handle.initialize()

    def array(values, dtype):
        return np.array(values, dtype=dtype)

    # one event with one muon
    columns = {
        "EventInfo": array([0, 1], np.uint64),
        "EventInfo.runNumber": array([284500], np.uint32),
        "EventInfo.RandomRunNumber": array([284500], np.uint32),
        "EventInfo.eventNumber": array([1234], np.uint64),
        "EventInfo.eventTypeBitmask": array([1], np.uint32),  # IS_SIMULATION
        "Muons": array([0, 1], np.uint64),
        "Muons.eventIndex": array([0], np.uint64),
        "Muons.pt": array([10e5], np.float32),
        "Muons.eta": array([1], np.float32),
        "Muons.phi": array([1], np.float32),
        "Muons.charge": array([1], np.float32),
        "Muons.muonType": array([0], np.uint16),  # Combined
        "Muons.author": array([8], np.uint16),  # CaloTag
        "Muons.ptOut": array([0], np.float32),
        "Muons.chargeOut": array([0], np.float32),
        "Muons.InnerDetectorPt": array([0], np.float32),
        "Muons.InnerDetectorCharge": array([0], np.float32),
        "Muons.MuonSpectrometerPt": array([0], np.float32),
        "Muons.MuonSpectrometerCharge": array([0], np.float32),
    }

    # The links are stored as key and index columns. The key is calculated
    # from the name of the target container and the CLID of its type, the
    # index is relative to the start of the event in the target container.
    links = {
        "extrapolatedMuonSpectrometerTrackParticleLink": "ExtrapolatedMuonTrackParticles",
        "combinedTrackParticleLink": "CombinedMuonTrackParticles",
        "inDetTrackParticleLink": "InDetTrackParticles",
    }
    for link, target in links.items():
        columns[f"Muons.{link}.m_persKey"] = array([sg_key(target, CLID_TRACK_PARTICLE_CONTAINER)], np.uint32)
        columns[f"Muons.{link}.m_persIndex"] = array([0], np.uint32)

    # each track container has one track in the event
    for container in ["InDetTrackParticles", "InDetForwardTrackParticles",
                      "CombinedMuonTrackParticles", "ExtrapolatedMuonTrackParticles"]:
        columns[container] = array([0, 1], np.uint64)
        for parameter in ["d0", "z0", "phi", "theta", "qOverP"]:
            columns[f"{container}.{parameter}"] = array([0], np.float32)
        columns[f"{container}.definingParametersCovMatrixDiag.data"] = array([0] * 5, np.float32)
        columns[f"{container}.definingParametersCovMatrixDiag.offset"] = array([0, 5], np.uint64)
        columns[f"{container}.definingParametersCovMatrixOffDiag.data"] = array([0] * 6, np.float32)
        columns[f"{container}.definingParametersCovMatrixOffDiag.offset"] = array([0, 6], np.uint64)

    for name, value in columns.items():
        handle.set_column(name, value)

    handle.call()

    assert columns["Muons.ptOut"][0] == 0
    assert columns["Muons.chargeOut"][0] == 1
    assert columns["Muons.InnerDetectorPt"][0] == 0
    assert columns["Muons.InnerDetectorCharge"][0] == 1
    assert columns["Muons.MuonSpectrometerPt"][0] == 0
    assert columns["Muons.MuonSpectrometerCharge"][0] == 1


if __name__ == "__main__":
    _run_tests(sys.modules[__name__])
