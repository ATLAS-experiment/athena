# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_SeededTrackFinderTool package

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

def TRT_SeededTrackFinder_ATLCfg(
        flags, name='InDetTRT_SeededTrackMaker', InputCollections=[], **kwargs):
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc = AtlasFieldCacheCondAlgCfg(flags)

    #
    # --- TRT seeded back tracking tool
    #
    if "PropagatorTool" not in kwargs:
        from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
        kwargs.setdefault("PropagatorTool", acc.popToolsAndMerge(
            RungeKuttaPropagatorCfg(flags)))

    if "UpdatorTool" not in kwargs:
        from TrkConfig.TrkMeasurementUpdatorConfig import KalmanUpdator_xkCfg
        kwargs.setdefault("UpdatorTool", acc.popToolsAndMerge(
            KalmanUpdator_xkCfg(flags)))

    if "CombinatorialTrackFinder" not in kwargs:
        from InDetConfig.SiCombinatorialTrackFinderToolConfig import (
            SiCombinatorialTrackFinder_xkCfg)
        kwargs.setdefault("CombinatorialTrackFinder", acc.popToolsAndMerge(
            SiCombinatorialTrackFinder_xkCfg(flags)))

    if ("RoadTool" not in kwargs and
        flags.Tracking.ActiveConfig.usePixel and flags.Tracking.ActiveConfig.useSCT):
        from InDetConfig.SiDetElementsRoadToolConfig import (
            SiDetElementsRoadMaker_xk_TRT_Cfg)
        kwargs.setdefault("RoadTool", acc.popToolsAndMerge(
            SiDetElementsRoadMaker_xk_TRT_Cfg(flags)))

    #
    # --- decide which TRT seed space point finder to use
    #
    if "SeedTool" not in kwargs:
        from InDetConfig.TRT_SeededSpacePointFinderToolConfig import (
            TRT_SeededSpacePointFinder_ATLCfg)
        kwargs.setdefault("SeedTool", acc.popToolsAndMerge(
            TRT_SeededSpacePointFinder_ATLCfg(flags, InputCollections=InputCollections)))

    kwargs.setdefault("pTmin", flags.Tracking.BackTracking.minPt)
    kwargs.setdefault("nHolesMax", flags.Tracking.BackTracking.nHolesMax)
    kwargs.setdefault("nHolesGapMax", flags.Tracking.BackTracking.nHolesGapMax)
    kwargs.setdefault("SearchInCaloROI", False)
    if kwargs["SearchInCaloROI"]:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            CaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(CaloClusterROIPhiRZContainerMakerCfg(flags))
        kwargs.setdefault("EMROIPhiRZContainer","InDetCaloClusterROIPhiRZ12GeV")
    else:
        kwargs.setdefault("EMROIPhiRZContainer","")
    kwargs.setdefault("ConsistentSeeds", True)
    kwargs.setdefault("BremCorrection", False)

    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("nWClustersMin", 0)

    acc.setPrivateTools(
        CompFactory.InDet.TRT_SeededTrackFinder_ATL(name, **kwargs))
    return acc
