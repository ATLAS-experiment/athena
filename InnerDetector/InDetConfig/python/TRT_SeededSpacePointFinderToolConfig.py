# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_SeededSpacePointFinderTool package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

def SimpleTRT_SeededSpacePointFinder_ATLCfg(flags, name='InDetTRT_SeededSpFinder', InputCollections=None, **kwargs):
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc = AtlasFieldCacheCondAlgCfg(flags)

    kwargs.setdefault("SpacePointsSCTName", 'SCT_SpacePoints')
    kwargs.setdefault("PRDtoTrackMap",
                      'InDetSegmentPRDtoTrackMap' if InputCollections is not None else "")
    kwargs.setdefault("PerigeeCut", 1000.)
    kwargs.setdefault("DirectionPhiCut", .3)
    kwargs.setdefault("MaxHoles", 2)
    kwargs.setdefault("RestrictROI", True)

    acc.setPrivateTools(
        CompFactory.InDet.SimpleTRT_SeededSpacePointFinder_ATL(name, **kwargs))
    return acc

def TRT_SeededSpacePointFinder_ATLCfg(flags, name='InDetTRT_SeededSpFinder', InputCollections=None, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("SpacePointsSCTName", 'SCT_SpacePoints')
    kwargs.setdefault("SpacePointsOverlapName", 'OverlapSpacePoints')
    kwargs.setdefault("PRDtoTrackMap",
                      'InDetSegmentPRDtoTrackMap' if InputCollections is not None else "")
    kwargs.setdefault("NeighborSearch", True)
    kwargs.setdefault("LoadFull", False)
    kwargs.setdefault("DoCosmics", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("pTmin", flags.Tracking.BackTracking.minPt)

    acc.setPrivateTools(
        CompFactory.InDet.TRT_SeededSpacePointFinder_ATL(name, **kwargs))
    return acc
