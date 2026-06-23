# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Project import(s).
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import FlagEnum


class CUDAStream(FlagEnum):
    '''Enum for the strategy to select/provide a CUDA stream to algorithms/tools
    that need to run operations on a CUDA device.
    '''
    # Use a single stream for the entire job, for all components.
    Single = 'Single'
    # Use one stream per event/slot.
    PerEvent = 'PerEvent'
    # Use one stream per component (algorithm/tool/service).
    PerComponent = 'PerComponent'
    # Use one stream per component and event/slot.
    PerEventAndComponent = 'PerEventAndComponent'


def createCUDAConfigFlags():
    '''Function to create the flags for using CUDA devices/GPUs in Athena
    '''

    # Create the flags container.
    result = AthConfigFlags()

    # Memory (resource) management flags.
    result.addFlag('CUDA.Stream', CUDAStream.Single, type=CUDAStream,
                   help='Strategy for selecting/providing a CUDA stream')

    # Return the container.
    return result
