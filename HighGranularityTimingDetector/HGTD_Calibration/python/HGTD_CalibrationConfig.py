"""Define calibration tools used on the digitization and reconstruction chains

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def HGTD_TdcCalibrationToolCfg(flags, name="HGTD_TdcCalibrationTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("PS_ActiveRange", 2.5)
    kwargs.setdefault("LHC_RiseEdge", 12.5)
    kwargs.setdefault("PS_LargeStep", 1.562)
    kwargs.setdefault("PS_SmallStep", 0.097)
    kwargs.setdefault("TOABinSize", 0.02)
    acc.setPrivateTools(CompFactory.HGTD_TdcCalibrationTool(name, **kwargs))
    return acc


def HGTD_TimeResolutionToolCfg(flags, name="HGTD_TimeResolutionTool", **kwargs):
    acc = ComponentAccumulator()

    acc.setPrivateTools(CompFactory.HGTD_TimeResolutionTool(name, **kwargs))
    return acc
