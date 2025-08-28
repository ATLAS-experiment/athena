# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

def TestVectorGeneratorCfg(flags, **kwargs):
    kwargs.setdefault("name", "TestVectorGenerator")
    kwargs.setdefault("bufferSize", 65536)

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory 
    acc.addEventAlgo(CompFactory.EFTrackingFPGAUtility.TestVectorGenerator(**kwargs))

    return acc

