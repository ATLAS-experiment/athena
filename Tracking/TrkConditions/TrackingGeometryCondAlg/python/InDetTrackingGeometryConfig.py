# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTrackingGeometry package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, ProductionStep

def BeamPipeBuilderCfg(flags, name='BeamPipeBuilder', useCond=True, **kwargs):
    from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
    result = BeamPipeGeometryCfg(flags)

    if useCond:
        name = name+'Cond'
    beamPipeBuilder = CompFactory.InDet.BeamPipeBuilderCond(name, **kwargs) \
                      if useCond else \
                         CompFactory.InDet.BeamPipeBuilder(name, **kwargs)

    result.setPrivateTools(beamPipeBuilder)
    return result

def ITkBeamPipeBuilderCfg(flags, name='ITkBeamPipeBuilder',
                          useCond=True,
                          **kwargs):
    kwargs.setdefault("BeamPipeMaterialBinsZ",
                      flags.ITk.trackingGeometry.beampipeMatZbins)
    return BeamPipeBuilderCfg(flags, name, useCond, **kwargs)

def PixelLayerBuilderCfg(flags, name='PixelLayerBuilder',
                         useCond=True,
                         **kwargs):
    # for Pixel DetectorElement conditions data:
    if flags.Common.ProductionStep != ProductionStep.Simulation: # Not needed by FATRAS
        from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
        result = PixelReadoutGeometryCfg(flags)
    else:
        from PixelGeoModel.PixelGeoModelConfig import PixelSimulationGeometryCfg
        result = PixelSimulationGeometryCfg(flags)

    kwargs.setdefault("PixelCase", True)
    kwargs.setdefault("Identification", 'Pixel')
    kwargs.setdefault("SiDetManagerLocation", 'Pixel')
    # additional layers - handle with care !
    kwargs.setdefault("BarrelAdditionalLayerRadii", [130.0])   # The PST
    kwargs.setdefault("BarrelAdditionalLayerType", [0])     # -- will shift volume boundary to PST
    # DBM
    kwargs.setdefault("EndcapAdditionalLayerPositionsZ", [-1900., 1900.])
    kwargs.setdefault("EndcapAdditionalLayerType", [1, 1])  # DBM
    # Pixel barrel specifications
    kwargs.setdefault("BarrelLayerBinsZ", 1)
    kwargs.setdefault("BarrelLayerBinsPhi", 1)
    kwargs.setdefault("EndcapLayerBinsR", 1)
    kwargs.setdefault("EndcapLayerBinsPhi", 1)
    kwargs.setdefault("SetLayerAssociation", True)
    kwargs.setdefault("BarrelEnvelope", 0.0)

    if useCond:
        name = name+'Cond'
    pixelLayerBuilder = CompFactory.InDet.SiLayerBuilderCond(name, **kwargs) \
                        if useCond else \
                           CompFactory.InDet.SiLayerBuilder(name, **kwargs)
    result.setPrivateTools(pixelLayerBuilder)
    return result

def ITkPixelLayerBuilderCfg(flags, name='ITkPixelLayerBuilder',
                            useCond=True,
                            **kwargs):
    # for Pixel DetectorElement conditions data:
    if flags.Common.ProductionStep != ProductionStep.Simulation: # Not needed by FATRAS
        from PixelGeoModelXml.ITkPixelGeoModelConfig import (
            ITkPixelReadoutGeometryCfg)
        result = ITkPixelReadoutGeometryCfg(flags)
    else:
        from PixelGeoModelXml.ITkPixelGeoModelConfig import (
            ITkPixelSimulationGeometryCfg)
        result = ITkPixelSimulationGeometryCfg(flags)

    kwargs.setdefault("PixelCase", True)
    kwargs.setdefault("SiDetManagerLocation", 'ITkPixel')
    kwargs.setdefault("PixelReadKey", 'ITkPixelDetectorElementCollection')
    kwargs.setdefault("SCT_ReadKey", 'ITkStripDetectorElementCollection')
    kwargs.setdefault("UseRingLayout", True)
    # Pixel barrel specifications
    kwargs.setdefault("BarrelLayerBinsZ",
                      flags.ITk.trackingGeometry.pixelBarrelMatZbins)
    kwargs.setdefault("BarrelLayerBinsPhi",
                      flags.ITk.trackingGeometry.pixelBarrelMatPhiBins)
    kwargs.setdefault("EndcapLayerBinsR",
                      flags.ITk.trackingGeometry.pixelEndcapMatRbins)
    kwargs.setdefault("EndcapLayerBinsPhi",
                      flags.ITk.trackingGeometry.pixelEndcapMatPhiBins)
    kwargs.setdefault("SetLayerAssociation", True)

    if useCond:
        name = name+'Cond'
    pixelLayerBuilder = CompFactory.InDet.SiLayerBuilderCond(name, **kwargs) \
                        if useCond else \
                           CompFactory.InDet.SiLayerBuilder(name, **kwargs)
    result.setPrivateTools(pixelLayerBuilder)
    return result

