"""Define a function to configure PLR_ConditionsSummaryCfg

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentFactory import CompFactory

from PixelConditionsAlgorithms.ITkPixelConditionsConfig import (
    ITkPixelDCSCondStateAlgCfg, ITkPixelDCSCondStatusAlgCfg, ITkPixelDeadMapCondAlgCfg
)
from PixelReadoutGeometry.PixelReadoutGeometryConfig import PLR_ReadoutManagerCfg

def PLR_ConditionsSummaryCfg(flags, name="PLR_ConditionsSummary", **kwargs):
    """Return configured ComponentAccumulator with tool for ITk Pixel Conditions"""
    acc = PLR_ReadoutManagerCfg(flags)
    acc.merge(ITkPixelDCSCondStateAlgCfg(flags))
    acc.merge(ITkPixelDCSCondStatusAlgCfg(flags))
    acc.merge(ITkPixelDeadMapCondAlgCfg(flags))

    kwargs.setdefault("PixelReadoutManager", acc.getPrimary())
    kwargs.setdefault("PixelDetEleCollKey", "PLR_DetectorElementCollection")
    kwargs.setdefault("PixelDCSStateCondData", "ITkPixelDCSStateCondData")
    kwargs.setdefault("PixelDCSStatusCondData", "ITkPixelDCSStatusCondData")
    kwargs.setdefault("PixelDeadMapCondData", "ITkPixelDeadMapCondData")
    kwargs.setdefault("UseByteStreamFEI4", not flags.Input.isMC)
    kwargs.setdefault("UseByteStreamFEI3", not flags.Input.isMC)
    kwargs.setdefault("UseByteStreamRD53", False) # Turned off until BS format is defined

    if flags.InDet.usePixelDCS:
        kwargs.setdefault("IsActiveStates", [ 'READY', 'ON', 'UNKNOWN', 'TRANSITION', 'UNDEFINED' ])
        kwargs.setdefault("IsActiveStatus", [ 'OK', 'WARNING', 'ERROR', 'FATAL' ])

    acc.setPrivateTools(CompFactory.PixelConditionsSummaryTool(name=name + "Tool", **kwargs))
    return acc
