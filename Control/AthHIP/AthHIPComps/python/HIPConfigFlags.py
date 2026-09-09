# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Project import(s).
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import FlagEnum


class HIPStream(FlagEnum):
    '''Enum for the strategy to select/provide a HIP stream to algorithms/tools
    that need to run operations on a HIP device.
    '''
    # Use a single stream for the entire job, for all components.
    Single = 'Single'
    # Use one stream per event/slot.
    PerEvent = 'PerEvent'
    # Use one stream per component (algorithm/tool/service).
    PerComponent = 'PerComponent'
    # Use one stream per component and event/slot.
    PerEventAndComponent = 'PerEventAndComponent'


def createHIPConfigFlags():
    '''Function to create the flags for using HIP devices/GPUs in Athena
    '''

    # Create the flags container.
    result = AthConfigFlags()

    # Memory (resource) management flags.
    result.addFlag('HIP.Stream', HIPStream.Single, type=HIPStream,
                   help='Strategy for selecting/providing a HIP stream')

    # Return the container.
    return result
