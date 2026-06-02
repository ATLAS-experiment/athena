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
    tool = CompFactory.AthCUDA.HostMemoryResourceTool(**kwargs)
    if flags.Device.Memory.Debug:
        debugTool = CompFactory.AthDevice.DebugMemoryResourceTool(
            'CUDAHostMemoryResourceDebugTool',
            MRTool=tool)
        tool = debugTool
        pass
    if flags.Device.Memory.Cache:
        cacheSvc = CompFactory.AthDevice.BinaryPageMemoryResourceSvc(
            'CUDAHostCachedMemoryResourceSvc',
            MRTool=tool)
        result.addService(cacheSvc)
        cacheTool = CompFactory.AthDevice.MemoryResourceSvcAdaptorTool(
            'CUDAHostCachedMemoryResourceTool',
            MRSvc=cacheSvc)
        tool = cacheTool
        if flags.Device.Memory.Debug:
            debugTool = CompFactory.AthDevice.DebugMemoryResourceTool(
                'CUDAHostCachedMemoryResourceDebugTool',
                MRTool=tool)
            tool = debugTool
            pass
        pass
    result.setPrivateTools(tool)

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
    tool = CompFactory.AthCUDA.DeviceMemoryResourceTool(**kwargs)
    if flags.Device.Memory.Debug:
        debugTool = CompFactory.AthDevice.DebugMemoryResourceTool(
            'CUDADeviceMemoryResourceDebugTool',
            MRTool=tool)
        tool = debugTool
        pass
    if flags.Device.Memory.Cache:
        cacheSvc = CompFactory.AthDevice.BinaryPageMemoryResourceSvc(
            'CUDADeviceCachedMemoryResourceSvc',
            MRTool=tool)
        result.addService(cacheSvc)
        cacheTool = CompFactory.AthDevice.MemoryResourceSvcAdaptorTool(
            'CUDADeviceCachedMemoryResourceTool',
            MRSvc=cacheSvc)
        tool = cacheTool
        if flags.Device.Memory.Debug:
            debugTool = CompFactory.AthDevice.DebugMemoryResourceTool(
                'CUDADeviceCachedMemoryResourceDebugTool',
                MRTool=tool)
            tool = debugTool
            pass
        pass
    result.setPrivateTools(tool)

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
    tool = CompFactory.AthCUDA.ManagedMemoryResourceTool(**kwargs)
    if flags.Device.Memory.Debug:
        debugTool = CompFactory.AthDevice.DebugMemoryResourceTool(
            'CUDAManagedMemoryResourceDebugTool',
            MRTool=tool)
        tool = debugTool
        pass
    if flags.Device.Memory.Cache:
        cacheSvc = CompFactory.AthDevice.BinaryPageMemoryResourceSvc(
            'CUDAManagedCachedMemoryResourceSvc',
            MRTool=tool)
        result.addService(cacheSvc)
        cacheTool = CompFactory.AthDevice.MemoryResourceSvcAdaptorTool(
            'CUDAManagedCachedMemoryResourceTool',
            MRSvc=cacheSvc)
        tool = cacheTool
        if flags.Device.Memory.Debug:
            debugTool = CompFactory.AthDevice.DebugMemoryResourceTool(
                'CUDAManagedCachedMemoryResourceDebugTool',
                MRTool=tool)
            tool = debugTool
            pass
        pass
    result.setPrivateTools(tool)

    # Return the CA.
    return result
