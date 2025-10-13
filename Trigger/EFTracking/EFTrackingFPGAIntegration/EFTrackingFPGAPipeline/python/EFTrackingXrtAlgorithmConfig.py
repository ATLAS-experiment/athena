# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

def EFTrackingXrtAlgorithmCfg(flags, **kwargs):
    kwargs.setdefault("name", "EFTrackingXrtAlgorithm")
    kwargs.setdefault("bufferSize", 8192)
    kwargs.setdefault("inputInterfaces", [])
    kwargs.setdefault("vSizeInterfaces", [])
    kwargs.setdefault("outputInterfaces", [])
    kwargs.setdefault("sharedInterfaces", [])
    kwargs.setdefault("kernelOrder", [])
    kwargs.setdefault("inputDataStreamKeys", [storeGateKey for kernelName, storeGateKey, argumentIndex in kwargs["inputInterfaces"]])
    kwargs.setdefault("vSizeDataStreamKeys", [storeGateKey for kernelName, storeGateKey, argumentIndex in kwargs["vSizeInterfaces"]])
    kwargs.setdefault("outputDataStreamKeys", [storeGateKey for kernelName, storeGateKey, argumentIndex in kwargs["outputInterfaces"]])

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory 
    acc.addEventAlgo(CompFactory.EFTrackingXrtAlgorithm(**kwargs))

    return acc

