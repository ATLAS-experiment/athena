# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ********************************************************************
# PixelNtupleConfig.py
# Configures PixelNtupleMaker
# Component accumulator version
# ********************************************************************

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PixelNtupleMakerCfg(flags, name="PixelMonitoringTool", **kwargs):
    acc= ComponentAccumulator()
    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        InDetTrackSelectionTool_Loose_Cfg)
    InDetTrackSelectionTool = acc.popToolsAndMerge(
        InDetTrackSelectionTool_Loose_Cfg(flags))
    acc.addPublicTool(InDetTrackSelectionTool, primary=False)
    kwargs["TrackSelectionTool"] = InDetTrackSelectionTool
    the_tool = CompFactory.DerivationFramework.PixelNtupleMaker(name,**kwargs)
    acc.addPublicTool(the_tool, primary = True)
    return acc

def EventInfoPixelModuleStatusMonitoringCfg(
        flags, name="EventInfoPixelModuleStatusMonitoring", **kwargs):
    from PixelConditionsAlgorithms.PixelConditionsConfig import (
        PixelDCSCondTempAlgCfg, PixelDCSCondHVAlgCfg,
        PixelDCSCondStateAlgCfg, PixelDCSCondStatusAlgCfg,
        PixelDeadMapCondAlgCfg, )
    acc = PixelDCSCondTempAlgCfg(flags)
    acc.merge(PixelDCSCondHVAlgCfg(flags))
    acc.merge(PixelDCSCondStateAlgCfg(flags))
    acc.merge(PixelDCSCondStatusAlgCfg(flags))
    acc.merge(PixelDeadMapCondAlgCfg(flags))

    if "PixelConditionsSummaryTool" not in kwargs:
        from PixelConditionsTools.PixelConditionsSummaryConfig import (
            PixelConditionsSummaryCfg)
        kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(
            PixelConditionsSummaryCfg(flags)))

    acc.addPublicTool(
        CompFactory.DerivationFramework.EventInfoPixelModuleStatusMonitoring(
            name,**kwargs), primary = True)
    return acc

def ITkEventInfoPixelModuleStatusMonitoringCfg(
        flags, name="ITkEventInfoPixelModuleStatusMonitoring", **kwargs):
    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import (
        ITkPixelDCSCondTempAlgCfg, ITkPixelDCSCondHVAlgCfg,
        ITkPixelDCSCondStateAlgCfg, ITkPixelDCSCondStatusAlgCfg,
        ITkPixelDeadMapCondAlgCfg)
    acc = ITkPixelDCSCondTempAlgCfg(flags)
    acc.merge(ITkPixelDCSCondHVAlgCfg(flags))
    acc.merge(ITkPixelDCSCondStateAlgCfg(flags))
    acc.merge(ITkPixelDCSCondStatusAlgCfg(flags))
    acc.merge(ITkPixelDeadMapCondAlgCfg(flags))

    kwargs.setdefault("ReadKeyeTemp", "ITkPixelDCSTempCondData")
    kwargs.setdefault("ReadKeyHV", "ITkPixelDCSHVCondData")
    kwargs.setdefault("PixelDCSStateCondData", "ITkPixelDCSStateCondData")
    kwargs.setdefault("PixelDCSStatusCondData", "ITkPixelDCSStatusCondData")
    kwargs.setdefault("PixelDeadMapCondData", "ITkPixelDeadMapCondData")

    # No PixelByteStreamErrs producer yet for ITk
    kwargs.setdefault("PixelByteStreamErrs", "")
    kwargs.setdefault("UseByteStreamRD53", False)

    kwargs.setdefault("UseByteStreamFEI4", False)
    kwargs.setdefault("UseByteStreamFEI3", False)

    if "PixelConditionsSummaryTool" not in kwargs:
        from PixelConditionsTools.ITkPixelConditionsSummaryConfig import (
            ITkPixelConditionsSummaryCfg)
        kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(
            ITkPixelConditionsSummaryCfg(flags)))

    acc.addPublicTool(
        CompFactory.DerivationFramework.EventInfoPixelModuleStatusMonitoring(
            name,**kwargs), primary = True)
    return acc

