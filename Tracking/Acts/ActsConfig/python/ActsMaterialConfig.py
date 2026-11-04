#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

def MaterialTrackRecorderCfg(configFlags, name="ActsTrk::UserActionSvc.MaterialTrackRecorderTool", **kwargs):
  from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
  from AthenaConfiguration.ComponentFactory import CompFactory
  acc = ComponentAccumulator()
  acc.setPrivateTools(CompFactory.ActsTrk.MaterialTrackRecorderTool(name, **kwargs))
  return acc


def MaterialTrackRecorderUserActionSvcCfg(configFlags, name="ActsTrk::MaterialTrackRecorderUserActionSvc", **kwargs):
  from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
  from AthenaConfiguration.ComponentFactory import CompFactory

  acc = ComponentAccumulator()

  #Setting up the CA for the MaterialStepRecorder
  actionAcc = ComponentAccumulator()
  actions = []
  actions += [actionAcc.popToolsAndMerge(MaterialTrackRecorderCfg(configFlags))]
  actionAcc.setPrivateTools(actions)
  MaterialTrackRecorderAction = acc.popToolsAndMerge(actionAcc)

  #Retrieving the default action list
  from G4AtlasServices.G4AtlasUserActionConfig import getDefaultActions
  defaultActions = acc.popToolsAndMerge(getDefaultActions(configFlags))

  #Adding material recorder action to defaults
  actionList = (defaultActions + MaterialTrackRecorderAction)

  kwargs.setdefault("UserActionTools",actionList)
  acc.addService(CompFactory.G4UA.UserActionSvc(name,**kwargs), primary = True)

  return acc

def MaterialTrackWriterCfg(configFlags,
                           name="MaterialTrackWriter",
                           FileName="material-tracks.root",
                           OutStream="ACTSMATERIALWRITER",
                           **kwargs) :
  from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
  from AthenaConfiguration.ComponentFactory import CompFactory
  acc = ComponentAccumulator()
  kwargs.setdefault("OutStream", OutStream)
  kwargs.setdefault("useTrackingGeometry", True)
  
  from MuonConfig.MuonConfigUtils import setupHistSvcCfg
  acc.merge(setupHistSvcCfg(configFlags, outFile = FileName, outStream=kwargs["OutStream"]))

  acc.addEventAlgo(CompFactory.ActsTrk.MaterialTrackWriter(name, **kwargs), primary = True)

  return acc

def MaterialTrackReaderCfg(configFlags, name="MaterialTrackReader", **kwargs) :
  from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
  from AthenaConfiguration.ComponentFactory import CompFactory
  acc = ComponentAccumulator()

  acc.addEventAlgo(CompFactory.ActsTrk.MaterialTrackReader(name, **kwargs), primary = True)

  return acc

def MaterialMappingCfg(configFlags,
                       name="MaterialMapping",
                       StoreTracks=False,
                       StoreSurfInfo=False,
                       OutputMappedMaterialTracks="OuputMappedMaterialTracks",
                       OutputUnmappedMaterialTracks="OutputUnmappedMaterialTracks",
                       **kwargs) :

  from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
  from AthenaConfiguration.ComponentFactory import CompFactory
  acc = ComponentAccumulator()

  # Need geometry
  from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
  acc.merge( ActsTrackingGeometrySvcCfg(configFlags))
  from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
  acc.merge(AtlasFieldCacheCondAlgCfg(configFlags))
  from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
  acc.merge(ActsGeometryContextAlgCfg(configFlags))

  kwargs.setdefault("MappedMaterialTrackCollectionKey", OutputMappedMaterialTracks)
  kwargs.setdefault("UnmappedMaterialTrackCollectionKey", OutputUnmappedMaterialTracks)

  acc.addEventAlgo(CompFactory.ActsTrk.MaterialMapping(name, **kwargs), primary = True)

  if StoreTracks:
      from ActsConfig.ActsMaterialConfig import MaterialTrackWriterCfg
      acc.merge(MaterialTrackWriterCfg(configFlags,
                                       name="MappedMaterialTrackWriter",
                                       FileName="material-tracks-mapped.root",
                                       OutStream="ACTSMAPPEDMATERIALWRITER",
                                       StoreSurface=StoreSurfInfo,
                                       MaterialTrackCollectionKey=OutputMappedMaterialTracks))
      acc.merge(MaterialTrackWriterCfg(configFlags,
                                       name="UnmappedMaterialTrackWriter",
                                       FileName="material-tracks-unmapped.root",
                                       OutStream="ACTSUNMAPPEDMATERIALWRITER",
                                       StoreSurface=StoreSurfInfo,
                                       MaterialTrackCollectionKey=OutputUnmappedMaterialTracks))

  return acc


def MaterialJsonDumpCfg(configFlags,
                        name="MaterialJsonDump",
                        FileName="material-maps.json.zst",
                        **kwargs) :
  """Dump the material assigned to the tracking geometry (e.g. loaded from a
  Root material map by the ITkMaterialDecoratorTool) with the ACTS material
  JSON codec. FileName includes the extension and defaults to .json.zst.
  Codec options can be supplied through kwargs."""
  from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
  from AthenaConfiguration.ComponentFactory import CompFactory
  acc = ComponentAccumulator()

  # Need geometry
  from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
  acc.merge( ActsTrackingGeometrySvcCfg(configFlags))
  from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
  acc.merge(AtlasFieldCacheCondAlgCfg(configFlags))
  from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
  acc.merge(ActsGeometryContextAlgCfg(configFlags))

  kwargs.setdefault("FileName", FileName)

  acc.addEventAlgo(CompFactory.ActsTrk.MaterialJsonDumpAlg(name, **kwargs), primary = True)

  return acc


def MaterialValidationCfg(configFlags,
                          StoreTracks=True,
                          OutputMaterialTracks="OutputMaterialTracks",
                          name="MaterialValidation", **kwargs) :
  from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
  from AthenaConfiguration.ComponentFactory import CompFactory
  acc = ComponentAccumulator()

  # Need geometry
  from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
  acc.merge( ActsTrackingGeometrySvcCfg(configFlags))
  from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
  acc.merge(AtlasFieldCacheCondAlgCfg(configFlags))
  from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
  acc.merge(ActsGeometryContextAlgCfg(configFlags))

  kwargs.setdefault("MaterialTrackCollectionKey", OutputMaterialTracks)

  acc.addEventAlgo(CompFactory.ActsTrk.MaterialValidation(name, **kwargs), primary = True)
  if StoreTracks:
      from ActsConfig.ActsMaterialConfig import MaterialTrackWriterCfg
      acc.merge(MaterialTrackWriterCfg(configFlags,
                                       FileName="material-tracks-validation.root",
                                       MaterialTrackCollectionKey=OutputMaterialTracks))


  return acc
