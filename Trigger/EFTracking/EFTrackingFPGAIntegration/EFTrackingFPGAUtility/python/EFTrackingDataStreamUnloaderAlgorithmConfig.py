# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

# Todo: Move this helper algorithm to pyAthena
def EFTrackingDataStreamUnloaderAlgorithmCfg(flags, **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    kwargs.setdefault("name", "EFTrackingDataStreamUnloaderAlgorithm")

    from AthenaConfiguration.ComponentFactory import CompFactory 
    acc.addEventAlgo(CompFactory.EFTrackingDataStreamUnloaderAlgorithm(**kwargs))

    return acc

