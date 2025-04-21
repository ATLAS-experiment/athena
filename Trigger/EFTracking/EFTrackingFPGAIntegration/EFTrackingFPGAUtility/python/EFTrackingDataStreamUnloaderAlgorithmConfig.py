# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

# Todo: Move this helper algorithm to pyAthena
def EFTrackingDataStreamUnloaderAlgorithmCfg(flags, **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory 
    acc.addEventAlgo(CompFactory.EFTrackingDataStreamUnloaderAlgorithm("EFTrackingDataStreamUnloaderAlgorithm", **kwargs))

    return acc

