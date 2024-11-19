# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from ISF_Algorithms.CollectionMergerConfig import CollectionMergerCfg


def TRTSensitiveDetectorCfg(flags, name="TRTSensitiveDetector", **kwargs):
    bare_collection_name = "TRTUncompressedHits"
    mergeable_collection_suffix = "_G4"
    merger_input_property = "TRTUncompressedHits"
    region = "ID"

    acc, hits_collection_name = CollectionMergerCfg(flags,
                                                    bare_collection_name,
                                                    mergeable_collection_suffix,
                                                    merger_input_property,
                                                    region)

    logicalVolumeNames = ["TRT::Gas"]
    if flags.GeoModel.Run in [LHCPeriod.Run2] or flags.GeoModel.SQLiteDB:
        logicalVolumeNames += ["TRT::GasMA"]
    if flags.GeoModel.Run in [LHCPeriod.Run2, LHCPeriod.Run3] and not flags.GeoModel.SQLiteDB:
        logicalVolumeNames += ["TRT::Gas_Ar", "TRT::GasMA_Ar"]
        # In the case that Krypton is used to fill some volumes then
        # logicalVolumeNames += ["TRT::Gas_Kr", TRT::GasMA_Kr"]
    kwargs.setdefault("LogicalVolumeNames", logicalVolumeNames)
    kwargs.setdefault("OutputCollectionNames", [hits_collection_name])

    result = ComponentAccumulator()
    result.merge(acc)
    result.setPrivateTools(CompFactory.TRTSensitiveDetectorTool(name, **kwargs))
    return result


def TRTSensitiveDetector_CTBCfg(flags, name="TRTSensitiveDetector_CTB", **kwargs):
    kwargs.setdefault("LogicalVolumeNames", ["TRT::GasMA"])
    kwargs.setdefault("OutputCollectionNames", ["TRTUncompressedHits"])
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.TRTSensitiveDetectorTool(name, **kwargs))
    return result
