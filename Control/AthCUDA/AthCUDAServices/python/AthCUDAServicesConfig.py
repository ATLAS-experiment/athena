# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Framework import(s).
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# Local import(s).
from AthCUDAServices.CUDAConfigFlags import CUDAStream


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


def CopyToolCfg(flags, **kwargs):
    '''Synchronous copy object provider tool
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the tool in a simple way.
    result.setPrivateTools(CompFactory.AthCUDA.CopyTool(**kwargs))

    # Return the CA.
    return result


def AsyncCopyToolCfg(flags, **kwargs):
    '''Asynchronous copy object provider tool
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the tool in a simple way.
    result.setPrivateTools(CompFactory.AthCUDA.AsyncCopyTool(**kwargs))

    # Return the CA.
    return result


def SingleStreamToolCfg(flags, **kwargs):
    '''Tool providing a single CUDA stream for all components in the entire job
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the stream service and add it to the accumulator.
    streamSvc = CompFactory.AthCUDA.SingleStreamSvc(**kwargs)
    result.addService(streamSvc)

    # Create an adaptor tool on top of the service, and set that as the main
    # component of the CA.
    streamTool = CompFactory.AthCUDA.StreamSvcAdaptorTool(
        'SingleStreamTool', StreamSvc=streamSvc)
    result.setPrivateTools(streamTool)

    # Return the CA.
    return result


def PerEventStreamToolCfg(flags, **kwargs):
    '''Tool providing one CUDA stream per event/slot
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the stream service and add it to the accumulator.
    streamSvc = CompFactory.AthCUDA.PerEventStreamSvc(**kwargs)
    result.addService(streamSvc)

    # Create an adaptor tool on top of the service, and set that as the main
    # component of the CA.
    streamTool = CompFactory.AthCUDA.StreamSvcAdaptorTool(
        'PerEventStreamTool', StreamSvc=streamSvc)
    result.setPrivateTools(streamTool)

    # Return the CA.
    return result


def PerComponentStreamToolCfg(flags, **kwargs):
    '''Tool providing one CUDA stream per component (algorithm/tool/service)
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create an tool that implements this behaviour.
    streamTool = CompFactory.AthCUDA.PerComponentStreamTool(**kwargs)
    result.setPrivateTools(streamTool)

    # Return the CA.
    return result

def PerEventAndComponentStreamToolCfg(flags, **kwargs):
    '''Tool providing one CUDA stream per component and event/slot
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create an tool that implements this behaviour.
    streamTool = CompFactory.AthCUDA.PerEventAndComponentStreamTool(**kwargs)
    result.setPrivateTools(streamTool)

    # Return the CA.
    return result


def StreamToolCfg(flags, **kwargs):
    '''Default CUDA stream provider tool to use
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the default stream tool, depending on the job's configuration.
    if flags.CUDA.Stream == CUDAStream.Single:
        cfg = SingleStreamToolCfg(flags, **kwargs)
        result.setPrivateTools(cfg.getPrimary())
        result.merge(cfg)
    elif flags.CUDA.Stream == CUDAStream.PerEvent:
        cfg = PerEventStreamToolCfg(flags, **kwargs)
        result.setPrivateTools(cfg.getPrimary())
        result.merge(cfg)
    elif flags.CUDA.Stream == CUDAStream.PerComponent:
        cfg = PerComponentStreamToolCfg(flags, **kwargs)
        result.setPrivateTools(cfg.getPrimary())
        result.merge(cfg)
    elif flags.CUDA.Stream == CUDAStream.PerEventAndComponent:
        cfg = PerEventAndComponentStreamToolCfg(flags, **kwargs)
        result.setPrivateTools(cfg.getPrimary())
        result.merge(cfg)
    else:
        raise ValueError(f"Invalid CUDA stream strategy: {flags.CUDA.Stream}")
        pass

    # Return the CA.
    return result
