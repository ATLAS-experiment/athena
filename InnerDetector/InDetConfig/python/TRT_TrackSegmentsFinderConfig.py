# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_TrackSegmentsFinder package
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
import AthenaCommon.SystemOfUnits as Units

def TRT_TrackSegmentsFinderCfg(
        flags, name = 'InDetTRT_TrackSegmentsFinder',
        InputCollections = None, **kwargs):

    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = AtlasFieldCacheCondAlgCfg(flags)

    if "SegmentsMakerTool" not in kwargs:
        from InDetConfig.TRT_TrackSegmentsToolConfig import (
            TRT_TrackSegmentsMaker_ATLxkCfg)
        InDetTRT_TrackSegmentsMaker = acc.popToolsAndMerge(
            TRT_TrackSegmentsMaker_ATLxkCfg(flags,
                                            InputCollections = InputCollections))
        kwargs.setdefault("SegmentsMakerTool", InDetTRT_TrackSegmentsMaker)

    if "RoadTool" not in kwargs:
        from InDetConfig.TRT_DetElementsRoadToolConfig import (
            TRT_DetElementsRoadMaker_xkCfg)
        kwargs.setdefault("RoadTool", acc.popToolsAndMerge(
            TRT_DetElementsRoadMaker_xkCfg(flags)))

    if flags.Tracking.BackTracking.doRoISeeded:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            CaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(CaloClusterROIPhiRZContainerMakerCfg(flags))
        kwargs.setdefault("useCaloSeeds", True)
        kwargs.setdefault("EMROIPhiRZContainer", (
            "InDetCaloClusterROIPhiRZ%.0fGeVBackTracking" %
            (flags.Tracking.BackTracking.minRoIClusterEt/Units.GeV)))

    kwargs.setdefault("SegmentsLocation", "TRTSegments")

    acc.addEventAlgo(CompFactory.InDet.TRT_TrackSegmentsFinder(name, **kwargs))
    return acc

def TRT_TrackSegmentsFinder_Cosmics_Cfg(
        flags, name = 'InDetTRT_TrackSegmentsFinder_Cosmics', **kwargs):
    acc = ComponentAccumulator()

    if "SegmentsMakerTool" not in kwargs:
        from InDetConfig.TRT_TrackSegmentsToolConfig import (
            TRT_TrackSegmentsMaker_BarrelCosmicsCfg)
        kwargs.setdefault("SegmentsMakerTool", acc.popToolsAndMerge(
            TRT_TrackSegmentsMaker_BarrelCosmicsCfg(flags)))

    acc.merge(TRT_TrackSegmentsFinderCfg(flags, name, **kwargs))
    return acc

def TRT_TrackSegmentsFinder_Phase_Cfg(
        flags, name = 'InDetTRT_TrackSegmentsFinder_Phase', **kwargs):
    acc = ComponentAccumulator()

    if "SegmentsMakerTool" not in kwargs:
        from InDetConfig.TRT_TrackSegmentsToolConfig import (
            TRT_TrackSegmentsMaker_ATLxk_Phase_Cfg)
        kwargs.setdefault("SegmentsMakerTool", acc.popToolsAndMerge(
            TRT_TrackSegmentsMaker_ATLxk_Phase_Cfg(flags)))

    kwargs.setdefault("SegmentsLocation", "TRTSegments_Phase")

    acc.merge(TRT_TrackSegmentsFinderCfg(flags, name, **kwargs))
    return acc

def TRT_TrackSegmentsFinder_TrackSegments_Cfg(
        flags, name = 'InDetTRT_TrackSegmentsFinder_TrackSegments', **kwargs):
    acc = ComponentAccumulator()

    if "SegmentsMakerTool" not in kwargs:
        from InDetConfig.TRT_TrackSegmentsToolConfig import (
            TRT_TrackSegmentsMaker_ATLxk_TrackSegmentsCfg)
        kwargs.setdefault("SegmentsMakerTool", acc.popToolsAndMerge(
            TRT_TrackSegmentsMaker_ATLxk_TrackSegmentsCfg(flags)))

    kwargs.setdefault("SegmentsLocation", "TRTSegmentsTRT")

    acc.merge(TRT_TrackSegmentsFinderCfg(flags, name, **kwargs))
    return acc
