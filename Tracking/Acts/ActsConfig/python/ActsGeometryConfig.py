# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator 
from AthenaConfiguration.ComponentFactory import CompFactory


def ActsTrackingGeometrySvcCfg(flags,
                               name: str = "ActsTrackingGeometrySvc",
                               **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()

  from ROOT.ActsTrk import DetectorType 
  kwargs.setdefault("NotAlignDetectors", [DetectorType.Trt, 
                                          DetectorType.Hgtd])
  kwargs.setdefault("UseBlueprint", flags.Acts.TrackingGeometry.UseBlueprint)
  kwargs.setdefault("ObjDebugOutput", flags.Acts.TrackingGeometry.ObjDebugOutput)
 
  subDetectors = []
  blueprintTools = []

  if flags.Detector.GeometryBpipe:
    from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
    acc.merge(BeamPipeGeometryCfg(flags))
    kwargs.setdefault("BuildBeamPipe", True)

  if flags.Detector.GeometryPixel:
    subDetectors += ["Pixel"]
    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    acc.merge(PixelReadoutGeometryCfg(flags))
   

  if flags.Detector.GeometrySCT:
    subDetectors += ["SCT"]
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc.merge(SCT_ReadoutGeometryCfg(flags))

  if flags.Detector.GeometryTRT:
    # Commented out because TRT is not production ready yet and we don't 
    # want to turn it on even if the global flag is set
    #  subDetectors += ["TRT"]
    from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
    acc.merge(TRT_ReadoutGeometryCfg(flags))

  if flags.Detector.GeometryCalo:
    # Commented out because Calo is not production ready yet and we don't 
    # want to turn it on even if the global flag is set
    #  subDetectors += ["Calo"]
    #  kwargs.setdefault("CaloVolumeBuilder", CompFactory.ActsCaloTrackingVolumeBuilder())

    # need to configure calo geometry, otherwise we get a crash
    # Do this even though it's not production ready yet, so the service can
    # be forced to build the calorimeter later on anyway
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

  if flags.Muon.usePhaseIIGeoSetup and not flags.Acts.TrackingGeometry.UseBlueprint:
    subDetectors += ["Muon"]
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc.merge(MuonGeoModelCfg(flags))    
    from ActsMuonDetector.ActsMuonDetectorCfg import  MsTrackingVolumeBuilderCfg
    kwargs.setdefault("MSVolumeBuilder", acc.popToolsAndMerge(MsTrackingVolumeBuilderCfg(flags)))

  #first add the itk builder and then the muon system - this is the correct order
  if flags.Acts.TrackingGeometry.UseBlueprint:    
    if flags.Detector.GeometryITkPixel or flags.Detector.GeometryITkStrip:
      blueprintTools += [acc.popToolsAndMerge(ItkBlueprintNodeBuilderCfg(flags))]
    if flags.Detector.GeometryMuon:
      subDetectors += ["Muon"]
      from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
      acc.merge(MuonGeoModelCfg(flags))  
      from ActsMuonDetector.ActsMuonDetectorCfg import MuonBlueprintNodeBuilderCfg
      blueprintTools += [acc.popToolsAndMerge(MuonBlueprintNodeBuilderCfg(flags))]
        # also Calo needs to be added
  
  if flags.Detector.GeometryITkPixel:
    subDetectors += ["ITkPixel"]
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))
    
  if flags.Detector.GeometryITkStrip:
    subDetectors += ["ITkStrip"]
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))
    

  if flags.Detector.GeometryHGTD:
    subDetectors += ["HGTD"]
    if flags.HGTD.Geometry.useGeoModelXml:
        from HGTD_GeoModelXml.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    else:
        from HGTD_GeoModel.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    acc.merge(HGTD_ReadoutGeometryCfg(flags))

  actsTrackingGeometrySvc = CompFactory.ActsTrackingGeometrySvc(name,
                                                                BuildSubDetectors=subDetectors,
                                                                BlueprintNodeBuilders=blueprintTools,
                                                                **kwargs)

  if flags.Acts.TrackingGeometry.MaterialSource == "Default":
    if flags.Detector.GeometryITk:
      extension = "ITk"
      if flags.Detector.GeometryHGTD:
        extension += "-HGTD"
      if flags.Acts.TrackingGeometry.InsertITkPassiveMaterialLayers:
        extension += "-passiveLayers"
      if flags.Acts.TrackingGeometry.MaterialFileExtension:
        extension += "-"+flags.Acts.TrackingGeometry.MaterialFileExtension
      actsTrackingGeometrySvc.UseMaterialMap = True
      actsTrackingGeometrySvc.MaterialMapCalibFolder = flags.Acts.TrackingGeometry.MaterialCalibrationFolder
      actsTrackingGeometrySvc.MaterialMapInputFile = \
        "material-maps-" + flags.GeoModel.AtlasVersion + "-" + extension + ".json"

  elif flags.Acts.TrackingGeometry.MaterialSource.find(".json") != -1:
    actsTrackingGeometrySvc.UseMaterialMap = True
    actsTrackingGeometrySvc.MaterialMapCalibFolder = flags.Acts.TrackingGeometry.MaterialCalibrationFolder
    actsTrackingGeometrySvc.MaterialMapInputFile = flags.Acts.TrackingGeometry.MaterialSource

  if flags.Acts.TrackingGeometry.InsertITkPassiveMaterialLayers:
    actsTrackingGeometrySvc.PassiveITkInnerPixelBarrelLayerRadii = flags.Acts.TrackingGeometry.PassiveITkInnerPixelBarrelLayerRadii
    actsTrackingGeometrySvc.PassiveITkInnerPixelBarrelLayerHalflengthZ = flags.Acts.TrackingGeometry.PassiveITkInnerPixelBarrelLayerHalflengthZ
    actsTrackingGeometrySvc.PassiveITkInnerPixelBarrelLayerThickness = flags.Acts.TrackingGeometry.PassiveITkInnerPixelBarrelLayerThickness
    actsTrackingGeometrySvc.PassiveITkOuterPixelBarrelLayerRadii = flags.Acts.TrackingGeometry.PassiveITkOuterPixelBarrelLayerRadii
    actsTrackingGeometrySvc.PassiveITkOuterPixelBarrelLayerHalflengthZ = flags.Acts.TrackingGeometry.PassiveITkOuterPixelBarrelLayerHalflengthZ
    actsTrackingGeometrySvc.PassiveITkOuterPixelBarrelLayerThickness = flags.Acts.TrackingGeometry.PassiveITkOuterPixelBarrelLayerThickness
    actsTrackingGeometrySvc.PassiveITkStripBarrelLayerRadii = flags.Acts.TrackingGeometry.PassiveITkStripBarrelLayerRadii
    actsTrackingGeometrySvc.PassiveITkStripBarrelLayerHalflengthZ = flags.Acts.TrackingGeometry.PassiveITkStripBarrelLayerHalflengthZ
    actsTrackingGeometrySvc.PassiveITkStripBarrelLayerThickness = flags.Acts.TrackingGeometry.PassiveITkStripBarrelLayerThickness

  

  acc.addService(actsTrackingGeometrySvc, primary = True)
  return acc


