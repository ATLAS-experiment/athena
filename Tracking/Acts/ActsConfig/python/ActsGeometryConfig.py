# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator 
from AthenaConfiguration.ComponentFactory import CompFactory


def ActsTrackingGeometrySvcCfg(flags,
                               name: str = "ActsTrackingGeometrySvc",
                               **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()

  from ROOT.ActsTrk import DetectorType 
  kwargs.setdefault("NotAlignDetectors", [DetectorType.Trt, 
                                          DetectorType.Hgtd])

  subDetectors = []
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
                                                                **kwargs)
  if flags.Detector.GeometryITk:
    if flags.Acts.TrackingGeometry.MaterialSource == "Default":
      extension = "ITk"
      if flags.Detector.GeometryHGTD:
        extension += "-HGTD"
      actsTrackingGeometrySvc.UseMaterialMap = True
      actsTrackingGeometrySvc.MaterialMapCalibFolder = flags.Acts.TrackingGeometry.MaterialCalibrationFolder
      actsTrackingGeometrySvc.MaterialMapInputFile = \
        "material-maps-" + flags.GeoModel.AtlasVersion + "-" + extension + ".json"
    elif flags.Acts.TrackingGeometry.MaterialSource.find(".json") != -1:
      actsTrackingGeometrySvc.UseMaterialMap = True
      actsTrackingGeometrySvc.MaterialMapCalibFolder = flags.Acts.TrackingGeometry.MaterialCalibrationFolder
      actsTrackingGeometrySvc.MaterialMapInputFile = flags.Acts.TrackingGeometry.MaterialSource

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
  acc.setPrivateTools(CompFactory.ActsTrackingGeometryTool(name))
  return acc

def ActsExtrapolationToolCfg(flags,
                             name: str = "ActsExtrapolationTool",
                             **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
  acc.merge(AtlasFieldCacheCondAlgCfg(flags))
  kwargs.setdefault("TrackingGeometryTool", acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle
  acc.setPrivateTools(CompFactory.ActsExtrapolationTool(name, **kwargs))
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
  kwargs.setdefault("TrackingGeometryTool", acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle
  acc.addPublicTool(CompFactory.ActsSurfaceMappingTool(name, **kwargs), primary=True)
  return acc


def ActsVolumeMappingToolCfg(flags,
                             name: str = "ActsVolumeMappingTool",
                             **kwargs ) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  kwargs.setdefault("TrackingGeometryTool", acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle
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
      kwargs.setdefault("TrackingGeometryTool", acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle

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

    if 'ObjWriterTool' not in kwargs: 
      kwargs.setdefault("ObjWriterTool",
                        acc.getPrimaryAndMerge(ActsObjWriterToolCfg(flags,
                                                                    OutputDirectory = "obj",
                                                                    SubDetectors = subDetectors) ))

    acc.addEventAlgo(CompFactory.ActsWriteTrackingGeometry(name, **kwargs))
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
