# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthDeviceComps.AthDeviceCompsConfig import (
    HostMemoryResourceToolCfg,
    DeviceMemoryResourceToolCfg,
    MemoryResourcesToolCfg,
    CopyToolCfg,
    CopiesToolCfg,
)
from AthenaConfiguration.Enums import BeamType

# ============================================================
# Algorithm configurations
# ============================================================

def RDOtoTracccCellConverterAlgCfg(flags,
                                   name="RDOtoTracccCellConverterAlg",
                                   **kwargs) -> ComponentAccumulator:
    #TODO: remove this once MC is fixed
    if not flags.Tracking.doPixelDigitalClustering:
        raise ValueError("clusterization on device is not compatible "
            "with analog clustering at the moment due to incorrent "
            "ToT values for Pixel hits in the simulation data.")

    if flags.ITk.selectStripIntimeHits and 'timeBins' not in kwargs:
        coll_25ns = flags.Beam.BunchSpacing<=25 and flags.Beam.Type is BeamType.Collisions
        kwargs.setdefault("timeBins", "01X" if coll_25ns else "X1X")

    acc = ComponentAccumulator()
    kwargs.setdefault("HostMR", acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)))
    kwargs.setdefault("DeviceMR", acc.popToolsAndMerge(DeviceMemoryResourceToolCfg(flags)))
    kwargs.setdefault("CopiesTool", acc.popToolsAndMerge(CopiesToolCfg(flags)))
    kwargs.setdefault("PixelRDO",    "ITkPixelRDOs")
    kwargs.setdefault("StripRDO",    "ITkStripRDOs")
    kwargs.setdefault("TracccCells", "TracccCells")
    kwargs.setdefault("HostConditionsObjectName", "TracccHostCondConfig")
    kwargs.setdefault("GeoIdMappingObjectName",     "TracccGeometryIdMapping")
    kwargs.setdefault("CPUCellSorting", not flags.Acts.Device.doCellSorting)
    kwargs.setdefault("UsePixelToTForCellActivation", not flags.Tracking.doPixelDigitalClustering)
    acc.addEventAlgo(
        CompFactory.ActsTrk.RDOtoTracccCellConverterAlg(name, **kwargs))
    return acc

def PhaseIIRDOtoTracccCellConverterAlgCfg(flags,
                                   name="PhaseIIRDOtoTracccCellConverterAlg",
                                   **kwargs) -> ComponentAccumulator:
    #TODO: remove this once MC is fixed
    if not flags.Tracking.doPixelDigitalClustering:
        raise ValueError("clusterization on device is not compatible "
            "with analog clustering at the moment due to incorrent "
            "ToT values for Pixel hits in the simulation data.")

    if flags.ITk.selectStripIntimeHits and 'timeBins' not in kwargs:
        coll_25ns = flags.Beam.BunchSpacing<=25 and flags.Beam.Type is BeamType.Collisions
        kwargs.setdefault("timeBins", "01X" if coll_25ns else "X1X")

    acc = ComponentAccumulator()
    kwargs.setdefault("HostMR", acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)))
    kwargs.setdefault("DeviceMR", acc.popToolsAndMerge(DeviceMemoryResourceToolCfg(flags)))
    kwargs.setdefault("CopiesTool", acc.popToolsAndMerge(CopiesToolCfg(flags)))
    kwargs.setdefault("PixelRDO", "ITkPixelRDOs")
    kwargs.setdefault("StripRDO", "ITkStripRDOs")
    kwargs.setdefault("TracccCells", "TracccCellsFromPh2RDO")
    kwargs.setdefault("HostConditionsObjectName", "TracccHostCondConfig")
    kwargs.setdefault("GeoIdMappingObjectName",     "TracccGeometryIdMapping")
    kwargs.setdefault("CPUCellSorting", not flags.Acts.Device.doCellSorting)
    kwargs.setdefault("UsePixelToTForCellActivation", not flags.Tracking.doPixelDigitalClustering)
    acc.addEventAlgo(
        CompFactory.ActsTrk.PhaseIIRDOtoTracccCellConverterAlg(name, **kwargs))
    return acc

