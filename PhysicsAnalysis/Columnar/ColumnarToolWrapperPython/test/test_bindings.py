# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Matthew Feickert
# @author Giordon Stark

# This follows the code Nils wrote in the ColumnarTests at:
# https://gitlab.cern.ch/atlas-asg/columnar-athena/-/blob/84feea5559c07a6a67233ab5465f90fb6f862509/PhysicsAnalysis/Columnar/ColumnarTests/test/gt_fullTools.cxx

import os
import sys
sys.path.insert(0, os.path.dirname(__file__))

from testutils import _run_tests, approx, xfail

import numpy as np

from ColumnarToolWrapperPython import PythonToolHandle


def test_properties():
    muon_eff_sf_tool_handle = PythonToolHandle()

    muon_eff_sf_tool_handle.set_type_and_name("CP::MuonEfficiencyScaleFactors/unique0")
    assert muon_eff_sf_tool_handle.type == "CP::MuonEfficiencyScaleFactors"
    assert muon_eff_sf_tool_handle.name == "unique0"


def test_accessors():
    muon_eff_sf_tool_handle = PythonToolHandle()

    muon_eff_sf_tool_handle.set_type_and_name("CP::MuonEfficiencyScaleFactors/unique0")
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

    muon_eff_sf_tool_handle.set_type_and_name("CP::MuonEfficiencyScaleFactors/unique0")
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

    assert columns["Muons.sfOut"] == approx(0.99509060382843018)
    assert columns["Muons.validOut"] == 1


def test_dict():
    """dict(handle) reflects column state: empty before set, populated after set, empty after call()."""
    handle = PythonToolHandle()
    handle.set_type_and_name("CP::MuonEfficiencyScaleFactors/unique0")
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
    muon_calib_tool_handle.set_type_and_name("CP::MuonCalibTool/unique0")

    muon_calib_tool_handle.set_property("IsRun3Geo", False)
    muon_calib_tool_handle.set_property("calibMode", 0)
    muon_calib_tool_handle.set_property("ExcludeNSWFromPrecisionLayers", False)
    muon_calib_tool_handle.initialize()

    assert muon_calib_tool_handle

    egamma_calib_tool_handle = PythonToolHandle()

    egamma_calib_tool_handle.set_type_and_name(
        "CP::EgammaCalibrationAndSmearingTool/unique1"
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
        "CP::EgammaCalibrationAndSmearingTool/unique2"
    )
    egamma_calib_tool_handle.ESModel = "es2022_R22_PRE"
    egamma_calib_tool_handle.decorrelationModel = "1NP_v1"
    egamma_calib_tool_handle.useFastSim = 0
    egamma_calib_tool_handle.useMVACalibration = 0
    egamma_calib_tool_handle.useLayerCorrection = 0
    egamma_calib_tool_handle.onlyElectrons = 1
    egamma_calib_tool_handle.initialize()

    assert egamma_calib_tool_handle


@xfail(reason="ElementLinks not yet supported")
def test_muon_calib_tool():
    muon_calib_tool_handle = PythonToolHandle()

    muon_calib_tool_handle.set_type_and_name("CP::MuonCalibTool/unique0")
    muon_calib_tool_handle.set_property("IsRun3Geo", False)
    muon_calib_tool_handle.set_property("calibMode", 0)
    muon_calib_tool_handle.set_property("ExcludeNSWFromPrecisionLayers", False)
    muon_calib_tool_handle.initialize()

    columns = {
        "EventInfo": np.array([0, 1], dtype=np.uint64),
        "EventInfo.runNumber": np.array([284500], dtype=np.uint32),
        "EventInfo.RandomRunNumber": np.array([284500], dtype=np.uint32),
        # columnMap.addColumn ("EventInfo.eventTypeBitmask", {unsigned(xAOD::EventInfo::IS_SIMULATION)}); ?
        "EventInfo.eventTypeBitmask": np.array([1], dtype=np.uint32),
        "Muons": np.array([0, 1], dtype=np.uint64),
        "Muons.pt": np.array([10e5], dtype=np.float32),
        "Muons.eta": np.array([1], dtype=np.float32),
        "Muons.phi": np.array([1], dtype=np.float32),
        "Muons.charge": np.array([1], dtype=np.float32),
        "Muons.muonType": np.array([0], dtype=np.uint16),
        # columnMap.addColumn ("Muons.author", {unsigned(xAOD::Muon::Author::CaloTag)});
        # columnMap.addColumn ("Muons.resolutionCategory", {unsigned(CP::IMuonSelectionTool::ResolutionCategory::highPt2station)});
        #
        # FIXME: Need to be able to handle ElementLinks
        # RuntimeError: invalid type for column: Muons.extrapolatedMuonSpectrometerTrackParticleLink
        "Muons.extrapolatedMuonSpectrometerTrackParticleLink": np.array(
            [0], dtype=np.uint16
        ),
        "Muons.combinedTrackParticleLink": np.array([0], dtype=np.uint16),
        "Muons.inDetTrackParticleLink": np.array([0], dtype=np.uint16),
        #
        # InDetTrackParticles
        # CombinedMuonTrackParticles
        # ExtrapolatedMuonTrackParticles
        #
        "Muons.ptOut": np.array([0], dtype=np.float32),
        "Muons.chargeOut": np.array([0], dtype=np.float32),
        "Muons.InnerDetectorPt": np.array([0], dtype=np.float32),
        "Muons.InnerDetectorCharge": np.array([0], dtype=np.float32),
        "Muons.MuonSpectrometerPt": np.array([0], dtype=np.float32),
        "Muons.MuonSpectrometerCharge": np.array([0], dtype=np.float32),
    }

    for name, value in columns.items():
        muon_calib_tool_handle.set_column(name, value)

    muon_calib_tool_handle.call()


if __name__ == "__main__":
    _run_tests(sys.modules[__name__])
