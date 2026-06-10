# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Project import(s).
from AthenaConfiguration.AthConfigFlags import AthConfigFlags


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

    # Return the container.
    return result
