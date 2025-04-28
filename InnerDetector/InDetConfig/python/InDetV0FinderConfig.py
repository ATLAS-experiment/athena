# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetV0FinderToolCfg(flags, name="InDetV0FinderTool", **kwargs):
    
    acc = ComponentAccumulator()

    if "V0Tools" not in kwargs:
        from TrkConfig.TrkVertexAnalysisUtilsConfig import V0ToolsCfg
        kwargs.setdefault("V0Tools",  acc.popToolsAndMerge(
            V0ToolsCfg(flags, name+"_V0Tools")))
    acc.addPublicTool(kwargs["V0Tools"])

    if "VertexPointEstimator" not in kwargs:
        from InDetConfig.InDetConversionFinderToolsConfig import (
            V0VertexPointEstimatorCfg)
        kwargs.setdefault("VertexPointEstimator", acc.popToolsAndMerge(
            V0VertexPointEstimatorCfg(flags)))
    acc.addPublicTool(kwargs["VertexPointEstimator"])

    if "VertexFitterTool" not in kwargs:
        from TrkConfig.TrkV0FitterConfig import TrkV0VertexFitter_InDetExtrCfg
        kwargs.setdefault("VertexFitterTool",  acc.popToolsAndMerge(
            TrkV0VertexFitter_InDetExtrCfg(flags)))
    acc.addPublicTool(kwargs["VertexFitterTool"])

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))
    acc.addPublicTool(kwargs["Extrapolator"])

    if "TrackToVertexTool" not in kwargs:
        from TrackToVertex.TrackToVertexConfig import InDetTrackToVertexCfg
        kwargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(
            InDetTrackToVertexCfg(flags)))
    acc.addPublicTool(kwargs["TrackToVertexTool"])

    if "TrackSelectorTool" not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import (
            V0InDetConversionTrackSelectorToolCfg)
        kwargs.setdefault("TrackSelectorTool", acc.popToolsAndMerge(
            V0InDetConversionTrackSelectorToolCfg(flags)))
    acc.addPublicTool(kwargs["TrackSelectorTool"])

    kwargs.setdefault("useV0Fitter",      True)
    kwargs.setdefault("doSimpleV0", False)
    kwargs.setdefault("useVertexCollection", True)

    from TrkConfig.TrkVKalVrtFitterConfig import V0VKalVrtFitterCfg

    if "VKVertexFitterTool" not in kwargs:
        kwargs.setdefault("VKVertexFitterTool", acc.popToolsAndMerge(
            V0VKalVrtFitterCfg(flags, "_BPhysVKVFitter")))
    if "KshortFitterTool" not in kwargs:
        kwargs.setdefault("KshortFitterTool", acc.popToolsAndMerge(
            V0VKalVrtFitterCfg(flags, "_BPhysVKKVFitter",
                               InputParticleMasses = [139.57,139.57],
                               MassForConstraint   = 497.672)))
    if "LambdaFitterTool" not in kwargs:
        kwargs.setdefault("LambdaFitterTool", acc.popToolsAndMerge(
            V0VKalVrtFitterCfg(flags, "_BPhysVKLFitter",
                               InputParticleMasses = [938.272,139.57],
                               MassForConstraint   = 1115.68)))
    if "LambdabarFitterTool" not in kwargs:
        kwargs.setdefault("LambdabarFitterTool", acc.popToolsAndMerge(
            V0VKalVrtFitterCfg(flags, "_BPhysVKLbFitter",
                               InputParticleMasses = [139.57,938.272],
                               MassForConstraint   = 1115.68)))
    if "GammaFitterTool" not in kwargs:
        kwargs.setdefault("GammaFitterTool", acc.popToolsAndMerge(
            V0VKalVrtFitterCfg(flags, "_BPhysVKGFitter",
                               Robustness          = 6,
                               usePhiCnst          = True,
                               useThetaCnst        = True,
                               InputParticleMasses = [0.511,0.511])))

    from AthenaServices.PartPropSvcConfig import PartPropSvcCfg
    kwargs.setdefault("PartPropSvc", acc.getPrimaryAndMerge(PartPropSvcCfg(flags)))
    acc.setPrivateTools(CompFactory.InDet.InDetV0FinderTool(name, **kwargs))
    return acc

def IDTR2_V0FinderToolCfg(flags, name="IDTR2_V0FinderTool", **kwargs):
    acc = ComponentAccumulator()

    if "TrackSelectorTool" not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import (
            InDetV0VxTrackSelectorLooseCfg)
        TrackSelectorTool = acc.popToolsAndMerge(
            InDetV0VxTrackSelectorLooseCfg(flags))
        acc.addPublicTool(TrackSelectorTool)
        kwargs.setdefault("TrackSelectorTool", TrackSelectorTool)

    acc.setPrivateTools(
        acc.popToolsAndMerge(InDetV0FinderToolCfg(flags, name, **kwargs)))

    return acc

def V0MainDecoratorCfg(flags, name="V0Decorator", **kwargs):
    acc = ComponentAccumulator()

    if "V0Tools" not in kwargs:
        from TrkConfig.TrkVertexAnalysisUtilsConfig import V0ToolsCfg
        kwargs.setdefault("V0Tools",  acc.popToolsAndMerge(
            V0ToolsCfg(flags, name+"_V0Tools")))

    if "masses" not in kwargs or kwargs("masses") == 1:
        from AthenaServices.PartPropSvcConfig import PartPropSvcCfg
        kwargs.setdefault("PartPropSvc", acc.getPrimaryAndMerge(PartPropSvcCfg(flags)))
    acc.setPrivateTools(CompFactory.InDet.V0MainDecorator(name, **kwargs))
    return acc

def InDetV0FinderCfg(flags, name="InDetV0Finder",
                     V0ContainerName="V0UnconstrVertices",
                     KshortContainerName="V0KshortVertices",
                     LambdaContainerName="V0LambdaVertices",
                     LambdabarContainerName="V0LambdabarVertices",
                     **kwargs):
    acc = ComponentAccumulator()

    if "InDetV0FinderToolName" not in kwargs:
        from InDetConfig.InDetV0FinderConfig import InDetV0FinderToolCfg
        kwargs.setdefault("InDetV0FinderToolName", acc.popToolsAndMerge(
            InDetV0FinderToolCfg(
                flags,
                V0ContainerName = V0ContainerName,
                KshortContainerName = KshortContainerName,
                LambdaContainerName = LambdaContainerName,
                LambdabarContainerName = LambdabarContainerName)))

    if "Decorator" not in kwargs:
        from InDetConfig.InDetV0FinderConfig import V0MainDecoratorCfg
        kwargs.setdefault("Decorator", acc.popToolsAndMerge(
            V0MainDecoratorCfg(
                flags,
                V0ContainerName = V0ContainerName,
                KshortContainerName = KshortContainerName,
                LambdaContainerName = LambdaContainerName,
                LambdabarContainerName = LambdabarContainerName)))

    acc.addEventAlgo(CompFactory.InDet.InDetV0Finder(
        name,
        V0ContainerName        = V0ContainerName,
        KshortContainerName    = KshortContainerName,
        LambdaContainerName    = LambdaContainerName,
        LambdabarContainerName = LambdabarContainerName,
        **kwargs))
    return acc