def ITkPixelLayerBuilderInnerCfg(flags, name='ITkPixelLayerBuilderInner',
                                 useCond=True,
                                 **kwargs):
    kwargs.setdefault("Identification", 'ITkPixelInner')
    kwargs.setdefault("LayerIndicesBarrel", [0, 1])
    kwargs.setdefault("LayerIndicesEndcap", [0, 1, 2])
    # adding one passive layer to map PLR material
    kwargs.setdefault("EndcapAdditionalLayerPositionsZ", [-2250., 2250.])
    kwargs.setdefault("EndcapAdditionalLayerType", [1, 1])
    return ITkPixelLayerBuilderCfg(flags, name, useCond, **kwargs)

def ITkPixelLayerBuilderOuterCfg(flags, name='ITkPixelLayerBuilderOuter',
                                 useCond=True,
                                 **kwargs):
    kwargs.setdefault("Identification", 'ITkPixelOuter')
    kwargs.setdefault("EndcapEnvelope", 25.)
    kwargs.setdefault("LayerIndicesBarrel", [2, 3, 4])
    kwargs.setdefault("LayerIndicesEndcap", [3, 4, 5, 6, 7, 8])
    return ITkPixelLayerBuilderCfg(flags, name, useCond, **kwargs)

def SCT_LayerBuilderCfg(flags, name='SCT_LayerBuilder', useCond=True, **kwargs):
    # for SCT DetectorElement conditions data:
    if flags.Common.ProductionStep != ProductionStep.Simulation: # Not needed by FATRAS
        from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
        result = SCT_ReadoutGeometryCfg(flags)
    else:
        from SCT_GeoModel.SCT_GeoModelConfig import SCT_SimulationGeometryCfg
        result = SCT_SimulationGeometryCfg(flags)

    kwargs.setdefault("PixelCase", False)
    kwargs.setdefault("Identification", 'SCT')
    kwargs.setdefault("SiDetManagerLocation", 'SCT')
    # additionall layers - handle with care !
    kwargs.setdefault("BarrelAdditionalLayerRadii", [])
    kwargs.setdefault("BarrelAdditionalLayerType", [])
    kwargs.setdefault("EndcapAdditionalLayerPositionsZ", [-2850., 2850.])
    kwargs.setdefault("EndcapAdditionalLayerType", [0, 0])
    kwargs.setdefault("BarrelLayerBinsZ", 1)
    kwargs.setdefault("BarrelLayerBinsPhi", 1)
    # SCT endcap specifications
    kwargs.setdefault("EndcapLayerBinsR", 1)
    kwargs.setdefault("EndcapLayerBinsPhi", 1)
    kwargs.setdefault("EndcapComplexRingBinning", True)
    kwargs.setdefault("SetLayerAssociation", True)
    kwargs.setdefault("BarrelEnvelope", 0.0)

    if useCond:
        name = name+'Cond'
    SCT_LayerBuilder = CompFactory.InDet.SiLayerBuilderCond(name, **kwargs) \
                       if useCond else \
                          CompFactory.InDet.SiLayerBuilder(name, **kwargs)
    result.setPrivateTools(SCT_LayerBuilder)
    return result

