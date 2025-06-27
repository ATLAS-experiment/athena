# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

def TestVectorCheckerCfg(flags, **kwargs):
    kwargs.setdefault("name", "TestVectorChecker")

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory 
    acc.addEventAlgo(CompFactory.EFTrackingFPGAUtility.TestVectorChecker(**kwargs))

    return acc

