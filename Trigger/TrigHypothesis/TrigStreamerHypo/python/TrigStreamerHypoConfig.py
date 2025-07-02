# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory

def StreamerHypoToolGenerator(flags, chainDict):
    """ Configure streamer tool from chainDict """
    return CompFactory.TrigStreamerHypoTool( chainDict['chainName'] )