def TracccCellValidationAlgCfg(flags,
        name="TracccCellValidationAlg",
        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("HostMR", acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)))
    kwargs.setdefault("DeviceCopyTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("ReferenceCells", "TracccCellsLegacy")
    kwargs.setdefault("Cells",          "TracccCells")
    acc.addEventAlgo(
        CompFactory.ActsTrk.TracccCellValidationAlg(name, **kwargs))
    return acc

def TracccMeasurementConverterAlgCfg(flags,
                                     name="TracccMeasurementConverterAlg",
                                     **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("HostMR", acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("InputMeasurements", "TracccMeasurements")
    kwargs.setdefault("InputClusters", "TracccClusterCollection")
    kwargs.setdefault("InputCells", "TracccCells")
    kwargs.setdefault("OutputPixelClusters", "ITkTracccPixelClusters")
    kwargs.setdefault("OutputStripClusters", "ITkTracccStripClusters")
    kwargs.setdefault("ConvertClustersWithCells", False)
    acc.addEventAlgo(
        CompFactory.ActsTrk.TracccMeasurementConverterAlg(name, **kwargs))
    return acc

def TracccSeedConverterAlgCfg(flags,
                                     name="TracccSeedConverterAlg",
                                     **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("HostMR", acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("InputSpacepoints", "TracccMeasurements")
    kwargs.setdefault("InputSeeds", "TracccClusterCollection")
    kwargs.setdefault("OutputSeeds", "ITkTracccSeeds")
    acc.addEventAlgo(
        CompFactory.ActsTrk.TracccSeedConverterAlg(name, **kwargs))
    return acc   

def TracccTrackConverterAlgCfg(flags,
                                     name="TracccTrackConverterAlg",
                                     **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("HostMR", acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("InputTracks", "TracccMeasurements")
    kwargs.setdefault("OutputTracks", "ITkTracccTracks")
    acc.addEventAlgo(
        CompFactory.ActsTrk.TracccTrackConverterAlg(name, **kwargs))
    return acc 

def xAODToTracccMeasurementConverterAlgCfg(flags,
                                           name="xAODToTracccMeasurementConverterAlg",
                                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("MemoryResourcesTool", acc.popToolsAndMerge(MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopiesTool", acc.popToolsAndMerge(CopiesToolCfg(flags)))
    kwargs.setdefault("InputPixelClusters", "ITkPixelClusters")
    kwargs.setdefault("InputStripClusters", "ITkStripClusters")
    kwargs.setdefault("OutputTracccMeasurements", "TracccMeasurementCollection")
    kwargs.setdefault("OutputMeasToPixelCluster", "TracccMeasToPixelCluster")
    kwargs.setdefault("OutputMeasToStripCluster", "TracccMeasToStripCluster")
    kwargs.setdefault("GeoIdMappingObjectName", "TracccGeometryIdMapping")
    kwargs.setdefault("HostConditionsObjectName", "TracccHostCondConfig")
    kwargs.setdefault("HostDigitizationObjectName", "TracccHostDigitizationConfig")
    acc.addEventAlgo(
        CompFactory.ActsTrk.xAODToTracccMeasurementConverterAlg(name, **kwargs))
    return acc

def xAODToTracccSpacePointConverterAlgCfg(flags,
                                          name="xAODToTracccSpacePointConverterAlg",
                                          **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("MemoryResourcesTool", acc.popToolsAndMerge(MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopiesTool", acc.popToolsAndMerge(CopiesToolCfg(flags)))
    kwargs.setdefault("InputSpacePoints", ["ITkPixelSpacePoints"])
    kwargs.setdefault("InputMeasToCluster", "TracccMeasToPixelCluster")
    kwargs.setdefault("OutputTracccSpacepoints", "TracccPixelSpacepointCollection")
    kwargs.setdefault("OutputSPToHostContainer", f"{kwargs['OutputTracccSpacepoints']}ToHostContainer")
    kwargs.setdefault("OutputSPToHostIndex", f"{kwargs['OutputTracccSpacepoints']}ToHostIndex")
    acc.addEventAlgo(
        CompFactory.ActsTrk.xAODToTracccSpacePointConverterAlg(name, **kwargs))
    return acc
