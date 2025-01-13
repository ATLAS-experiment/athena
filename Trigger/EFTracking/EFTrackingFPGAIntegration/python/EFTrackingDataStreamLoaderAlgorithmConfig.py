# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

# Todo: Move this helper algorithm to pyAthena
def EFTrackingDataStreamLoaderAlgorithmCfg(flags, **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    kwargs.setdefault("bufferSize", 8192)
    kwargs.setdefault("ignoreIsolatedLineFeed", False)

    from AthenaConfiguration.ComponentFactory import CompFactory 
    acc.addEventAlgo(CompFactory.EFTrackingDataStreamLoaderAlgorithm("EFTrackingDataStreamLoaderAlgorithm", **kwargs))

    return acc

