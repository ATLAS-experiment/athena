# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator 
from AthenaConfiguration.ComponentFactory import CompFactory


def ActsTrackingGeometrySvcCfg(flags, name = "ActsTrackingGeometrySvc", **kwargs) :
  result = ComponentAccumulator()

  from ROOT.ActsTrk import DetectorType 
  kwargs.setdefault("NotAlignDetectors", [DetectorType.Trt, 
                                          DetectorType.Hgtd])

  subDetectors = []
  if flags.Detector.GeometryBpipe:
    from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
    result.merge(BeamPipeGeometryCfg(flags))
    kwargs.setdefault("BuildBeamPipe", True)

  if flags.Detector.GeometryPixel:
    subDetectors += ["Pixel"]
    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    result.merge(PixelReadoutGeometryCfg(flags))

  if flags.Detector.GeometrySCT:
    subDetectors += ["SCT"]
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    result.merge(SCT_ReadoutGeometryCfg(flags))

  if flags.Detector.GeometryTRT:
    # Commented out because TRT is not production ready yet and we don't 
    # want to turn it on even if the global flag is set
    #  subDetectors += ["TRT"]
    from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
    result.merge(TRT_ReadoutGeometryCfg(flags))

  if flags.Detector.GeometryCalo:
    # Commented out because Calo is not production ready yet and we don't 
    # want to turn it on even if the global flag is set
    #  subDetectors += ["Calo"]
    #  kwargs.setdefault("CaloVolumeBuilder", CompFactory.ActsCaloTrackingVolumeBuilder())

    # need to configure calo geometry, otherwise we get a crash
    # Do this even though it's not production ready yet, so the service can
    # be forced to build the calorimeter later on anyway
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))

  if flags.Detector.GeometryITkPixel:
    subDetectors += ["ITkPixel"]
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    result.merge(ITkPixelReadoutGeometryCfg(flags))

  if flags.Detector.GeometryITkStrip:
    subDetectors += ["ITkStrip"]
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    result.merge(ITkStripReadoutGeometryCfg(flags))

  if flags.Detector.GeometryHGTD:
    subDetectors += ["HGTD"]
    if flags.HGTD.Geometry.useGeoModelXml:
        from HGTD_GeoModelXml.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    else:
        from HGTD_GeoModel.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    result.merge(HGTD_ReadoutGeometryCfg(flags))

  actsTrackingGeometrySvc = CompFactory.ActsTrackingGeometrySvc(name,
                                                                BuildSubDetectors=subDetectors,
                                                                **kwargs)
  if flags.Acts.TrackingGeometry.MaterialSource == "Default":
    if flags.Detector.GeometryITk:
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

  result.addService(actsTrackingGeometrySvc, primary = True)
  return result


def ActsPropStepRootWriterSvcCfg(flags,
                                 name="ActsPropStepRootWriterSvc",
                                 **kwargs):
    result = ComponentAccumulator()
    result.addService(CompFactory.ActsPropStepRootWriterSvc(name, **kwargs))
    return result


def ActsTrackingGeometryToolCfg(flags, name = "ActsTrackingGeometryTool" ) :
  result = ComponentAccumulator()
  result.merge(ActsTrackingGeometrySvcCfg(flags))
  result.merge(ActsAlignmentCondAlgCfg(flags))
  result.setPrivateTools(CompFactory.ActsTrackingGeometryTool(name))
  return result

def ActsDetAlignCondAlgCfg(flags, name="ActsDetAlignmentCondAlg", **kwargs):
  result = ComponentAccumulator()
  kwargs.setdefault("TrackingGeometrySvc", result.getPrimaryAndMerge(ActsTrackingGeometrySvcCfg(flags)))
  the_alg = CompFactory.ActsDetAlignCondAlg(name, **kwargs)
  result.addCondAlgo(the_alg, primary = True)
  return result
def ActsAlignmentCondAlgCfg(flags, name = "ActsAlignmentCondAlg", **kwargs) :
  result = ComponentAccumulator()
  from ROOT.ActsTrk import DetectorType 
  if flags.Detector.GeometryITk:
    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelAlignCondAlgCfg
    result.merge(ITkPixelAlignCondAlgCfg(flags))
    result.merge(ActsDetAlignCondAlgCfg(flags, name="ActsDetAlignmentCondAlgITkPixel",
                                               InputTransforms="ITkPixelAlignmentStore",
                                               ActsTransforms="ActsITkPixelAlignmentStore",
                                               DetectorType=DetectorType.Pixel))
    from SCT_ConditionsAlgorithms.ITkStripConditionsAlgorithmsConfig import ITkStripAlignCondAlgCfg
    result.merge(ITkStripAlignCondAlgCfg(flags))
    result.merge(ActsDetAlignCondAlgCfg(flags, name="ActsDetAlignmentCondAlgITkSct",
                                               InputTransforms="ITkStripAlignmentStore",
                                               ActsTransforms="ActsITkStripAlignmentStore",
                                               DetectorType=DetectorType.Sct))
    
    kwargs.setdefault("AlignmentStores", ["ActsITkPixelAlignmentStore", "ActsITkStripAlignmentStore"])
  else:
    from PixelConditionsAlgorithms.PixelConditionsConfig import PixelAlignCondAlgCfg
    result.merge(PixelAlignCondAlgCfg(flags))
    result.merge(ActsDetAlignCondAlgCfg(flags, name="ActsDetAlignmentCondAlgPixel",
                                               InputTransforms="PixelAlignmentStore",
                                               ActsTransforms="ActsPixelAlignmentStore",
                                               DetectorType=DetectorType.Pixel))
    
    from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConfig import SCT_AlignCondAlgCfg
    result.merge(SCT_AlignCondAlgCfg(flags))
    result.merge(ActsDetAlignCondAlgCfg(flags, name="ActsDetAlignmentCondAlgSct",
                                               InputTransforms="SCTAlignmentStore",
                                               ActsTransforms="ActsSCTAlignmentStore",
                                               DetectorType=DetectorType.Sct))
    kwargs.setdefault("AlignmentStores", ["ActsPixelAlignmentStore", "ActsSCTAlignmentStore"])

  result.addCondAlgo(CompFactory.ActsAlignmentCondAlg(name, **kwargs))
  return result


