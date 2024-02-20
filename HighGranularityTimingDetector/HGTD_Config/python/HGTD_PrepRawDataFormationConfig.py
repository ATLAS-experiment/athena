# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def HGTD_ClusterMakerToolCfg(flags, name = "HGTD_ClusterMakerTool", **kwargs):
    """Configures a tool that forms HGTD clusters """
    acc = ComponentAccumulator()

    acc.setPrivateTools(CompFactory.HGTD_ClusterMakerTool(name, **kwargs))
    return acc

def SinglePadClusterToolCfg(flags, name = "SinglePadClusterTool", **kwargs):
    """Configures a tool that creates 1-to-1 HGTD clusters out of single pads """
    if flags.HGTD.Geometry.useGeoModelXml:
        from HGTD_GeoModelXml.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    else:
        from HGTD_GeoModel.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    acc = HGTD_ReadoutGeometryCfg(flags)

    kwargs.setdefault("ClusterMakerTool", acc.popToolsAndMerge(HGTD_ClusterMakerToolCfg(flags)))
    acc.setPrivateTools(CompFactory.HGTD.SinglePadClusterTool(name, **kwargs))
    return acc

def PadClusterizationCfg(flags, name = "PadClusterizationAlg", **kwargs):
    """Schedules a clusterization alg to produce HGTD_Clusters out of HGTD_RDOs """
    acc = ComponentAccumulator()

    kwargs.setdefault("ClusterizationTool", acc.popToolsAndMerge(SinglePadClusterToolCfg(flags)))
    kwargs.setdefault("RDOContainerName", "HGTD_RDOs")
    kwargs.setdefault("PRDContainerName", "HGTD_Clusters")

    # Monitor Execution Time
    if flags.HGTD.doMonitoring and 'MonTool' not in kwargs:
        # Monitoring lives in ACTS since this is (will be) the same used for the ACTS clusterisation algorithm
        from ActsConfig.ActsMonitoringConfig import ActsHgtdClusterizationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsHgtdClusterizationMonitoringToolCfg(flags)))
        
    acc.addEventAlgo(CompFactory.HGTD.PadClusterizationAlg(name, **kwargs))
    return acc
