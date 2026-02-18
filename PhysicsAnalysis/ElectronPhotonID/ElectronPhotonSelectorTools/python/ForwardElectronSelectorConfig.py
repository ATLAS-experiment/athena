# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""
ComponentAccumulator configuration for DNN-based forward electron
calibration and identification tools in Run 4 with ITk and HGTD.
Internal Notes:
    Calibration: https://cds.cern.ch/record/2922184
    ID:          https://cds.cern.ch/record/2922175
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def AsgForwardElectronCalibrationToolCfg(flags, name="AsgForwardElectronCalibrationTool", **kwargs):
    """
    Configure the DNN-based pT calibration tool for forward electrons.
    Three eta bins: [2.5,2.7], [2.7,3.2], [3.2,4.0]
    """
    acc = ComponentAccumulator()

    kwargs.setdefault("ModelFiles", [
        "ElectronPhotonSelectorTools/ForwardElectronCalibration/calibration_eta_2.5_2.7_DNN.json",
        "ElectronPhotonSelectorTools/ForwardElectronCalibration/calibration_eta_2.7_3.2_DNN.json",
        "ElectronPhotonSelectorTools/ForwardElectronCalibration/calibration_eta_3.2_4.0_DNN.json",
    ])
    kwargs.setdefault("pTMin", 10000.)   #  10 GeV in MeV
    kwargs.setdefault("pTMax", 255000.)  # 255 GeV in MeV

    acc.setPrivateTools(
        CompFactory.AsgForwardElectronCalibrationTool(name, **kwargs)
    )
    return acc


def AsgForwardElectronSelectorToolCfg(flags, name="AsgForwardElectronSelectorTool",
                                       workingPoint="Medium", **kwargs):
    """
    Configure the DNN-based ID selector tool for forward electrons.
    Three eta bins: [2.5,2.7], [2.7,3.2], [3.2,4.0]
    Available working points: Loose = 90% | Medium = 80% | Tight = 70%
    """
    acc = ComponentAccumulator()

    # Build the calibration tool as a private tool
    calibAcc = AsgForwardElectronCalibrationToolCfg(
        flags,
        name=name + "_CalibTool"
    )
    calibTool = acc.popToolsAndMerge(calibAcc)

    kwargs.setdefault("ModelFiles", [
        "ElectronPhotonSelectorTools/ForwardElectronID/id_eta_2.5_2.7_DNN.json",
        "ElectronPhotonSelectorTools/ForwardElectronID/id_eta_2.7_3.2_DNN.json",
        "ElectronPhotonSelectorTools/ForwardElectronID/id_eta_3.2_4.0_DNN.json",
    ])
    kwargs.setdefault("WorkingPoint",      workingPoint)
    kwargs.setdefault("CalibrationTool",   calibTool)

    acc.setPrivateTools(
        CompFactory.AsgForwardElectronSelectorTool(name, **kwargs)
    )
    return acc



def AsgForwardElectronLooseSelectorToolCfg(flags, name="AsgForwardElectronLooseSelectorTool", **kwargs):
    return AsgForwardElectronSelectorToolCfg(flags, name=name, workingPoint="Loose", **kwargs)

def AsgForwardElectronMediumSelectorToolCfg(flags, name="AsgForwardElectronMediumSelectorTool", **kwargs):
    return AsgForwardElectronSelectorToolCfg(flags, name=name, workingPoint="Medium", **kwargs)

def AsgForwardElectronTightSelectorToolCfg(flags, name="AsgForwardElectronTightSelectorTool", **kwargs):
    return AsgForwardElectronSelectorToolCfg(flags, name=name, workingPoint="Tight", **kwargs)