def ActsExtrapolationToolCfg(flags, name="ActsExtrapolationTool", **kwargs) :
  result=ComponentAccumulator()
  from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
  result.merge(AtlasFieldCacheCondAlgCfg(flags))
  kwargs.setdefault("TrackingGeometryTool", result.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle
  result.setPrivateTools(CompFactory.ActsExtrapolationTool(name, **kwargs))
  return result


def ActsMaterialTrackWriterSvcCfg(flags, name="ActsMaterialTrackWriterSvc", **kwargs) :
  result = ComponentAccumulator()
  result.merge(ActsTrackingGeometrySvcCfg(flags))
  result.addService(CompFactory.ActsMaterialTrackWriterSvc(name, **kwargs), primary=True)
  return result


def ActsMaterialStepConverterToolCfg(flags, name = "ActsMaterialStepConverterTool", **kwargs ) :
  result=ComponentAccumulator()
  result.addPublicTool(CompFactory.ActsMaterialStepConverterTool(name, **kwargs), primary=True)
  return result


def ActsSurfaceMappingToolCfg(flags, name = "ActsSurfaceMappingTool", **kwargs ) :
  result=ComponentAccumulator()
  kwargs.setdefault("TrackingGeometryTool", result.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle
  result.addPublicTool(CompFactory.ActsSurfaceMappingTool(name, **kwargs), primary=True)
  return result


def ActsVolumeMappingToolCfg(flags, name = "ActsVolumeMappingTool", **kwargs ) :
  result=ComponentAccumulator()
  kwargs.setdefault("TrackingGeometryTool", result.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle
  result.addPublicTool(CompFactory.ActsVolumeMappingTool(name, **kwargs), primary=True)
  return result


def ActsMaterialJsonWriterToolCfg(flags, name= "ActsMaterialJsonWriterTool", **kwargs) :
  result=ComponentAccumulator()
  result.addPublicTool(CompFactory.ActsMaterialJsonWriterTool(name, **kwargs), primary=True)
  return result


def ActsObjWriterToolCfg(flags, name= "ActsObjWriterTool", **kwargs) :
  result=ComponentAccumulator()
  result.addPublicTool(CompFactory.ActsObjWriterTool(name, **kwargs), primary=True)
  return result


def ActsExtrapolationAlgCfg(flags, name = "ActsExtrapolationAlg", **kwargs):
  result = ComponentAccumulator()

  if "ExtrapolationTool" not in kwargs:
    kwargs["ExtrapolationTool"] = result.popToolsAndMerge(ActsExtrapolationToolCfg(flags)) # PrivateToolHandle

  result.merge(ActsPropStepRootWriterSvcCfg(flags, FilePath="propsteps.root", TreeName="propsteps"))
  result.addEventAlgo(CompFactory.ActsExtrapolationAlg(name, **kwargs))
  return result

def ActsWriteTrackingGeometryCfg(flags, name="ActsWriteTrackingGeometry", **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("TrackingGeometryTool", result.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))) # PrivateToolHandle

    kwargs["MaterialJsonWriterTool"] = \
      result.getPrimaryAndMerge(ActsMaterialJsonWriterToolCfg(flags,
                                                              OutputFile = "geometry-maps.json",
                                                              processSensitives = False,
                                                              processNonMaterial = True) )

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

    kwargs["ObjWriterTool"] = \
      result.getPrimaryAndMerge(ActsObjWriterToolCfg(flags,
                                                     OutputDirectory = "obj",
                                                     SubDetectors = subDetectors) )

    result.addEventAlgo(CompFactory.ActsWriteTrackingGeometry(name, **kwargs))
    return result


def ActsMaterialMappingCfg(flags, name = "ActsMaterialMapping", **kwargs):
    result = ComponentAccumulator()

    kwargs["MaterialStepConverterTool"] = result.getPrimaryAndMerge(ActsMaterialStepConverterToolCfg(flags))
    kwargs["SurfaceMappingTool"] = result.getPrimaryAndMerge(ActsSurfaceMappingToolCfg(flags))
    kwargs["VolumeMappingTool"] = result.getPrimaryAndMerge(ActsVolumeMappingToolCfg(flags))
    kwargs["MaterialJsonWriterTool"] = \
      result.getPrimaryAndMerge( ActsMaterialJsonWriterToolCfg(flags,
                                                               OutputFile = "material-maps.json",
                                                               processSensitives = False,
                                                               processNonMaterial = False) )

    result.addEventAlgo(CompFactory.ActsMaterialMapping(name, **kwargs))
    return result
