# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of SiSpacePointFormation and SiSpacePointTool packages

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

def InDetToXAODSpacePointConversionCfg(flags,
                                       name: str = "InDetToXAODSpacePointConversion",
                                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('ProcessPixel', flags.Detector.EnableITkPixel)
    kwargs.setdefault('ProcessStrip', flags.Detector.EnableITkStrip and not flags.Tracking.doITkFastTracking)
    acc.addEventAlgo( CompFactory.InDet.InDetToXAODSpacePointConversion(name, **kwargs) )
    return acc

def IDInDetToXAODSpacePointConversionCfg(flags,
                                       name: str = "IDInDetToXAODSpacePointConversion",
                                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('ProcessPixel', flags.Detector.EnablePixel)
    kwargs.setdefault('ProcessStrip', flags.Detector.EnableSCT)

    kwargs.setdefault('PixelDetEleCollKey', "PixelDetectorElementCollection")
    kwargs.setdefault('StripDetEleCollKey', "SCT_DetectorElementCollection")

    kwargs.setdefault("InputPixelSpacePointsName", "PixelSpacePoints")
    kwargs.setdefault("InputStripSpacePointsName", "SCT_SpacePoints")
    kwargs.setdefault("InputStripOverlapSpacePointsName", "OverlapSpacePoints")

    kwargs.setdefault("OutputPixelClustersName", "PixelClustersSP")
    kwargs.setdefault("OutputStripClustersName", "SCT_ClustersSP")

    kwargs.setdefault("OutputPixelSpacePointsName", "PixelSpacePoints")
    kwargs.setdefault("OutputStripSpacePointsName", "SCT_SpacePoints")
    kwargs.setdefault("OutputStripOverlapSpacePointsName", "OverlapSpacePoints")

    kwargs.setdefault('PixelDetEleCollKey', "PixelDetectorElementCollection")
    kwargs.setdefault('StripDetEleCollKey', "SCT_DetectorElementCollection")

    acc.addEventAlgo( CompFactory.InDet.InDetToXAODSpacePointConversion(name, **kwargs) )
    return acc

def InDetSiElementPropertiesTableCondAlgCfg(
        flags, name="InDetSiElementPropertiesTableCondAlg", **kwargs):
    # For SCT DetectorElementCollection used
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc = SCT_ReadoutGeometryCfg(flags)

    acc.addCondAlgo(
        CompFactory.InDet.SiElementPropertiesTableCondAlg(name, **kwargs))
    return acc


def ITkSiElementPropertiesTableCondAlgCfg(
        flags, name="ITkSiElementPropertiesTableCondAlg", **kwargs):
    # For strip DetectorElementCollection used
    from StripGeoModelXml.ITkStripGeoModelConfig import (
        ITkStripReadoutGeometryCfg)
    acc = ITkStripReadoutGeometryCfg(flags)

    kwargs.setdefault("ReadKey", "ITkStripDetectorElementCollection")
    kwargs.setdefault("WriteKey", "ITkStripElementPropertiesTable")
    kwargs.setdefault("doEndcapEtaNeighbour", flags.Tracking.doEndcapEtaOverlapSpacePoint)

    acc.addCondAlgo(
        CompFactory.InDet.SiElementPropertiesTableCondAlg(name, **kwargs))
    return acc


def SiSpacePointMakerToolCfg(
        flags, name="InDetSiSpacePointMakerTool", **kwargs):
    acc = ComponentAccumulator()
    if flags.Beam.Type is BeamType.Cosmics or flags.Tracking.doBeamGas:
        kwargs.setdefault("StripLengthTolerance", 0.05)
    acc.setPrivateTools(
        CompFactory.InDet.SiSpacePointMakerTool(name, **kwargs))
    return acc


def ITkSiSpacePointMakerToolCfg(
        flags, name="ITkSiSpacePointMakerTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("SCTGapParameter", 0.0015)
    acc.setPrivateTools(
        CompFactory.InDet.SiSpacePointMakerTool(name, **kwargs))
    return acc


def InDetSiTrackerSpacePointFinderCfg(
        flags, name="InDetSiTrackerSpacePointFinder", **kwargs):
    # For SCT DetectorElementCollection used
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc = SCT_ReadoutGeometryCfg(flags)

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))
    acc.merge(InDetSiElementPropertiesTableCondAlgCfg(flags))

    kwargs.setdefault("SiSpacePointMakerTool", acc.popToolsAndMerge(
        SiSpacePointMakerToolCfg(flags)))
    kwargs.setdefault("PixelsClustersName", 'PixelClusters')
    kwargs.setdefault("SCT_ClustersName", 'SCT_Clusters')
    kwargs.setdefault("SpacePointsPixelName", 'PixelSpacePoints')
    kwargs.setdefault("SpacePointsSCTName", 'SCT_SpacePoints')
    kwargs.setdefault("SpacePointsOverlapName", 'OverlapSpacePoints')
    kwargs.setdefault("ProcessPixels", flags.Detector.EnablePixel)
    kwargs.setdefault("ProcessSCTs", flags.Detector.EnableSCT)
    kwargs.setdefault("ProcessOverlaps", flags.Detector.EnableSCT)

    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("ProcessOverlaps", False)
        kwargs.setdefault("OverrideBeamSpot", True)
        kwargs.setdefault("VertexZ", 0)
        kwargs.setdefault("VertexX", 0)
        kwargs.setdefault("VertexY", 99999999)
        kwargs.setdefault("OverlapLimitOpposite", 5)

    acc.addEventAlgo(
        CompFactory.InDet.SiTrackerSpacePointFinder(name, **kwargs))
    return acc


def TrigSiTrackerSpacePointFinderCfg(
        flags, name="InDetTrigSiTrackerSpacePointFinder", **kwargs):
    # For SCT DetectorElementCollection used
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc = SCT_ReadoutGeometryCfg(flags)

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))
    acc.merge(InDetSiElementPropertiesTableCondAlgCfg(flags))

    kwargs.setdefault("SiSpacePointMakerTool", acc.popToolsAndMerge(
        SiSpacePointMakerToolCfg(flags)))
    kwargs.setdefault("PixelsClustersName", 'PixelTrigClusters')
    kwargs.setdefault("SCT_ClustersName", 'SCT_TrigClusters')
    kwargs.setdefault("SpacePointsPixelName", 'PixelTrigSpacePoints')
    kwargs.setdefault("SpacePointsSCTName", 'SCT_TrigSpacePoints')
    kwargs.setdefault("SpacePointsOverlapName", 'OverlapSpacePoints')
    kwargs.setdefault("ProcessPixels", flags.Detector.EnablePixel)
    kwargs.setdefault("ProcessSCTs", flags.Detector.EnableSCT)
    kwargs.setdefault("ProcessOverlaps", flags.Detector.EnableSCT)
    kwargs.setdefault("SpacePointCacheSCT", flags.Trigger.InDetTracking.SpacePointCacheSCT)
    kwargs.setdefault("SpacePointCachePix", flags.Trigger.InDetTracking.SpacePointCachePix)

    acc.addEventAlgo(
        CompFactory.InDet.SiTrackerSpacePointFinder(name, **kwargs))
    return acc


