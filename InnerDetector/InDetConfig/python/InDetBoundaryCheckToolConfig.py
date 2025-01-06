# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetBoundaryCheckTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def InDetBoundaryCheckToolCfg(flags, name='InDetBoundarySearchTool', **kwargs):
    result = ComponentAccumulator()

    if 'SctSummaryTool' not in kwargs:
        if flags.Detector.EnableSCT:
            from SCT_ConditionsTools.SCT_ConditionsToolsConfig import (
                SCT_ConditionsSummaryToolCfg)
            kwargs.setdefault("SctSummaryTool", result.popToolsAndMerge(
                SCT_ConditionsSummaryToolCfg(flags)))
        else:
            kwargs.setdefault("SctSummaryTool", None)

    if flags.Detector.EnableSCT:
        if "SCTDetElStatus" not in kwargs and not flags.Common.isOnline:
            from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConfig import  (
                SCT_DetectorElementStatusAlgCfg)
            result.merge(SCT_DetectorElementStatusAlgCfg(flags))
            kwargs.setdefault("SCTDetElStatus", "SCTDetectorElementStatus")

    if flags.Detector.EnablePixel:
        if 'PixelLayerTool' not in kwargs:
            from InDetConfig.InDetTestPixelLayerConfig import (
                InDetTestPixelLayerToolCfg)
            kwargs.setdefault("PixelLayerTool", result.popToolsAndMerge(
                InDetTestPixelLayerToolCfg(flags)))
    else:
        kwargs.setdefault("PixelLayerTool", "")

    kwargs.setdefault("UsePixel", flags.Detector.EnablePixel)
    kwargs.setdefault("UseSCT", flags.Detector.EnableSCT)
    kwargs.setdefault("CheckBadSCT", flags.InDet.checkDeadElementsOnTrack)

    result.setPrivateTools(
        CompFactory.InDet.InDetBoundaryCheckTool(name, **kwargs))
    return result


def CombinedMuonIDBoundaryCheckToolCfg(
        flags, name="CombinedMuonIDBoundaryCheckTool", **kwargs):
    result = ComponentAccumulator()

    if flags.Detector.EnablePixel:
        if 'PixelLayerTool' not in kwargs:
            from InDetConfig.InDetTestPixelLayerConfig import (
                CombinedMuonPixelLayerToolCfg)
            kwargs.setdefault("PixelLayerTool", result.popToolsAndMerge(
                CombinedMuonPixelLayerToolCfg(flags)))
    else:
        kwargs.setdefault("PixelLayerTool", "")

    result.setPrivateTools(result.popToolsAndMerge(
        InDetBoundaryCheckToolCfg(flags, name, **kwargs)))
    return result


def InDetTrigBoundaryCheckToolCfg(
        flags, name="InDetTrigBoundaryCheckTool", **kwargs):
    acc = ComponentAccumulator()
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import (
        SCT_ConditionsSummaryToolCfg)

    kwargs.setdefault("SCTDetElStatus", "")
    kwargs.setdefault("SctSummaryTool", acc.popToolsAndMerge(
        SCT_ConditionsSummaryToolCfg(flags,
                                     withFlaggedCondTool=False,
                                     withTdaqTool=False)))

    from InDetConfig.InDetTestPixelLayerConfig import (
        InDetTrigTestPixelLayerToolCfg)
    kwargs.setdefault("PixelLayerTool", acc.popToolsAndMerge(
        InDetTrigTestPixelLayerToolCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(
        InDetBoundaryCheckToolCfg(flags, name, **kwargs)))
    return acc


def CombinedMuonTrigIDBoundaryCheckToolCfg(
        flags, name="CombinedMuonIDBoundaryCheckTool", **kwargs):
    result = ComponentAccumulator()

    if flags.Detector.EnablePixel:
        if 'PixelLayerTool' not in kwargs:
            from InDetConfig.InDetTestPixelLayerConfig import (
                CombinedMuonTrigPixelLayerToolCfg)
            kwargs.setdefault("PixelLayerTool", result.popToolsAndMerge(
                CombinedMuonTrigPixelLayerToolCfg(flags)))
    else:
        kwargs.setdefault("PixelLayerTool", "")

    result.setPrivateTools(result.popToolsAndMerge(
        InDetTrigBoundaryCheckToolCfg(flags, name, **kwargs)))
    return result


def ITkBoundaryCheckToolCfg(flags, name='ITkBoundaryCheckTool', **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("SctSummaryTool", None) # SCTDetElStatus is used instead
    if "SCTDetElStatus" not in kwargs :
        if flags.Detector.EnableITkStrip :
            if not flags.Trigger.doHLT :
                from SCT_ConditionsAlgorithms.ITkStripConditionsAlgorithmsConfig import  (
                    ITkStripDetectorElementStatusAlgCfg)
                result.merge(ITkStripDetectorElementStatusAlgCfg(flags))
            kwargs.setdefault("SCTDetElStatus", "ITkStripDetectorElementStatus")
        else :
            kwargs.setdefault("SCTDetElStatus", "")

    if 'PixelLayerTool' not in kwargs:
        from InDetConfig.InDetTestPixelLayerConfig import (
            ITkTestPixelLayerToolCfg)
        kwargs.setdefault("PixelLayerTool", result.popToolsAndMerge(
            ITkTestPixelLayerToolCfg(flags)))

    kwargs.setdefault("UsePixel", flags.Detector.EnableITkPixel)
    kwargs.setdefault("UseSCT", flags.Detector.EnableITkStrip)
    kwargs.setdefault("CheckBadSCT", False)

    result.setPrivateTools(
        CompFactory.InDet.InDetBoundaryCheckTool(name, **kwargs))
    return result
