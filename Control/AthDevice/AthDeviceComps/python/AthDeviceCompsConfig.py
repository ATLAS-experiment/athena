# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Framework import(s).
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# Local import(s).
from AthDeviceComps.DeviceConfigFlags import DeviceBackend


def HostCopyToolCfg(flags, **kwargs):
    '''Default "host side" copy object provider tool
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the tool in a simple way.
    result.setPrivateTools(CompFactory.AthDevice.HostCopyTool(**kwargs))

    # Return the CA.
    return result


def HostMemoryResourceToolCfg(flags, **kwargs):
    '''Default "host side" memory resource provider tool
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Make a decision based on the selected backend.
    if flags.Device.Backend == DeviceBackend.CUDA:
        from AthCUDAServices.AthCUDAServicesConfig \
            import HostMemoryResourceToolCfg as CUDAHostMemoryResourceToolCfg
        result.setPrivateTools(result.popToolsAndMerge(
            CUDAHostMemoryResourceToolCfg(flags, **kwargs)))
    else:
        # For an unknown backend, we just fall back on the generic host memory
        # resource tool.
        result.setPrivateTools(
            CompFactory.AthDevice.HostMemoryResourceTool(**kwargs))
        pass

    # Return the CA.
    return result


def DeviceMemoryResourceToolCfg(flags, **kwargs):
    '''Default "device side" memory resource provider tool
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Make a decision based on the selected backend.
    if flags.Device.Backend == DeviceBackend.CUDA:
        from AthCUDAServices.AthCUDAServicesConfig \
            import DeviceMemoryResourceToolCfg as \
            CUDADeviceMemoryResourceToolCfg
        result.setPrivateTools(result.popToolsAndMerge(
            CUDADeviceMemoryResourceToolCfg(flags, **kwargs)))
    else:
        raise RuntimeError('No device memory resource tool is available for '
                           'the selected backend: {}'.format(
                               flags.Device.Backend))

    # Return the CA.
    return result


def SharedMemoryResourceToolCfg(flags, **kwargs):
    '''Default "host/device shared" memory resource provider tool
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Make a decision based on the selected backend.
    if flags.Device.Backend == DeviceBackend.CUDA:
        from AthCUDAServices.AthCUDAServicesConfig \
            import ManagedMemoryResourceToolCfg as \
            CUDAManagedMemoryResourceToolCfg
        result.setPrivateTools(result.popToolsAndMerge(
            CUDAManagedMemoryResourceToolCfg(flags, **kwargs)))
    else:
        raise RuntimeError('No shared memory resource tool is available for '
                           'the selected backend: {}'.format(
                               flags.Device.Backend))

    # Return the CA.
    return result


def MemoryResourcesToolCfg(flags, **kwargs):
    '''Default tool providing the IMemoryResourcesTool interface for the
       selected backend
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Make a decision based on the selected backend.
    if flags.Device.Backend == DeviceBackend.CUDA:
        from AthCUDAServices.AthCUDAServicesConfig \
            import MemoryResourcesToolCfg as CUDAMemoryResourcesToolCfg
        result.setPrivateTools(result.popToolsAndMerge(
            CUDAMemoryResourcesToolCfg(flags, **kwargs)))
    else:
        raise RuntimeError('No memory resources tool is available for the '
                           'selected backend: {}'.format(flags.Device.Backend))

    # Return the CA.
    return result


def CopyToolCfg(flags, **kwargs):
    '''Default tool providing the ICopyTool interface for the selected backend
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Make a decision based on the selected backend.
    if flags.Device.Backend == DeviceBackend.CUDA:
        from AthCUDAServices.AthCUDAServicesConfig \
            import CopyToolCfg as CUDACopyToolCfg
        result.setPrivateTools(result.popToolsAndMerge(
            CUDACopyToolCfg(flags, **kwargs)))
    else:
        raise RuntimeError('No copy tool is available for the selected '
                           'backend: {}'.format(flags.Device.Backend))

    # Return the CA.
    return result


def CopiesToolCfg(flags, **kwargs):
    '''Default tool providing the ICopiesTool interface for the selected backend
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Make a decision based on the selected backend.
    if flags.Device.Backend == DeviceBackend.CUDA:
        from AthCUDAServices.AthCUDAServicesConfig \
            import CopiesToolCfg as CUDACopiesToolCfg
        result.setPrivateTools(result.popToolsAndMerge(
            CUDACopiesToolCfg(flags, **kwargs)))
    else:
        raise RuntimeError('No copies tool is available for the selected '
                           'backend: {}'.format(flags.Device.Backend))

    # Return the CA.
    return result
