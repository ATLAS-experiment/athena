# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Framework import(s).
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def GPUSystemInfoSvcCfg(flags):
    acc = ComponentAccumulator()
    svc = CompFactory.getComp("AthCUDA::GPUSystemInfoSvc")("GPUSystemInfoSvc")
    acc.addService(svc)
    return acc


def HostMemoryResourceToolCfg(flags, **kwargs):
    '''Default CUDA host memory resource tool to use

    It makes sure that appropriate caching would be used, as allocating pinned
    host memory is relatively slow.
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the components that would collaborate to provide thread-safe
    # caching to the "bare" memory resource.
    hostTool = CompFactory.AthCUDA.HostMemoryResourceTool(**kwargs)
    cacheSvc = CompFactory.AthDevice.BinaryPageMemoryResourceSvc(
        'CUDAHostCachedMemoryResourceSvc',
        MRTool=hostTool)
    result.addService(cacheSvc)
    finalTool = CompFactory.AthDevice.MemoryResourceSvcAdaptorTool(
        'CUDAHostCachedMemoryResourceTool',
        MRSvc=cacheSvc)
    result.setPrivateTools(finalTool)

    # Return the CA.
    return result


def DeviceMemoryResourceToolCfg(flags, **kwargs):
    '''Default CUDA device memory resource tool to use

    It makes sure that appropriate caching would be used, as allocating device
    memory is relatively slow.
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the components that would collaborate to provide thread-safe
    # caching to the "bare" memory resource.
    deviceTool = CompFactory.AthCUDA.DeviceMemoryResourceTool(**kwargs)
    cacheSvc = CompFactory.AthDevice.BinaryPageMemoryResourceSvc(
        'CUDADeviceCachedMemoryResourceSvc',
        MRTool=deviceTool)
    result.addService(cacheSvc)
    finalTool = CompFactory.AthDevice.MemoryResourceSvcAdaptorTool(
        'CUDADeviceCachedMemoryResourceTool',
        MRSvc=cacheSvc)
    result.setPrivateTools(finalTool)

    # Return the CA.
    return result


def ManagedMemoryResourceToolCfg(flags, **kwargs):
    '''Default CUDA managed memory resource tool to use

    It makes sure that appropriate caching would be used, as allocating managed
    memory is relatively slow.
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the components that would collaborate to provide thread-safe
    # caching to the "bare" memory resource.
    managedTool = CompFactory.AthCUDA.ManagedMemoryResourceTool(**kwargs)
    cacheSvc = CompFactory.AthDevice.BinaryPageMemoryResourceSvc(
        'CUDAManagedCachedMemoryResourceSvc',
        MRTool=managedTool)
    result.addService(cacheSvc)
    finalTool = CompFactory.AthDevice.MemoryResourceSvcAdaptorTool(
        'CUDAManagedCachedMemoryResourceTool',
        MRSvc=cacheSvc)
    result.setPrivateTools(finalTool)

    # Return the CA.
    return result