def ActsPropStepRootWriterSvcCfg(flags,
                                 name: str = "ActsPropStepRootWriterSvc",
                                 **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.addService(CompFactory.ActsPropStepRootWriterSvc(name, **kwargs))
    return acc


def ActsTrackingGeometryToolCfg(flags,
                                name: str = "ActsTrackingGeometryTool" ) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  acc.merge(ActsTrackingGeometrySvcCfg(flags))
  from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
  acc.merge(ActsGeometryContextAlgCfg(flags))
  acc.addPublicTool(CompFactory.ActsTrackingGeometryTool(name), primary = True)
  return acc

def ActsExtrapolationToolCfg(flags,
                             name: str = "ActsExtrapolationTool",
                             **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
  acc.merge(AtlasFieldCacheCondAlgCfg(flags))
  kwargs.setdefault("TrackingGeometryTool", acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle
  acc.setPrivateTools(CompFactory.ActsTrk.ExtrapolationTool(name, **kwargs))
  return acc


def ActsMaterialTrackWriterSvcCfg(flags,
                                  name: str = "ActsMaterialTrackWriterSvc",
                                  **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  acc.merge(ActsTrackingGeometrySvcCfg(flags))
  acc.addService(CompFactory.ActsMaterialTrackWriterSvc(name, **kwargs), primary=True)
  return acc


def ActsMaterialStepConverterToolCfg(flags,
                                     name: str = "ActsMaterialStepConverterTool",
                                     **kwargs ) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  acc.addPublicTool(CompFactory.ActsMaterialStepConverterTool(name, **kwargs), primary=True)
  return acc


def ActsSurfaceMappingToolCfg(flags,
                              name: str = "ActsSurfaceMappingTool",
                              **kwargs ) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  kwargs.setdefault("TrackingGeometryTool", acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle
  acc.addPublicTool(CompFactory.ActsSurfaceMappingTool(name, **kwargs), primary=True)
  return acc


def ActsVolumeMappingToolCfg(flags,
                             name: str = "ActsVolumeMappingTool",
                             **kwargs ) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  kwargs.setdefault("TrackingGeometryTool", acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle
  acc.addPublicTool(CompFactory.ActsVolumeMappingTool(name, **kwargs), primary=True)
  return acc


def ActsMaterialJsonWriterToolCfg(flags,
                                  name: str = "ActsMaterialJsonWriterTool",
                                  **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  acc.addPublicTool(CompFactory.ActsMaterialJsonWriterTool(name, **kwargs), primary=True)
  return acc


def ActsObjWriterToolCfg(flags,
                         name: str = "ActsObjWriterTool",
                         **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  acc.addPublicTool(CompFactory.ActsObjWriterTool(name, **kwargs), primary=True)
  return acc


def ActsExtrapolationAlgCfg(flags,
                            name: str = "ActsExtrapolationAlg",
                            **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()

  if "ExtrapolationTool" not in kwargs:
    kwargs.setdefault("ExtrapolationTool", acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags))) # PrivateToolHandle

  acc.merge(ActsPropStepRootWriterSvcCfg(flags, FilePath="propsteps.root", TreeName="propsteps"))
  acc.addEventAlgo(CompFactory.ActsExtrapolationAlg(name, **kwargs))
  return acc

def ActsWriteTrackingGeometryCfg(flags,
                                 name: str = "ActsWriteTrackingGeometry",
                                 **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if 'TrackingGeometryTool' not in kwargs:
      kwargs.setdefault("TrackingGeometryTool", acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle

    if 'MaterialJsonWriterTool' not in kwargs:
      kwargs.setdefault("MaterialJsonWriterTool", acc.getPrimaryAndMerge(ActsMaterialJsonWriterToolCfg(flags,
                                                                                                       OutputFile = "geometry-maps.json",
                                                                                                       processSensitives = False,
                                                                                                       processNonMaterial = True) ))

    subDetectors = []
    if flags.Detector.GeometryBpipe:
      subDetectors = ["BeamPipe"]

    if flags.Detector.GeometryPixel:
      subDetectors += ["Pixel"]
    if flags.Detector.GeometryITkPixel:
      subDetectors += ["ITkPixel"]

    if flags.Detector.GeometrySCT:
      subDetectors += ["SCT"]
    if flags.Detector.GeometryITkStrip:
      subDetectors += ["ITkStrip"]
    if flags.Detector.GeometryHGTD:
      subDetectors += ["HGTD"]

    acc.addEventAlgo(CompFactory.ActsWriteTrackingGeometry(name, **kwargs))
    return acc

def ActsWriteTrackingGeometryTransformsAlgCfg(flags,
                                              name: str = "ActsWriteTrackingGeometryTransformsAlg",
                                              **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if 'TrackingGeometryTool' not in kwargs:
      kwargs.setdefault("TrackingGeometryTool", acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle

    acc.addEventAlgo(CompFactory.ActsWriteTrackingGeometryTransforms(name,**kwargs))
    return acc

def ActsMaterialMappingCfg(flags,
                           name: str = "ActsMaterialMapping",
                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if 'MaterialStepConverterTool' not in kwargs:
      kwargs.setdefault("MaterialStepConverterTool", acc.getPrimaryAndMerge(ActsMaterialStepConverterToolCfg(flags)))

    if 'SurfaceMappingTool' not in kwargs:
      kwargs.setdefault("SurfaceMappingTool", acc.getPrimaryAndMerge(ActsSurfaceMappingToolCfg(flags)))

    if 'VolumeMappingTool' not in kwargs:
      kwargs.setdefault("VolumeMappingTool", acc.getPrimaryAndMerge(ActsVolumeMappingToolCfg(flags)))

    if 'MaterialJsonWriterTool' not in kwargs:
      kwargs.setdefault("MaterialJsonWriterTool",
                        acc.getPrimaryAndMerge( ActsMaterialJsonWriterToolCfg(flags,
                                                                              OutputFile = "material-maps.json",
                                                                              processSensitives = False,
                                                                              processNonMaterial = False) ))
      
    acc.addEventAlgo(CompFactory.ActsMaterialMapping(name, **kwargs))
    return acc

def ActsVolumeIdToDetectorCollectionMappingAlgCfg(flags,
                           name: str = "ActsVolumeIdToDetectorCollectionMappingAlgCfg",
                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if 'TrackingGeometryTool' not in kwargs :
      kwargs.setdefault('TrackingGeometryTool',
                        acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))
    kwargs.setdefault('ActsVolumeIdToDetectorElementCollectionMap', 'VolumeIdToDetectorElementCollectionMap')

    def filterCollections(flags, pixel_det_el, strip_det_el) :
      ret=[]
      if flags.Detector.GeometryITkPixel:
        ret += [ pixel_det_el ]
      if flags.Detector.GeometryITkStrip:
        ret += [ strip_det_el ]
      return ret
    kwargs.setdefault('DetectorElementsKeys', filterCollections( flags,
                                                                 'ITkPixelDetectorElementCollection',
                                                                 'ITkStripDetectorElementCollection'))

    acc.addCondAlgo(CompFactory.ActsTrk.ActsVolumeIdToDetectorElementCollectionMappingAlg(name, **kwargs))
    return acc

def ItkBlueprintNodeBuilderCfg(flags,
                                   name: str = "ItkBlueprintNodeBuilder",
                                   **kwargs) -> ComponentAccumulator:
    result = ComponentAccumulator()
    the_tool = CompFactory.ActsTrk.ItkBlueprintNodeBuilder(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

