# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
        
def xAODSpacePointReaderAlgCfg(flags,
                               name: str,
                               **kwargs: dict) -> ComponentAccumulator:
    """ Read xAOD::SpacePoints from file and attach bare pointer to xAOD::UncalibratedMeasurements as a decoration """
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.InDet.SpacePointReader(name=name, **kwargs))
    return acc

