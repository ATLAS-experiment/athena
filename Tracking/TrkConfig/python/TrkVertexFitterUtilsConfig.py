# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVertexFitterUtils package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FullLinearizedTrackFactoryCfg(flags, name='FullLinearizedTrkFactory', **kwargs):
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = AtlasFieldCacheCondAlgCfg(flags) # To produce AtlasFieldCacheCondObj

    if 'Extrapolator' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    acc.setPrivateTools(
        CompFactory.Trk.FullLinearizedTrackFactory(name, **kwargs))
    return acc

def AtlasFullLinearizedTrackFactoryCfg(flags, name='AtlasFullLinearizedTrkFactory', **kwargs):
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = AtlasFieldCacheCondAlgCfg(flags) # To produce AtlasFieldCacheCondObj

    if 'Extrapolator' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    acc.setPrivateTools(
        CompFactory.Trk.FullLinearizedTrackFactory(name, **kwargs))
    return acc

def DetAnnealingMakerCfg(flags, name='DetAnnealingMaker', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("SetOfTemperatures", [ 64., 16., 4., 2., 1.5, 1. ])
    acc.setPrivateTools(CompFactory.Trk.DetAnnealingMaker(name, **kwargs))
    return acc

def SecVtxDetAnnealingMakerCfg(flags, name='SecVtxDetAnnealingMaker', **kwargs):
    kwargs.setdefault("SetOfTemperatures", [ 1.0 ])
    return DetAnnealingMakerCfg(flags, name, **kwargs)

def TauDetAnnealingMakerCfg(flags, name='tauRec_TauDetAnnealingMaker', **kwargs):
    kwargs.setdefault("SetOfTemperatures", [ 64, 32, 16, 8, 4, 2, 1 ])
    return DetAnnealingMakerCfg(flags, name, **kwargs)

def TrackToVertexIPEstimatorCfg(flags, name='TrackToVertexIPEstimator', **kwargs):
    acc = ComponentAccumulator()

    if 'Extrapolator' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    if 'LinearizedTrackFactory' not in kwargs:
        kwargs.setdefault("LinearizedTrackFactory", acc.popToolsAndMerge(
            FullLinearizedTrackFactoryCfg(flags)))

    acc.setPrivateTools(CompFactory.Trk.TrackToVertexIPEstimator(name, **kwargs))
    return acc

def AtlasTrackToVertexIPEstimatorCfg(flags, name='AtlasTrackToVertexIPEstimator', **kwargs):
    acc = ComponentAccumulator()

    if 'Extrapolator' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    if 'LinearizedTrackFactory' not in kwargs:
        kwargs.setdefault("LinearizedTrackFactory", acc.popToolsAndMerge(
            AtlasFullLinearizedTrackFactoryCfg(flags)))

    acc.setPrivateTools(CompFactory.Trk.TrackToVertexIPEstimator(name, **kwargs))
    return acc

def ImpactPoint3dEstimatorCfg(flags, name='ImpactPoint3dEstimator', **kwargs):
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = AtlasFieldCacheCondAlgCfg(flags) # To produce AtlasFieldCacheCondObj

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        Extrapolator = acc.popToolsAndMerge(InDetExtrapolatorCfg(flags))
        acc.addPublicTool(Extrapolator)
        kwargs.setdefault("Extrapolator", Extrapolator)

    acc.setPrivateTools(CompFactory.Trk.ImpactPoint3dEstimator(name, **kwargs))
    return acc

def AtlasImpactPoint3dEstimatorCfg(flags, name='AtlasImpactPoint3dEstimator', **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        Extrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
        acc.addPublicTool(Extrapolator)
        kwargs.setdefault("Extrapolator", Extrapolator)

    acc.setPrivateTools(acc.popToolsAndMerge(
        ImpactPoint3dEstimatorCfg(flags, name, **kwargs)))
    return acc

def KalmanVertexUpdatorCfg(flags, name='KalmanVertexUpdator', **kwargs):

    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.Trk.KalmanVertexUpdator(name, **kwargs))
    return acc
