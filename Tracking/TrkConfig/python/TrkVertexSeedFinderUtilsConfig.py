# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVertexSeedFinderUtils package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def Trk2dDistanceSeederCfg(flags, name='Trk2dDistanceSeeder', **kwargs):
    # To produce AtlasFieldCacheCondObj
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = AtlasFieldCacheCondAlgCfg(flags)
    acc.setPrivateTools(CompFactory.Trk.Trk2dDistanceSeeder(name, **kwargs))
    return acc

def NewtonTrkDistanceFinderCfg(flags, name='NewtonTrkDistanceFinder', **kwargs):
    # To produce AtlasFieldCacheCondObj
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = AtlasFieldCacheCondAlgCfg(flags)
    acc.setPrivateTools(CompFactory.Trk.NewtonTrkDistanceFinder(name, **kwargs))
    return acc

def SeedNewtonTrkDistanceFinderCfg(flags, name='SeedNewtonTrkDistanceFinder', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("Trk2dDistanceSeeder", acc.addPublicTool(
        acc.popToolsAndMerge(Trk2dDistanceSeederCfg(flags))))
    kwargs.setdefault("TrkDistanceFinderImplementation", acc.addPublicTool(
        acc.popToolsAndMerge(NewtonTrkDistanceFinderCfg(flags))))

    acc.setPrivateTools(
        CompFactory.Trk.SeedNewtonTrkDistanceFinder(name, **kwargs))
    return acc


def Mode3dFromFsmw1dFinderCfg(flags, name='Mode3dFromFsmw1dFinder', **kwargs):

  acc = ComponentAccumulator()
  
  kwargs["MinimalDistanceFromZtoXY"] = 0.25

  acc.setPrivateTools(CompFactory.Trk.Mode3dFromFsmw1dFinder(name, **kwargs))
  return acc
