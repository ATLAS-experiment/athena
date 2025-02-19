# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ISF_Algorithms.CollectionMergerConfig import CollectionMergerCfg


def HgtdSensorSDCfg(flags, name="HgtdSensorSD", **kwargs):
    bare_collection_name = "HGTD_Hits"
    mergeable_collection_suffix = "_G4"
    merger_input_property = "HGTDHits"
    region = "ITk"

    acc, hits_collection_name = CollectionMergerCfg(flags,
                                                    bare_collection_name,
                                                    mergeable_collection_suffix,
                                                    merger_input_property,
                                                    region)
                                                    
    if flags.HGTD.Geometry.useGeoModelXml:
        kwargs.setdefault("GmxSensor", True)
        kwargs.setdefault("LogicalVolumeNames", ["HGTD::HGTDSiSensor","HGTD::HGTDSiSensorPosL0","HGTD::HGTDSiSensorPosL1","HGTD::HGTDSiSensorPosL2","HGTD::HGTDSiSensorPosL3","HGTD::HGTDSiSensorNegL0","HGTD::HGTDSiSensorNegL1","HGTD::HGTDSiSensorNegL2","HGTD::HGTDSiSensorNegL3"])    
    else:
        kwargs.setdefault("LogicalVolumeNames", ["HGTD::HGTDSiSensor0", "HGTD::HGTDSiSensor1", "HGTD::HGTDSiSensor2", "HGTD::HGTDSiSensor3"])

    kwargs.setdefault("OutputCollectionNames", [hits_collection_name])

    result = ComponentAccumulator()
    result.merge(acc)
    result.setPrivateTools(CompFactory.HGTDSensorSDTool(name, **kwargs))
    return result
