# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonSpacePointCalibratorCfg(flags,name="MuonSpacePointCalibrator",**kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MdtPropagationTimeUncert", False)
    if flags.Detector.GeometryMDT: 
        from MuonConfig.MuonCalibrationConfig import MdtCalibrationToolCfg
        kwargs.setdefault("MdtCalibrationTool", result.popToolsAndMerge(MdtCalibrationToolCfg(flags,DoPropagationTimeUncert=kwargs["MdtPropagationTimeUncert"])))
    the_tool = CompFactory.MuonR4.SpacePointCalibrator(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result