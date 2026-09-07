# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Framework import(s).
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# Local import(s).
from AthHIPComps.HIPConfigFlags import HIPStream


def HostMemoryResourceToolCfg(flags, **kwargs):
    '''Default HIP host memory resource tool to use

    It makes sure that appropriate caching would be used, as allocating pinned
    host memory is relatively slow.
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the components that would collaborate to provide thread-safe
    # caching to the "bare" memory resource.
    tool = CompFactory.AthHIP.HostMemoryResourceTool(**kwargs)
    if flags.Device.Memory.Debug:
        debugTool = CompFactory.AthDevice.DebugMemoryResourceTool(
            'HIPHostMemoryResourceDebugTool',
            MRTool=tool)
        tool = debugTool
        pass
    if flags.Device.Memory.Cache:
        cacheSvc = CompFactory.AthDevice.BinaryPageMemoryResourceSvc(
            'HIPHostCachedMemoryResourceSvc',
            MRTool=tool)
        result.addService(cacheSvc)
        cacheTool = CompFactory.AthDevice.MemoryResourceSvcAdaptorTool(
            'HIPHostCachedMemoryResourceTool',
            MRSvc=cacheSvc)
        tool = cacheTool
        if flags.Device.Memory.Debug:
            debugTool = CompFactory.AthDevice.DebugMemoryResourceTool(
                'HIPHostCachedMemoryResourceDebugTool',
                MRTool=tool)
            tool = debugTool
            pass
        pass
    result.setPrivateTools(tool)

    # Return the CA.
    return result


def DeviceMemoryResourceToolCfg(flags, **kwargs):
    '''Default HIP device memory resource tool to use

    It makes sure that appropriate caching would be used, as allocating device
    memory is relatively slow.
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the components that would collaborate to provide thread-safe
    # caching to the "bare" memory resource.
    tool = CompFactory.AthHIP.DeviceMemoryResourceTool(**kwargs)
    if flags.Device.Memory.Debug:
        debugTool = CompFactory.AthDevice.DebugMemoryResourceTool(
            'HIPDeviceMemoryResourceDebugTool',
            MRTool=tool)
        tool = debugTool
        pass
    if flags.Device.Memory.Cache:
        cacheSvc = CompFactory.AthDevice.BinaryPageMemoryResourceSvc(
            'HIPDeviceCachedMemoryResourceSvc',
            MRTool=tool)
        result.addService(cacheSvc)
        cacheTool = CompFactory.AthDevice.MemoryResourceSvcAdaptorTool(
            'HIPDeviceCachedMemoryResourceTool',
            MRSvc=cacheSvc)
        tool = cacheTool
        if flags.Device.Memory.Debug:
            debugTool = CompFactory.AthDevice.DebugMemoryResourceTool(
                'HIPDeviceCachedMemoryResourceDebugTool',
                MRTool=tool)
            tool = debugTool
            pass
        pass
    result.setPrivateTools(tool)

    # Return the CA.
    return result


def ManagedMemoryResourceToolCfg(flags, **kwargs):
    '''Default HIP managed memory resource tool to use

    It makes sure that appropriate caching would be used, as allocating managed
    memory is relatively slow.
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the components that would collaborate to provide thread-safe
    # caching to the "bare" memory resource.
    tool = CompFactory.AthHIP.ManagedMemoryResourceTool(**kwargs)
    if flags.Device.Memory.Debug:
        debugTool = CompFactory.AthDevice.DebugMemoryResourceTool(
            'HIPManagedMemoryResourceDebugTool',
            MRTool=tool)
        tool = debugTool
        pass
    if flags.Device.Memory.Cache:
        cacheSvc = CompFactory.AthDevice.BinaryPageMemoryResourceSvc(
            'HIPManagedCachedMemoryResourceSvc',
            MRTool=tool)
        result.addService(cacheSvc)
        cacheTool = CompFactory.AthDevice.MemoryResourceSvcAdaptorTool(
            'HIPManagedCachedMemoryResourceTool',
            MRSvc=cacheSvc)
        tool = cacheTool
        if flags.Device.Memory.Debug:
            debugTool = CompFactory.AthDevice.DebugMemoryResourceTool(
                'HIPManagedCachedMemoryResourceDebugTool',
                MRTool=tool)
            tool = debugTool
            pass
        pass
    result.setPrivateTools(tool)

    # Return the CA.
    return result


def MemoryResourcesToolCfg(flags, **kwargs):
    '''Default tool providing the IMemoryResourcesTool interface for HIP
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the main tool that would provide the
    # AthDevice::IMemoryResourcesTool interface.
    tool = CompFactory.AthDevice.MemoryResourcesAdaptorTool(**kwargs)

    # Set up the main tool according to the received flags.
    if flags.Device.Memory.Shared:
        tool.MainMRTool = result.getPrimaryAndMerge(
            ManagedMemoryResourceToolCfg(flags))
    else:
        tool.MainMRTool = result.getPrimaryAndMerge(
            DeviceMemoryResourceToolCfg(flags))
        tool.HostMRTool = result.getPrimaryAndMerge(
            HostMemoryResourceToolCfg(flags))
        pass

    # Return the adaptor tool as the main component of the CA.
    result.setPrivateTools(tool)
    return result


