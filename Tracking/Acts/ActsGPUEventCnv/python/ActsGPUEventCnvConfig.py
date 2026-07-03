# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# ============================================================
# Algorithm configurations
# ============================================================

def RDOtoTracccCellConverterAlgCfg(flags,
                                   name="RDOtoTracccCellConverterAlg",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("PixelRDO",    "ITkPixelRDOs")
    kwargs.setdefault("StripRDO",    "ITkStripRDOs")
    kwargs.setdefault("TracccCells", "TracccCells")
    kwargs.setdefault("HostConditionsObjectName", "TracccHostCondConfig")
    acc.addEventAlgo(
        CompFactory.ActsTrk.RDOtoTracccCellConverterAlg(name, **kwargs))
    return acc

def TracccMeasurementConverterAlgCfg(flags,
                                     name="TracccMeasurementConverterAlg",
                                     **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("InputMeasurements",   "TracccMeasurements")
    kwargs.setdefault("OutputPixelClusters", "ITkTracccPixelClusters")
    kwargs.setdefault("OutputStripClusters", "ITkTracccStripClusters")
    acc.addEventAlgo(
        CompFactory.ActsTrk.TracccMeasurementConverterAlg(name, **kwargs))
    return acc
