# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthDeviceComps.AthDeviceCompsConfig import (
    HostMemoryResourceToolCfg,
    DeviceMemoryResourceToolCfg,
    CopyToolCfg,
    CopiesToolCfg,
)

# ============================================================
# Algorithm configurations
# ============================================================

def RDOtoTracccCellConverterAlgCfg(flags,
                                   name="RDOtoTracccCellConverterAlg",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("HostMR", acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)))
    kwargs.setdefault("DeviceMR", acc.popToolsAndMerge(DeviceMemoryResourceToolCfg(flags)))
    kwargs.setdefault("CopiesTool", acc.popToolsAndMerge(CopiesToolCfg(flags)))
    kwargs.setdefault("PixelRDO",    "ITkPixelRDOs")
    kwargs.setdefault("StripRDO",    "ITkStripRDOs")
    kwargs.setdefault("TracccCells", "TracccCells")
    kwargs.setdefault("HostConditionsObjectName", "TracccHostCondConfig")
    kwargs.setdefault("CPUCellSorting", not flags.Acts.Device.doCellSorting)
    kwargs.setdefault("UsePixelToTForCellActivation", not flags.Tracking.doPixelDigitalClustering)
    acc.addEventAlgo(
        CompFactory.ActsTrk.RDOtoTracccCellConverterAlg(name, **kwargs))
    return acc

def PhaseIIRDOtoTracccCellConverterAlgCfg(flags,
                                   name="PhaseIIRDOtoTracccCellConverterAlg",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("HostMR", acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)))
    kwargs.setdefault("DeviceMR", acc.popToolsAndMerge(DeviceMemoryResourceToolCfg(flags)))
    kwargs.setdefault("CopiesTool", acc.popToolsAndMerge(CopiesToolCfg(flags)))
    kwargs.setdefault("PixelRDO", "ITkPixelRDOs")
    kwargs.setdefault("StripRDO", "ITkStripRDOs")
    kwargs.setdefault("TracccCells", "TracccCellsFromPh2RDO")
    kwargs.setdefault("HostConditionsObjectName", "TracccHostCondConfig")
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
    kwargs.setdefault("InputMeasurements",   "TracccMeasurements")
    kwargs.setdefault("OutputPixelClusters", "ITkTracccPixelClusters")
    kwargs.setdefault("OutputStripClusters", "ITkTracccStripClusters")
    acc.addEventAlgo(
        CompFactory.ActsTrk.TracccMeasurementConverterAlg(name, **kwargs))
    return acc