def ITkStripLayerBuilderCfg(flags, name='ITkStripLayerBuilder',
                            useCond=True,
                            **kwargs):
    if flags.Common.ProductionStep != ProductionStep.Simulation: # Not needed by FATRAS
        from StripGeoModelXml.ITkStripGeoModelConfig import (
            ITkStripReadoutGeometryCfg)
        result = ITkStripReadoutGeometryCfg(flags)
    else:
        from StripGeoModelXml.ITkStripGeoModelConfig import (
            ITkStripSimulationGeometryCfg)
        result = ITkStripSimulationGeometryCfg(flags)

    kwargs.setdefault("PixelCase", False)
    kwargs.setdefault("Identification", 'ITkStrip')
    kwargs.setdefault("SiDetManagerLocation", 'ITkStrip')
    kwargs.setdefault("PixelReadKey", 'ITkPixelDetectorElementCollection')
    kwargs.setdefault("SCT_ReadKey", 'ITkStripDetectorElementCollection')
    kwargs.setdefault("AddMoreSurfaces", True)
    # additionall layers - handle with care !
    kwargs.setdefault("BarrelLayerBinsZ",
                      flags.ITk.trackingGeometry.stripBarrelMatZbins)
    kwargs.setdefault("BarrelLayerBinsPhi",
                      flags.ITk.trackingGeometry.stripBarrelMatPhiBins)
    kwargs.setdefault("EndcapLayerBinsR",
                      flags.ITk.trackingGeometry.stripEndcapMatRbins)
    kwargs.setdefault("EndcapLayerBinsPhi",
                      flags.ITk.trackingGeometry.stripEndcapMatPhiBins)
    kwargs.setdefault("SetLayerAssociation", True)

    if useCond:
        name = name+'Cond'
    stripLayerBuilder = CompFactory.InDet.SiLayerBuilderCond(name, **kwargs) \
                        if useCond else \
                           CompFactory.InDet.SiLayerBuilder(name, **kwargs)
    result.setPrivateTools(stripLayerBuilder)
    return result

def TRT_LayerBuilderCfg(flags, name='TRT_LayerBuilder', useCond=True, **kwargs):
    # for TRT DetectorElement conditions data:
    from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
    result = TRT_ReadoutGeometryCfg(flags)

    useFatras = flags.Common.ProductionStep in \
                [ProductionStep.Simulation, ProductionStep.FastChain] \
                and flags.Sim.ISF.Simulator.usesFatras()

    kwargs.setdefault("ModelLayersOnly", 
                      flags.Beam.Type is not BeamType.Cosmics and \
                      not(useFatras))
                      
    if not useFatras:
        kwargs.setdefault("BarrelLayerBinsZ", 1)
        kwargs.setdefault("BarrelLayerBinsPhi", 1)
        kwargs.setdefault("EndcapLayerBinsR", 1)

    if useCond:
        name = name+'Cond'
    TRT_LayerBuilder = CompFactory.InDet.TRT_LayerBuilderCond(name, **kwargs) \
                       if useCond else \
                          CompFactory.InDet.TRT_LayerBuilder(name, **kwargs)
    result.setPrivateTools(TRT_LayerBuilder)
    return result

# Geometry builders

