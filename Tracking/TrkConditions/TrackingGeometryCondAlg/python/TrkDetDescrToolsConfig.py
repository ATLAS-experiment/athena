# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

def InDetTrackingVolumeHelperCfg(flags, name='InDetTrackingVolumeHelper',
                                 **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.Trk.TrackingVolumeHelper(name, **kwargs))
    return result

def ITkTrackingVolumeHelperCfg(flags, name='ITkTrackingVolumeHelper', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("BarrelLayerBinsZ",
                      flags.ITk.trackingGeometry.passiveBarrelMatZbins)
    kwargs.setdefault("BarrelLayerBinsPhi",
                      flags.ITk.trackingGeometry.passiveBarrelMatPhiBins)
    kwargs.setdefault("EndcapLayerBinsR",
                      flags.ITk.trackingGeometry.passiveEndcapMatRbins)
    kwargs.setdefault("EndcapLayerBinsPhi",
                      flags.ITk.trackingGeometry.passiveEndcapMatPhiBins)
    result.setPrivateTools(CompFactory.Trk.TrackingVolumeHelper(name, **kwargs))
    return result

# Generic interface for conveniency
def TrackingVolumeHelperCfg(flags, name='TrackingVolumeHelper', **kwargs):
    if flags.Detector.GeometryITk:
        return ITkTrackingVolumeHelperCfg(flags, name, **kwargs)
    else:
        return InDetTrackingVolumeHelperCfg(flags, name, **kwargs)

def HGTD_TrackingVolumeHelperCfg(flags, name='HGTD_TrackingVolumeHelper',
                                 **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("BarrelLayerBinsZ",
                      flags.HGTD.trackingGeometry.passiveBarrelMatZbins)
    kwargs.setdefault("BarrelLayerBinsPhi",
                      flags.HGTD.trackingGeometry.passiveBarrelMatPhiBins)
    kwargs.setdefault("EndcapLayerBinsR",
                      flags.HGTD.trackingGeometry.passiveEndcapMatRbins)
    kwargs.setdefault("EndcapLayerBinsPhi",
                      flags.HGTD.trackingGeometry.passiveEndcapMatPhiBins)
    result.setPrivateTools(CompFactory.Trk.TrackingVolumeHelper(name, **kwargs))
    return result

def TrackingVolumeArrayCreatorCfg(flags, name="TrackingVolumeArrayCreator",
                                  **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(
        CompFactory.Trk.TrackingVolumeArrayCreator(name, **kwargs))
    return result

@AccumulatorCache
def LayerArrayCreatorCfg(flags, name='LayerArrayCreator', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("EmptyLayerMode", 2) # deletes empty material layers from arrays
    result.setPrivateTools(CompFactory.Trk.LayerArrayCreator(name, **kwargs))
    return result

def CylinderVolumeCreatorCfg(flags, name='CylinderVolumeCreator',
                             **kwargs):
    result = ComponentAccumulator()

    if "LayerArrayCreator" not in kwargs:
        kwargs.setdefault("LayerArrayCreator", result.popToolsAndMerge(
            LayerArrayCreatorCfg(flags)))

    if "TrackingVolumeArrayCreator" not in kwargs:
        kwargs.setdefault("TrackingVolumeArrayCreator", result.popToolsAndMerge(
            TrackingVolumeArrayCreatorCfg(flags)))

    result.setPrivateTools(
        CompFactory.Trk.CylinderVolumeCreator(name, **kwargs))
    return result

def InDetCylinderVolumeCreatorCfg(flags, name='InDetCylinderVolumeCreator',
                                  **kwargs):
    result = ComponentAccumulator()

    if "TrackingVolumeHelper" not in kwargs:
        kwargs.setdefault("TrackingVolumeHelper", result.popToolsAndMerge(
            InDetTrackingVolumeHelperCfg(flags)))

    kwargs.setdefault("PassiveLayerBinsRZ", 1)

    result.setPrivateTools(result.popToolsAndMerge(
        CylinderVolumeCreatorCfg(flags, name, **kwargs)))
    return result

def ITkCylinderVolumeCreatorCfg(flags, name='ITkCylinderVolumeCreator',
                                **kwargs):
    result = ComponentAccumulator()

    if "TrackingVolumeHelper" not in kwargs:
        kwargs.setdefault("TrackingVolumeHelper", result.popToolsAndMerge(
            ITkTrackingVolumeHelperCfg(flags)))

    kwargs.setdefault("PassiveLayerThickness", 1.) # in mm
    kwargs.setdefault("PassiveLayerBinsRZ",
                      flags.ITk.trackingGeometry.passiveBarrelMatZbins)
    kwargs.setdefault("PassiveLayerBinsPhi",
                      flags.ITk.trackingGeometry.passiveBarrelMatPhiBins)

    result.setPrivateTools(result.popToolsAndMerge(
        CylinderVolumeCreatorCfg(flags, name, **kwargs)))
    return result

def HGTD_CylinderVolumeCreatorCfg(flags, name='HGTD_CylinderVolumeCreator',
                                **kwargs):
    result = ComponentAccumulator()

    if "TrackingVolumeHelper" not in kwargs:
        kwargs.setdefault("TrackingVolumeHelper", result.popToolsAndMerge(
            HGTD_TrackingVolumeHelperCfg(flags)))

    kwargs.setdefault("PassiveLayerBinsRZ",
                      flags.HGTD.trackingGeometry.passiveBarrelMatZbins)
    kwargs.setdefault("PassiveLayerBinsPhi",
                      flags.HGTD.trackingGeometry.passiveBarrelMatPhiBins)

    result.setPrivateTools(result.popToolsAndMerge(
        CylinderVolumeCreatorCfg(flags, name, **kwargs)))
    return result

def ITkBeamPipeProviderCfg(flags, name='ITkBeamPipeProvider',
                           useCond=True,
                           **kwargs):
    result = ComponentAccumulator()

    if "LayerBuilder" not in kwargs:
        from TrackingGeometryCondAlg.InDetTrackingGeometryConfig import (
            ITkBeamPipeBuilderCfg)
        beamPipeBuilder = result.popToolsAndMerge(
            ITkBeamPipeBuilderCfg(flags, useCond = useCond))
        result.addPublicTool(beamPipeBuilder)
        kwargs.setdefault("LayerBuilder", beamPipeBuilder)

    layerProvider = CompFactory.Trk.LayerProviderCond(name, **kwargs) \
                    if useCond else \
                       CompFactory.Trk.LayerProvider(name, **kwargs)
    result.setPrivateTools(layerProvider)
    return result

def ITkPixelLayerProviderInnerCfg(flags, name='ITkPixelProviderInner',
                                  useCond=True,
                                  **kwargs):
    result = ComponentAccumulator()

    if "LayerBuilder" not in kwargs:
        from TrackingGeometryCondAlg.InDetTrackingGeometryConfig import (
            ITkPixelLayerBuilderInnerCfg)
        layerBuilder = result.popToolsAndMerge(
            ITkPixelLayerBuilderInnerCfg(flags, useCond = useCond))
        result.addPublicTool(layerBuilder)
        kwargs.setdefault("LayerBuilder", layerBuilder)

    layerProvider = CompFactory.Trk.LayerProviderCond(name, **kwargs) \
                    if useCond else \
                       CompFactory.Trk.LayerProvider(name, **kwargs)
    result.setPrivateTools(layerProvider)
    return result

def ITkPixelLayerProviderOuterCfg(flags, name='ITkPixelProviderOuter',
                                  useCond=True,
                                  **kwargs):
    result = ComponentAccumulator()

    if "LayerBuilder" not in kwargs:
        from TrackingGeometryCondAlg.InDetTrackingGeometryConfig import (
            ITkPixelLayerBuilderOuterCfg)
        layerBuilder = result.popToolsAndMerge(
            ITkPixelLayerBuilderOuterCfg(flags, useCond = useCond))
        result.addPublicTool(layerBuilder)
        kwargs.setdefault("LayerBuilder", layerBuilder)

    layerProvider = CompFactory.Trk.LayerProviderCond(name, **kwargs) \
                    if useCond else \
                       CompFactory.Trk.LayerProvider(name, **kwargs)
    result.setPrivateTools(layerProvider)
    return result

def ITkStripLayerProviderCfg(flags, name='ITkStripProvider',
                             useCond=True,
                             **kwargs):
    result = ComponentAccumulator()

    if "LayerBuilder" not in kwargs:
       from TrackingGeometryCondAlg.InDetTrackingGeometryConfig import (
            ITkStripLayerBuilderCfg)
       layerBuilder = result.popToolsAndMerge(
           ITkStripLayerBuilderCfg(flags, useCond = useCond))
       result.addPublicTool(layerBuilder)
       kwargs.setdefault("LayerBuilder", layerBuilder)

    layerProvider = CompFactory.Trk.LayerProviderCond(name, **kwargs) \
                    if useCond else \
                       CompFactory.Trk.LayerProvider(name, **kwargs)
    result.setPrivateTools(layerProvider)
    return result

def LayerMaterialProviderCfg(flags, name='AtlasMaterialCondProvider', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(
        CompFactory.Trk.LayerMaterialProvider(name, **kwargs))
    return result

def InputLayerMaterialProviderCfg(flags, name='AtlasMaterialProvider',
                                  **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(
        CompFactory.Trk.InputLayerMaterialProvider(name, **kwargs))
    return result

def LayerMaterialInspectorCfg(flags, name='AtlasLayerMaterialInspector',
                              **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(
        CompFactory.Trk.LayerMaterialInspector(name, **kwargs))
    return result
