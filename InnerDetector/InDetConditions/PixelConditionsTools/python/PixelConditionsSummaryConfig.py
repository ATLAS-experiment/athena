"""Define a function to configure PixelConditionsSummaryCfg

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format, ProductionStep

from PixelConditionsAlgorithms.PixelConditionsConfig import (
    PixelDCSCondStateAlgCfg, PixelDCSCondStatusAlgCfg, PixelDeadMapCondAlgCfg
)

@AccumulatorCache
def PixelConditionsSummaryCfg(flags, name="PixelConditionsSummary", **kwargs):
    """Return configured ComponentAccumulator with tool for Pixel Conditions"""
    acc = PixelDCSCondStateAlgCfg(flags)
    acc.merge(PixelDCSCondStatusAlgCfg(flags))
    acc.merge(PixelDeadMapCondAlgCfg(flags))

    kwargs.setdefault("UseByteStreamFEI4", (not flags.Input.isMC and flags.Input.Format is Format.BS) or flags.Overlay.DataOverlay)
    kwargs.setdefault("UseByteStreamFEI3", (not flags.Input.isMC and flags.Input.Format is Format.BS) or flags.Overlay.DataOverlay)
    if (flags.Overlay.DataOverlay and
        flags.Common.ProductionStep is not ProductionStep.MinbiasPreprocessing and
        not flags.Overlay.ByteStream):
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, ["IDCInDetBSErrContainer#PixelByteStreamErrs"]))

    if flags.InDet.usePixelDCS:
        pixel_states_active =  [ 'READY', 'ON' ]     # 'UNKNOWN', 'TRANSITION', 'UNDEFINED', 'DISABLED', 'LOCKED_OUT', 'OFF' states should be masked.

        kwargs.setdefault("IsActiveStates", pixel_states_active)
        kwargs.setdefault("IsActiveStatus", [ 'OK', 'WARNING', 'ERROR', 'FATAL' ])

    from PixelConditionsAlgorithms.PixelConditionsConfig import PixelDetectorElementCondAlgCfg
    acc.merge(PixelDetectorElementCondAlgCfg(flags))
    kwargs.setdefault( "PixelDetEleCollKey", "PixelDetectorElementCollection")

    acc.setPrivateTools(CompFactory.PixelConditionsSummaryTool(name=name + "Tool", **kwargs))
    return acc

def PixelConditionsSummaryToolNoByteStreamErrorsActiveOnlyCfg(flags,name = "PixelConditionsSummaryToolNoByteStreamErrorsAcriveOnly", **kwargs) :
    kwargs.setdefault("ActiveOnly",        True)
    kwargs.setdefault("UseByteStreamFEI4", False)
    kwargs.setdefault("UseByteStreamFEI3", False)
    kwargs.setdefault("UseByteStreamRD53", False)
    return PixelConditionsSummaryCfg(flags, name, **kwargs)

def PixelActiveDetectorElementStatusToolCfg(flags,name = "PixelActiveDetectorElementStatusTool",**kwargs) :
    acc = ComponentAccumulator()
    kwargs.setdefault("IsActiveStatus", [ 'OK', 'WARNING', 'ERROR', 'FATAL' ] )
    acc.setPrivateTools( CompFactory.PixelActiveDetectorElementStatusTool(name, **kwargs) )
    return acc

def PixelByteStreamErrorDetectorElementStatusToolCfg(flags, name = "PixelByteStreamErrorDetectorElementStatusTool",**kwargs) :
    acc = ComponentAccumulator()
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

def PixelByteStreamErrorDetectorElementStatusToolActiveOnlyCfg(flags, name = "PixelByteStreamErrorDetectorElementStatusToolActiveOnly",**kwargs) :
    kwargs.setdefault("ActiveOnly", True)
    return PixelByteStreamErrorDetectorElementStatusToolCfg(flags, name, **kwargs )
