# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTestPixelLayer package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def InDetTestPixelLayerToolCfg(flags, name="InDetTestPixelLayerTool", **kwargs):
    if flags.Detector.GeometryITk:
        name = name.replace("InDet", "ITk")
        return ITkTestPixelLayerToolCfg(flags, name, **kwargs)

    result = ComponentAccumulator()

    if 'PixelSummaryTool' not in kwargs:
        from PixelConditionsTools.PixelConditionsSummaryConfig import (
            PixelConditionsSummaryCfg)
        kwargs.setdefault("PixelSummaryTool", result.popToolsAndMerge(
            PixelConditionsSummaryCfg(flags)))

    if "PixelDetElStatus" not in kwargs and not flags.Common.isOnline:
        from PixelReadoutGeometry.PixelReadoutGeometryConfig import (
            PixelReadoutManagerCfg)
        result.merge(PixelReadoutManagerCfg(flags))
        from PixelConditionsAlgorithms.PixelConditionsConfig import (
            PixelDetectorElementStatusAlgCfg)
        result.merge(PixelDetectorElementStatusAlgCfg(flags))
        kwargs.setdefault("PixelDetElStatus", "PixelDetectorElementStatus")

    if 'Extrapolator' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    kwargs.setdefault("CheckActiveAreas", flags.InDet.checkDeadElementsOnTrack)
    kwargs.setdefault("CheckDeadRegions", flags.InDet.checkDeadElementsOnTrack)
    kwargs.setdefault("CheckDisabledFEs", flags.InDet.checkDeadElementsOnTrack)

    result.setPrivateTools(
        CompFactory.InDet.InDetTestPixelLayerTool(name, **kwargs))
    return result


def InDetTestPixelLayerToolInnerCfg(
        flags, name="InDetTestPixelLayerToolInner", **kwargs):
    if flags.Detector.GeometryITk:
        name = name.replace("InDet", "ITk")
        return ITkTestPixelLayerToolInnerCfg(flags, name, **kwargs)

    kwargs.setdefault("CheckActiveAreas", False)
    kwargs.setdefault("CheckDeadRegions", False)
    kwargs.setdefault("CheckDisabledFEs", False)
    # To allow for extrapolation up to B-layer = next-to-innermost
    kwargs.setdefault("OuterRadius", 100.)
    return InDetTestPixelLayerToolCfg(flags, name, **kwargs)


def CombinedMuonPixelLayerToolCfg(
        flags, name="CombinedMuonPixelLayerToolDefault", **kwargs):
    result = ComponentAccumulator()

    if 'Extrapolator' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    result.setPrivateTools(result.popToolsAndMerge(
        InDetTestPixelLayerToolCfg(flags, name, **kwargs)))
    return result


def InDetTrigTestPixelLayerToolCfg(
        flags, name="InDetTrigTestPixelLayerTool", **kwargs):
    kwargs.setdefault("CheckActiveAreas", True)
    kwargs.setdefault("CheckDeadRegions", True)
    kwargs.setdefault("PixelDetElStatus", "")
    return InDetTestPixelLayerToolCfg(flags, name, **kwargs)


def InDetTrigTestPixelLayerToolInnerCfg(
        flags, name="InDetTrigTestPixelLayerToolInner", **kwargs):
    kwargs.setdefault("CheckActiveAreas", True)
    kwargs.setdefault("PixelDetElStatus", "")
    return InDetTestPixelLayerToolInnerCfg(flags, name, **kwargs)


def CombinedMuonTrigPixelLayerToolCfg(
        flags, name="CombinedMuonPixelLayerToolDefault", **kwargs):
    result = ComponentAccumulator()

    if 'Extrapolator' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    result.setPrivateTools(result.popToolsAndMerge(
        InDetTrigTestPixelLayerToolCfg(flags, name, **kwargs)))
    return result


def ITkTestPixelLayerToolCfg(flags, name="ITkTestPixelLayerTool", **kwargs):
    result = ComponentAccumulator()
    if 'PixelSummaryTool' not in kwargs:
        from PixelConditionsTools.ITkPixelConditionsSummaryConfig import (
            ITkPixelConditionsSummaryCfg)
        kwargs.setdefault("PixelSummaryTool", result.popToolsAndMerge(
            ITkPixelConditionsSummaryCfg(flags)))

    if 'Extrapolator' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    kwargs.setdefault("CheckActiveAreas", flags.ITk.checkDeadPixelsOnTrack)
    kwargs.setdefault("CheckDeadRegions", flags.ITk.checkDeadPixelsOnTrack)
    kwargs.setdefault("CheckDisabledFEs", flags.ITk.checkDeadPixelsOnTrack)
    # Outer pixel detector radius, to allow for extrapolation up to outermost
    # layer if needed
    kwargs.setdefault("OuterRadius", 350.)

    result.setPrivateTools(
        CompFactory.InDet.InDetTestPixelLayerTool(name, **kwargs))
    return result


def ITkTestPixelLayerToolInnerCfg(
        flags, name="ITkTestPixelLayerInnerTool", **kwargs):
    kwargs.setdefault("CheckActiveAreas", False)
    kwargs.setdefault("CheckDeadRegions", False)
    kwargs.setdefault("CheckDisabledFEs", False)
    # To allow for extrapolation up to next-to-innermost layer
    kwargs.setdefault("OuterRadius", 130.)
    return ITkTestPixelLayerToolCfg(flags, name, **kwargs)