def InDetTrackingGeometryBuilderCfg(flags, name='InDetTrackingGeometryBuilder',
                                    useCond=True,
                                    **kwargs):
    result = ComponentAccumulator()

    # beampipe
    beamPipeBuilder = result.popToolsAndMerge(
        BeamPipeBuilderCfg(flags, useCond = useCond))
    result.addPublicTool(beamPipeBuilder)

    layerbuilders = []
    binnings = []
    colors = []

    # Material due to InDet services
    if (useCond and flags.Detector.GeometryID):
        from InDetServMatGeoModel.InDetServMatGeoModelConfig import (
            InDetServiceMaterialCfg)
        result.merge(InDetServiceMaterialCfg(flags))

    # Pixel
    if flags.Detector.GeometryPixel:
        pixelLayerBuilder = result.popToolsAndMerge(
            PixelLayerBuilderCfg(flags, useCond = useCond))
        result.addPublicTool(pixelLayerBuilder)

        # the binning type of the layers
        pixelLayerBinning = 2

        # put them to the caches
        layerbuilders += [pixelLayerBuilder]
        binnings += [pixelLayerBinning]
        colors += [3]

    if flags.Detector.GeometrySCT:
        # SCT building
        SCT_LayerBuilder = result.popToolsAndMerge(
            SCT_LayerBuilderCfg(flags, useCond = useCond))
        result.addPublicTool(SCT_LayerBuilder)

        # the binning type of the layer
        SCT_LayerBinning = 2

        # put them to the caches
        layerbuilders += [SCT_LayerBuilder]
        binnings += [SCT_LayerBinning]
        colors += [4]

    if flags.Detector.GeometryTRT:
        TRT_LayerBuilder = result.popToolsAndMerge(
            TRT_LayerBuilderCfg(flags, useCond = useCond))
        
        # The RobustTrackingGeometryBuilderCond expects a public tool
        # but for some unknown reason, this creates a crash
        result.addPublicTool(TRT_LayerBuilder)

        # set the binning from bi-aequidistant to arbitrary for complex TRT
        # volumes
        TRT_LayerBinning = 1
        useFatras = flags.Common.ProductionStep in \
                    [ProductionStep.Simulation, ProductionStep.FastChain] \
                    and flags.Sim.ISF.Simulator.usesFatras()
        if (flags.Beam.Type is BeamType.Cosmics or useFatras):
            TRT_LayerBinning = 2

        # put them to the caches
        layerbuilders += [TRT_LayerBuilder]
        binnings += [TRT_LayerBinning]
        colors += [5]

    # The following public sub-tools of the cylinder volume creator are only
    # configured here to rename them consistently with the legacy config to
    # avoid conflict with CA wrapping
    # Those configs can be removed once the support for legacy config is dropped
    nameSuffix = 'Cond' if useCond else ''

    # helpers for the InDetTrackingGeometry Builder : layer array creator
    from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
        LayerArrayCreatorCfg)
    layerArrayCreator = result.popToolsAndMerge(
        LayerArrayCreatorCfg(flags, name='InDetLayerArrayCreator' + nameSuffix))
    result.addPublicTool(layerArrayCreator)

    # helpers for the InDetTrackingGeometry Builder : tracking volume helper
    from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
        InDetTrackingVolumeHelperCfg)
    trackingVolumeHelper = result.popToolsAndMerge(
        InDetTrackingVolumeHelperCfg(flags, name='InDetTrackingVolumeHelper' + \
                                     nameSuffix))
    result.addPublicTool(trackingVolumeHelper)

    # helpers for the InDetTrackingGeometry Builder : tracking volume array creator
    from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
        TrackingVolumeArrayCreatorCfg)
    trackingVolumeArrayCreator = result.popToolsAndMerge(
        TrackingVolumeArrayCreatorCfg(flags,
                                      name='InDetTrackingVolumeArrayCreator' + \
                                      nameSuffix))
    result.addPublicTool(trackingVolumeArrayCreator)

    # helpers for the InDetTrackingGeometry Builder : cylinder volume creator
    from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
        InDetCylinderVolumeCreatorCfg)
    inDetCylinderVolumeCreator = result.popToolsAndMerge(
        InDetCylinderVolumeCreatorCfg(flags, name='InDetCylinderVolumeCreator' \
                                      + nameSuffix,
                                      LayerArrayCreator = layerArrayCreator,
                                      TrackingVolumeHelper = trackingVolumeHelper,
                                      TrackingVolumeArrayCreator = trackingVolumeArrayCreator))
    result.addPublicTool(inDetCylinderVolumeCreator)

    from SubDetectorEnvelopes.SubDetectorEnvelopesConfig import (
        EnvelopeDefSvcCfg)
    envelopeDefinitionSvc = result.getPrimaryAndMerge(EnvelopeDefSvcCfg(flags))

    kwargs.setdefault("BeamPipeBuilder", beamPipeBuilder)
    kwargs.setdefault("LayerBuilders", layerbuilders)
    kwargs.setdefault("LayerBinningType", binnings)
    kwargs.setdefault("ColorCodes", colors)
    kwargs.setdefault("EnvelopeDefinitionSvc", envelopeDefinitionSvc)
    kwargs.setdefault("VolumeEnclosureDiscPositions", [3000., 3450.])
    kwargs.setdefault("TrackingVolumeCreator", inDetCylinderVolumeCreator)
    kwargs.setdefault("LayerArrayCreator", layerArrayCreator)
    kwargs.setdefault("ReplaceAllJointBoundaries", True)
    kwargs.setdefault("VolumeEnclosureCylinderRadii", [])
    kwargs.setdefault("BuildBoundaryLayers", True)
    kwargs.setdefault("ExitVolumeName", 'InDet::Containers::InnerDetector')

    # the tracking geometry builder
    if useCond:
        name = name + 'Cond'
    geometryBuilder = CompFactory.InDet.RobustTrackingGeometryBuilderCond(name, **kwargs) if useCond else \
                      CompFactory.InDet.RobustTrackingGeometryBuilder(name, **kwargs)
    result.setPrivateTools(geometryBuilder)
    return result

