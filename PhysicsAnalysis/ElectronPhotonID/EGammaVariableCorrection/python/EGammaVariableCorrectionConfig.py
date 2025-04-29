# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ElectronVariableCorrectionToolCfg(
        flags, name="ElectronVariableCorrectionTool", **kwargs):
    """Configure the e/gamma variable correction tool"""
    acc = ComponentAccumulator()
    # Can ultimately be configured differently between Run 2 and Run 3 configs
    kwargs.setdefault("ConfigFile", "EGammaVariableCorrection/TUNE27/ElVariableNominalCorrection.conf")
    acc.setPrivateTools(
        CompFactory.ElectronPhotonVariableCorrectionTool(name, **kwargs))
    return acc

def PhotonVariableCorrectionToolCfg(
        flags, name="PhotonVariableCorrectionTool", **kwargs):
    """Configure the e/gamma variable correction tool"""
    acc = ComponentAccumulator()
    # Use TUNE 25 for now for photons
    kwargs.setdefault("ConfigFile", "EGammaVariableCorrection/TUNE25/ElPhVariableNominalCorrection.conf")
    acc.setPrivateTools(
        CompFactory.ElectronPhotonVariableCorrectionTool(name, **kwargs))
    return acc


