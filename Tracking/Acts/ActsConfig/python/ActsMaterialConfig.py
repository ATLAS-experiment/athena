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

def MaterialTrackWriterCfg(configFlags, name="MaterialTrackWriter", **kwargs) :
  from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
  from AthenaConfiguration.ComponentFactory import CompFactory
  acc = ComponentAccumulator()

  # Need geometry
  from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
  acc.merge( ActsTrackingGeometrySvcCfg(configFlags,
                                        RunConsistencyChecks=False,
                                        ObjDebugOutput=False))

  acc.addEventAlgo(CompFactory.ActsTrk.MaterialTrackWriter(name, **kwargs), primary = True)

  return acc