def ITkSiTrackerSpacePointFinderCfg(
        flags, name="ITkSiTrackerSpacePointFinder", **kwargs):
    # For strip DetectorElementCollection used
    from StripGeoModelXml.ITkStripGeoModelConfig import (
        ITkStripReadoutGeometryCfg)
    acc = ITkStripReadoutGeometryCfg(flags)

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))
    acc.merge(ITkSiElementPropertiesTableCondAlgCfg(flags))

    kwargs.setdefault("SiSpacePointMakerTool", acc.popToolsAndMerge(
        ITkSiSpacePointMakerToolCfg(flags)))
    kwargs.setdefault("PixelsClustersName", 'ITkPixelClusters')
    kwargs.setdefault("SCT_ClustersName", 'ITkStripClusters')
    kwargs.setdefault("SCTPropertiesKey", "ITkStripElementPropertiesTable")
    kwargs.setdefault("SCTDetEleCollKey", "ITkStripDetectorElementCollection")
    kwargs.setdefault("SpacePointsPixelName", 'ITkPixelSpacePoints')
    kwargs.setdefault("SpacePointsSCTName", 'ITkStripSpacePoints')
    kwargs.setdefault("SpacePointsOverlapName", 'ITkOverlapSpacePoints')
    kwargs.setdefault("ProcessPixels", flags.Detector.EnableITkPixel)
    kwargs.setdefault("ProcessSCTs", flags.Detector.EnableITkStrip)
    kwargs.setdefault("ProcessOverlaps", flags.Detector.EnableITkStrip)
    kwargs.setdefault("useSCTLayerDep_OverlapCuts", False)
    kwargs.setdefault("doEndcapEtaOverlapSP",flags.Tracking.doEndcapEtaOverlapSpacePoint)

    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("ProcessOverlaps", False)
        kwargs.setdefault("OverrideBeamSpot", True)
        kwargs.setdefault("VertexZ", 0)
        kwargs.setdefault("VertexX", 0)
        kwargs.setdefault("VertexY", 99999999)
        kwargs.setdefault("OverlapLimitOpposite", 5)

    acc.addEventAlgo(
        CompFactory.InDet.SiTrackerSpacePointFinder(name, **kwargs))
    return acc


def ITkPixelSiTrackerSpacePointFinderCfg(
        flags, name="ITkPixelSiTrackerSpacePointFinder", **kwargs):
    kwargs.setdefault("ProcessSCTs", False)
    kwargs.setdefault("ProcessOverlaps", False)
    return ITkSiTrackerSpacePointFinderCfg(flags, name, **kwargs)


def ITkStripSiTrackerSpacePointFinderCfg(
        flags, name="ITkStripSiTrackerSpacePointFinder", **kwargs):
    kwargs.setdefault("ProcessPixels", False)
    return ITkSiTrackerSpacePointFinderCfg(flags, name, **kwargs)


def ITkFastSiTrackerSpacePointFinderCfg(flags):
    acc = ITkPixelSiTrackerSpacePointFinderCfg(flags)
    if flags.Tracking.doLargeD0:
        acc.merge(ITkStripSiTrackerSpacePointFinderCfg(flags))
    return acc


def ITkTrigSiTrackerSpacePointFinderCfg(
        flags, name="ITkTrigSiTrackerSpacePointFinder", signature="", **kwargs):
    kwargs.setdefault("PixelsClustersName", "ITkTrigPixelClusters")
    kwargs.setdefault("SCT_ClustersName", "ITkTrigStripClusters")
    kwargs.setdefault("SpacePointsPixelName", "ITkPixelTrigSpacePoints")
    kwargs.setdefault("SpacePointsSCTName", "ITkStripTrigSpacePoints")
    return ITkSiTrackerSpacePointFinderCfg(flags, name+signature, **kwargs)