def ITkTrackingGeometryBuilderCfg(flags, name='ITkTrackingGeometryBuilder',
                                  useCond = True,
                                  **kwargs):
    result = ComponentAccumulator()

    # beampipe
    from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
        ITkBeamPipeProviderCfg)
    beamPipeProvider = result.popToolsAndMerge(
        ITkBeamPipeProviderCfg(flags, useCond = useCond))
    result.addPublicTool(beamPipeProvider)

    beamPipeBinning = 2

    layerProviders = [beamPipeProvider]
    binnings_barrel = [beamPipeBinning]
    binnings_endcap = [beamPipeBinning]
    colors = [2]

    # Pixel
    if flags.Detector.GeometryITkPixel:
        from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
            ITkPixelLayerProviderInnerCfg)
        pixelProviderInner = result.popToolsAndMerge(
            ITkPixelLayerProviderInnerCfg(flags, useCond = useCond))
        result.addPublicTool(pixelProviderInner)

        # the binning type of the layers
        pixelLayerBinning = 2

        # put them to the caches
        layerProviders += [pixelProviderInner]
        binnings_barrel += [pixelLayerBinning]
        binnings_endcap += [pixelLayerBinning]
        colors += [3]

        from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
            ITkPixelLayerProviderOuterCfg)
        pixelProviderOuter = result.popToolsAndMerge(
            ITkPixelLayerProviderOuterCfg(flags, useCond = useCond))
        result.addPublicTool(pixelProviderOuter)

        # the binning type of the layers
        pixelLayerBinning = 2

        # put them to the caches
        layerProviders += [pixelProviderOuter]
        binnings_barrel += [pixelLayerBinning]
        binnings_endcap += [pixelLayerBinning]
        colors += [3]

    if flags.Detector.GeometryITkStrip:
        from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
            ITkStripLayerProviderCfg)
        stripProvider = result.popToolsAndMerge(
            ITkStripLayerProviderCfg(flags, useCond = useCond))
        result.addPublicTool(stripProvider)

        # the binning type of the layer
        stripLayerBinning = 2

        # put them to the caches
        layerProviders += [stripProvider]
        binnings_barrel += [stripLayerBinning]
        binnings_endcap += [stripLayerBinning]
        colors += [4]

    # helpers for the InDetTrackingGeometry Builder : layer array creator
    from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
        LayerArrayCreatorCfg)
    layerArrayCreator = result.popToolsAndMerge(
        LayerArrayCreatorCfg(flags))
    result.addPublicTool(layerArrayCreator)

    # helpers for the InDetTrackingGeometry Builder : cylinder volume creator
    from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
        ITkCylinderVolumeCreatorCfg)
    cylinderVolumeCreator = result.popToolsAndMerge(
        ITkCylinderVolumeCreatorCfg(flags))
    result.addPublicTool(cylinderVolumeCreator)

    from SubDetectorEnvelopes.SubDetectorEnvelopesConfig import EnvelopeDefSvcCfg
    envelopeDefinitionSvc = result.getPrimaryAndMerge(EnvelopeDefSvcCfg(flags))

    kwargs.setdefault("LayerBuilders", layerProviders)
    kwargs.setdefault("LayerBinningTypeCenter", binnings_barrel)
    kwargs.setdefault("LayerBinningTypeEndcap", binnings_endcap)
    kwargs.setdefault("ColorCodes", colors)
    kwargs.setdefault("EnvelopeDefinitionSvc", envelopeDefinitionSvc)
    kwargs.setdefault("TrackingVolumeCreator", cylinderVolumeCreator)
    kwargs.setdefault("LayerArrayCreator", layerArrayCreator)
    kwargs.setdefault("CheckForRingLayout", True)
    kwargs.setdefault("MinimalRadialGapForVolumeSplit",
                      flags.ITk.trackingGeometry.minimalRadialGapForVolumeSplit)
    kwargs.setdefault("ReplaceAllJointBoundaries", True)
    kwargs.setdefault("BuildBoundaryLayers", True)
    kwargs.setdefault("ExitVolumeName", 'InDet::Containers::InnerDetector')
    kwargs.setdefault("RemoveHGTD", True)
    kwargs.setdefault("ZminHGTD", 3420.)

    if useCond:
        name = name + 'Cond'
    GeometryBuilder = CompFactory.InDet.StagedTrackingGeometryBuilderCond(name, **kwargs) if useCond else \
                      CompFactory.InDet.StagedTrackingGeometryBuilder(name, **kwargs)
    result.setPrivateTools(GeometryBuilder)
    return result
