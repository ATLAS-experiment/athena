# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject          import D3PDObject
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


LArRawChannelSGKey='LArRawChannels'

def makeLArRawChannelD3PDObject (name, prefix, object_name='LArRawChannelD3PDObject', getter = None,
                                 sgkey = None,
                                 label = None):
    if sgkey is None: sgkey = LArRawChannelSGKey
    if label is None: label = prefix

    if not getter:
        getter = D3PD.LArRawChannelContainerGetterTool(name + '_Getter',
                  TypeName = 'LArRawChannelContainer',
                  SGKey = LArRawChannelSGKey)

    # create the selected cells
    return D3PD.VectorFillerTool (name,
                                  Prefix = prefix,
                                  Getter = getter,
                                  ObjectName = object_name)

LArRawChannelD3PDObject = D3PDObject (makeLArRawChannelD3PDObject, 'larrawchannel_', 'LArRawChannelD3PDObject')

LArRawChannelD3PDObject.defineBlock (0, 'Digits',
                                     D3PD.LArRawChannelFillerTool)
