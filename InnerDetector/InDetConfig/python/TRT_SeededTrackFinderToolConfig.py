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
    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
    InDetPatternPropagator = acc.popToolsAndMerge(
        RungeKuttaPropagatorCfg(flags, name="InDetPatternPropagator"))
    acc.addPublicTool(InDetPatternPropagator)
    kwargs.setdefault("PropagatorTool", InDetPatternPropagator)

    from TrkConfig.TrkMeasurementUpdatorConfig import KalmanUpdator_xkCfg
    InDetPatternUpdator = acc.popToolsAndMerge(
        KalmanUpdator_xkCfg(flags, name="InDetPatternUpdator"))
    acc.addPublicTool(InDetPatternUpdator)
    kwargs.setdefault("UpdatorTool", InDetPatternUpdator)

    from InDetConfig.SiCombinatorialTrackFinderToolConfig import (
        SiCombinatorialTrackFinder_xkCfg)
    kwargs.setdefault("CombinatorialTrackFinder", acc.popToolsAndMerge(
        SiCombinatorialTrackFinder_xkCfg(flags)))

    if (flags.Tracking.ActiveConfig.usePixel and
        flags.Tracking.ActiveConfig.useSCT):
        from InDetConfig.SiDetElementsRoadToolConfig import (
            SiDetElementsRoadMaker_xk_TRT_Cfg)
        InDetTRT_SeededSiRoadMaker = acc.popToolsAndMerge(
            SiDetElementsRoadMaker_xk_TRT_Cfg(flags))
        acc.addPublicTool(InDetTRT_SeededSiRoadMaker)
        kwargs.setdefault("RoadTool", InDetTRT_SeededSiRoadMaker)

    #
    # --- decide which TRT seed space point finder to use
    #
    from InDetConfig.TRT_SeededSpacePointFinderToolConfig import (
        TRT_SeededSpacePointFinder_ATLCfg)
    InDetTRT_SeededSpacePointFinder = acc.popToolsAndMerge(
        TRT_SeededSpacePointFinder_ATLCfg(flags, InputCollections=InputCollections))
    acc.addPublicTool(InDetTRT_SeededSpacePointFinder)
    kwargs.setdefault("SeedTool", InDetTRT_SeededSpacePointFinder)

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
