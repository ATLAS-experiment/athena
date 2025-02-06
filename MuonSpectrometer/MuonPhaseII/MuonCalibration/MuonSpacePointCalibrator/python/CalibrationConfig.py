# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonSpacePointCalibratorCfg(flags,name="MuonSpacePointCalibrator", **kwargs):
    result = ComponentAccumulator()
    if flags.Detector.GeometryMDT: 
        from MuonConfig.MuonCalibrationConfig import MdtCalibrationToolCfg
        kwargs.setdefault("MdtCalibrationTool", result.popToolsAndMerge(MdtCalibrationToolCfg(flags)))
    the_tool = CompFactory.MuonR4.SpacePointCalibrator(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result