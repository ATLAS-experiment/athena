"""Define a function to configure ITkPixelConditionsSummaryCfg

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import Format
from PixelConditionsAlgorithms.ITkPixelConditionsConfig import (
    ITkPixelDCSCondStateAlgCfg, ITkPixelDCSCondStatusAlgCfg, ITkPixelDeadMapCondAlgCfg
)

def ITkPixelConditionsSummaryCfg(flags, name="ITkPixelConditionsSummary", **kwargs):
    """Return configured ComponentAccumulator with tool for ITk Pixel Conditions"""
    acc = ITkPixelDCSCondStateAlgCfg(flags)
    acc.merge(ITkPixelDCSCondStatusAlgCfg(flags))
    acc.merge(ITkPixelDeadMapCondAlgCfg(flags))

    kwargs.setdefault("PixelDetEleCollKey", "ITkPixelDetectorElementCollection")
    kwargs.setdefault("PixelDCSStateCondData", "ITkPixelDCSStateCondData")
    kwargs.setdefault("PixelDCSStatusCondData", "ITkPixelDCSStatusCondData")
    kwargs.setdefault("PixelDeadMapCondData", "ITkPixelDeadMapCondData")
    kwargs.setdefault("UseByteStreamFEI4", not flags.Input.isMC)
    kwargs.setdefault("UseByteStreamFEI3", not flags.Input.isMC)
    kwargs.setdefault("UseByteStreamRD53", False) # Turned off until BS format is defined

    acc.setPrivateTools(CompFactory.PixelConditionsSummaryTool(name=name + "Tool", **kwargs))
    return acc


def ITkByteStreamErrorDetectorElementStatusToolCfg(flags, name = "ITkByteStreamErrorDetectorElementStatusTool",**kwargs) :
    # @TODO bytestream errors for ITk have not been defined yet.
    acc = ComponentAccumulator()
    kwargs.setdefault("PixelDetEleCollKey","ITkPixelDetectorElementCollection") # @TODO do we need the DetEleColl for the Acts chain ?
    if (not flags.Input.isMC and flags.Input.Format is Format.BS) or flags.Overlay.DataOverlay:
        kwargs.setdefault("PixelByteStreamErrs", "PixelByteStreamErrs")
        kwargs.setdefault("UseByteStreamFEI4",   True)
        kwargs.setdefault("UseByteStreamFEI3",   True)
        kwargs.setdefault("ActiveOnly",          False)
        acc.setPrivateTools( CompFactory.PixelByteStreamErrorDetectorElementStatusTool(name, **kwargs) )
    else :
        kwargs.pop("ActiveOnly",False)
        acc.setPrivateTools( CompFactory.PixelDetectorElementStatusCloneTool(name, **kwargs) )
    return acc
