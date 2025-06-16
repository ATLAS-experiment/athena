# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

def MasqueradeCoordinatesCfg(flags, **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    kwargs.setdefault("name", "MasqueradeCoordinates")
    kwargs.setdefault("cylindricalDataStream", "cylindricalDataStream")
    kwargs.setdefault("cartesianDataStream", "cartesianDataStream")

    from AthenaConfiguration.ComponentFactory import CompFactory 
    MasqueradeCoordinates = CompFactory.MasqueradeCoordinates(**kwargs)

    acc.addEventAlgo(MasqueradeCoordinates)

    return acc

