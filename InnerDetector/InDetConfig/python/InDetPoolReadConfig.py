# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def InDetPoolReadCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # Schedule reader algorithms for xAOD::SpacePoint collections
    from InDetConfig.InDetMeasurementsUtilitiesConfig import xAODSpacePointReaderAlgCfg
    typedCollections = flags.Input.TypedCollections
    for typedCollection in typedCollections:
        [colType, colName] = typedCollection.split('#')
        
        # Space Point Collections
        if colType == "xAOD::SpacePointContainer":
            acc.merge(xAODSpacePointReaderAlgCfg(flags,
                                                 name=f"{colName}ReaderAlg",
                                                 SpacePointKey=colName))
            continue
        
    return acc
