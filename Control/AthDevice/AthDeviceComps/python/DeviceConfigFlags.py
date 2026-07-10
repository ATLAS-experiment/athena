# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Project import(s).
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import FlagEnum


class DeviceBackend(FlagEnum):
    '''Enum for the exact device (GPU) backend to use.
    '''
    # CUDA code, targeting NVIDIA GPUs.
    CUDA = 'CUDA'
    # HIP code, targeting AMD GPUs.
    HIPAMD = 'HIPAMD'
    # HIP code, targeting NVIDIA GPUs.
    HIPNVIDIA = 'HIPNVIDIA'
    # Alpaka code, targeting NVIDIA GPUs.
    AlpakaNVIDIA = 'AlpakaNVIDIA'
    # Alpaka code, targeting AMD GPUs.
    AlpakaAMD = 'AlpakaAMD'
    # Alpaka code, targeting Intel GPUs.
    AlpakaIntel = 'AlpakaIntel'
    # SYCL code
    SYCL = 'SYCL'


def createDeviceConfigFlags():
    '''Function to create the flags for using accelerator devices in Athena
    '''

    # Create the flags container.
    result = AthConfigFlags()

    # Memory (resource) management flags.
    result.addFlag('Device.Memory.Cache', True,
                   help='Whether or not to cache memory allocations')
    result.addFlag('Device.Memory.Debug', False,
                   help='Whether or not to actively debug memory allocations')
    result.addFlag('Device.Memory.Shared', False,
                   help='Whether or not to use "shared" memory for device '
                        'allocations. In case it is set to False, a separate '
                        'host and device memory resource is to be used.')

    # Memory copy management flag(s).
    result.addFlag('Device.Copy.Async', True,
                   help='Whether or not to use asynchronous memory copies')

    # Backend selection flag(s).
    result.addFlag('Device.Backend', DeviceBackend.CUDA, type=DeviceBackend,
                   help='The backend to use for accelerator devices')

    # Return the container.
    return result