def SingleStreamToolCfg(flags, **kwargs):
    '''Tool providing a single HIP stream for all components in the entire job
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the stream service and add it to the accumulator.
    streamSvc = CompFactory.AthHIP.SingleStreamSvc(**kwargs)
    result.addService(streamSvc)

    # Create an adaptor tool on top of the service, and set that as the main
    # component of the CA.
    streamTool = CompFactory.AthHIP.StreamSvcAdaptorTool(
        'HIPSingleStreamTool', StreamSvc=streamSvc)
    result.setPrivateTools(streamTool)

    # Return the CA.
    return result


def PerEventStreamToolCfg(flags, **kwargs):
    '''Tool providing one HIP stream per event/slot
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the stream service and add it to the accumulator.
    streamSvc = CompFactory.AthHIP.PerEventStreamSvc(**kwargs)
    result.addService(streamSvc)

    # Create an adaptor tool on top of the service, and set that as the main
    # component of the CA.
    streamTool = CompFactory.AthHIP.StreamSvcAdaptorTool(
        'HIPPerEventStreamTool', StreamSvc=streamSvc)
    result.setPrivateTools(streamTool)

    # Return the CA.
    return result


def PerComponentStreamToolCfg(flags, **kwargs):
    '''Tool providing one HIP stream per component (algorithm/tool/service)
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create an tool that implements this behaviour.
    streamTool = CompFactory.AthHIP.PerComponentStreamTool(**kwargs)
    result.setPrivateTools(streamTool)

    # Return the CA.
    return result


def PerEventAndComponentStreamToolCfg(flags, **kwargs):
    '''Tool providing one HIP stream per component and event/slot
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create an tool that implements this behaviour.
    streamTool = CompFactory.AthHIP.PerEventAndComponentStreamTool(**kwargs)
    result.setPrivateTools(streamTool)

    # Return the CA.
    return result


def StreamToolCfg(flags, **kwargs):
    '''Default HIP stream provider tool to use
    '''

    # Create the default stream tool, depending on the job's configuration.
    if flags.HIP.Stream == HIPStream.Single:
        return SingleStreamToolCfg(flags, **kwargs)
    elif flags.HIP.Stream == HIPStream.PerEvent:
        return PerEventStreamToolCfg(flags, **kwargs)
    elif flags.HIP.Stream == HIPStream.PerComponent:
        return PerComponentStreamToolCfg(flags, **kwargs)
    elif flags.HIP.Stream == HIPStream.PerEventAndComponent:
        return PerEventAndComponentStreamToolCfg(flags, **kwargs)
    else:
        raise ValueError(f"Invalid HIP stream strategy: {flags.HIP.Stream}")
        pass
    pass


def SyncCopyToolCfg(flags, **kwargs):
    '''Synchronous copy object provider tool
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the tool in a simple way.
    result.setPrivateTools(CompFactory.AthHIP.CopyTool(**kwargs))

    # Return the CA.
    return result


def AsyncCopyToolCfg(flags, **kwargs):
    '''Asynchronous copy object provider tool
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the tool. Attaching a stream tool to it.
    copyTool = CompFactory.AthHIP.AsyncCopyTool(**kwargs)
    streamTool = StreamToolCfg(flags, **kwargs)
    copyTool.StreamTool = streamTool.getPrimary()
    result.merge(streamTool)
    result.setPrivateTools(copyTool)

    # Return the CA.
    return result


def CopyToolCfg(flags, **kwargs):
    '''Default tool providing the ICopyTool interface for HIP
    '''

    # Set up the device copy tool according to the received flags.
    if flags.Device.Copy.Async:
        return AsyncCopyToolCfg(flags, **kwargs)
    else:
        return SyncCopyToolCfg(flags, **kwargs)
        pass


def CopiesToolCfg(flags, **kwargs):
    '''Default tool providing the ICopiesTool interface for HIP
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the main tool that would provide the AthDevice::ICopiesTool
    # interface.
    tool = CompFactory.AthDevice.CopiesAdaptorTool(**kwargs)

    # Set up the "host" copy tool. Which is always the same in our current code.
    from AthDeviceComps.AthDeviceCompsConfig import HostCopyToolCfg
    tool.HostCopyTool = \
        result.popToolsAndMerge(HostCopyToolCfg(flags, **kwargs))

    # Set up the device copy tool according to the received flags.
    if flags.Device.Copy.Async:
        tool.DeviceCopyTool = \
            result.popToolsAndMerge(AsyncCopyToolCfg(flags, **kwargs))
    else:
        tool.DeviceCopyTool = \
            result.popToolsAndMerge(SyncCopyToolCfg(flags, **kwargs))
        pass

    # Return the adaptor tool as the main component of the CA.
    result.setPrivateTools(tool)

    # Return the CA.
    return result
