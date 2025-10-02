# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Matthew Feickert

# This follows the code Nils wrote in the ColumnarTests at:
# https://gitlab.cern.ch/atlas-asg/columnar-athena/-/blob/84feea5559c07a6a67233ab5465f90fb6f862509/PhysicsAnalysis/Columnar/ColumnarTests/test/gt_fullTools.cxx
import python_tool_handle
import numpy as np
import pytest


def test_properties():
    muon_eff_sf_tool_handle = python_tool_handle.PythonToolHandle()

    muon_eff_sf_tool_handle.set_type_and_name("CP::MuonEfficiencyScaleFactors/unique0")
    assert muon_eff_sf_tool_handle.type == "CP::MuonEfficiencyScaleFactors"
    assert muon_eff_sf_tool_handle.name == "unique0"


def test_accessors():
    muon_eff_sf_tool_handle = python_tool_handle.PythonToolHandle()

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
    muon_eff_sf_tool_handle = python_tool_handle.PythonToolHandle()

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

    assert columns["Muons.sfOut"] == pytest.approx(0.99509060382843018)
    assert columns["Muons.validOut"] == 1


def test_set_property():
    muon_calib_tool_handle = python_tool_handle.PythonToolHandle()
    muon_calib_tool_handle.set_type_and_name("CP::MuonCalibTool/unique0")

    muon_calib_tool_handle.set_property("IsRun3Geo", False)
    muon_calib_tool_handle.set_property("calibMode", 0)
    muon_calib_tool_handle.set_property("ExcludeNSWFromPrecisionLayers", False)
    muon_calib_tool_handle.set_property("readResolutionCategory", True)
    muon_calib_tool_handle.initialize()

    assert muon_calib_tool_handle

    egamma_calib_tool_handle = python_tool_handle.PythonToolHandle()

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

    egamma_calib_tool_handle = python_tool_handle.PythonToolHandle()

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


@pytest.mark.xfail(reason="ElementLinks not yet supported")
def test_muon_calib_tool():
    muon_calib_tool_handle = python_tool_handle.PythonToolHandle()

    muon_calib_tool_handle.set_type_and_name("CP::MuonCalibTool/unique0")
    muon_calib_tool_handle.set_property("IsRun3Geo", False)
    muon_calib_tool_handle.set_property("calibMode", 0)
    muon_calib_tool_handle.set_property("ExcludeNSWFromPrecisionLayers", False)
    muon_calib_tool_handle.set_property("readResolutionCategory", True)
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
