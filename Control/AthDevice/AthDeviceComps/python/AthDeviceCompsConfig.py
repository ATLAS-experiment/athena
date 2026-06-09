# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Framework import(s).
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def HostCopyToolCfg(flags, **kwargs):
    '''Default "host side" copy object provider tool
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the tool in a simple way.
    result.setPrivateTools(CompFactory.AthDevice.HostCopyTool(**kwargs))

    # Return the CA.
    return result
