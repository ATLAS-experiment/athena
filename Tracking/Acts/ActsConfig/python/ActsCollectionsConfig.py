# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SpacePointReaderCfg(flags,
                        name: str,
                        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.ActsTrk.SpacePointReader(name=name, **kwargs))
    return acc

def ActsPoolReadCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    import re
    typedCollections = flags.Input.TypedCollections
    for col in typedCollections:
        match = re.findall(r"xAOD::SpacePointContainer#(\S+)", col)
        if match:
            spCol = match[0]
            acc.merge(SpacePointReaderCfg(flags,
                                          name=f"{spCol}Reader",
                                          SpacePointKey=spCol))

    return acc
