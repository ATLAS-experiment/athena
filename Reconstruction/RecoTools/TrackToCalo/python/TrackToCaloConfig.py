# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

""" Tool configuration for the track to calo tools. """

# ---------------------------------------
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ParticleCaloExtensionToolCfg(flags,
                                 name='ParticleCaloExtensionTool',
                                 **kwargs):
    ''' PFO/Muon/Tau common configuration of the tool.
    Useful when targeting intersection through
    the full calorimeter and provide intersections
    with arbitrary layers. Results are typically "cached"
    and re-used'''
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs["Extrapolator"] = acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags))

    acc.setPrivateTools(
        CompFactory.Trk.ParticleCaloExtensionTool(name, **kwargs))
    return acc


def EMParticleCaloExtensionToolCfg(flags,
                                   name='EMParticleCaloExtensionTool',
                                   **kwargs):
    ''' e/gamma configuration of the tool.
    Useful when targeting a specific layers in
    "direct" extrapolation mode. It needs the
    CaloSurfaceBuilder.
    '''
    acc = ComponentAccumulator()
    kwargs.setdefault("StartFromPerigee", True)

    if "CaloSurfaceBuilder" not in kwargs:
        from CaloTrackingGeometry.CaloTrackingGeometryConfig import (
            CaloSurfaceBuilderMiddleCfg)
        kwargs["CaloSurfaceBuilder"] = acc.popToolsAndMerge(
            CaloSurfaceBuilderMiddleCfg(flags))

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import egammaCaloExtrapolatorCfg
        kwargs["Extrapolator"] = acc.popToolsAndMerge(
            egammaCaloExtrapolatorCfg(flags))

    acc.setPrivateTools(acc.popToolsAndMerge(
        ParticleCaloExtensionToolCfg(flags, **kwargs)))
    return acc


def HLTPF_ParticleCaloExtensionToolCfg(flags,
                                       name='HLTPF_ParticleCaloExtension',
                                       **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import TrigPFlowExtrapolatorCfg
        kwargs["Extrapolator"] = acc.popToolsAndMerge(
            TrigPFlowExtrapolatorCfg(flags))

    acc.setPrivateTools(
        CompFactory.Trk.ParticleCaloExtensionTool(name, **kwargs))
    return acc


def ParticleCaloCellAssociationToolCfg(flags,
                                       name='ParticleCaloCellAssociationTool',
                                       **kwargs):
    acc = ComponentAccumulator()

    if "ParticleCaloExtensionTool" not in kwargs:
        kwargs["ParticleCaloExtensionTool"] = acc.popToolsAndMerge(
            ParticleCaloExtensionToolCfg(flags))

    # should this be a more global flag?
    kwargs.setdefault("CaloCellContainer", flags.Egamma.Keys.Input.CaloCells)

    acc.setPrivateTools(
        CompFactory.Rec.ParticleCaloCellAssociationTool(name, **kwargs))
    return acc


def HLTPF_ParticleCaloCellAssociationToolCfg(flags,
                                             name='HLTPF_ParticleCaloCellAssociationTool',
                                             **kwargs):
    acc = ComponentAccumulator()

    if "ParticleCaloExtensionTool" not in kwargs:
        kwargs["ParticleCaloExtensionTool"] = acc.popToolsAndMerge(
            HLTPF_ParticleCaloExtensionToolCfg(flags))

    kwargs.setdefault("CaloCellContainer", "")

    acc.setPrivateTools(
        CompFactory.Rec.ParticleCaloCellAssociationTool(name, **kwargs))
    return acc
