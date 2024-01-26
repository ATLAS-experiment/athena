# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject import D3PDObject
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


TileRawChannelSGKey='TileRawChannelFlt'

def makeTileRawChannelD3PDObject (name, prefix, object_name='TileRawChannelD3PDObject', getter = None,
                           sgkey = None,
                           label = None):
    if sgkey is None: sgkey = TileRawChannelSGKey
    if label is None: label = prefix

    if not getter:
        getter = D3PD.SGTileRawChannelGetterTool \
                 (name + '_Getter',
                  TypeName = 'TileRawChannelContainer',
                  SGKey = sgkey,
                  Label = label)

    # create the selected cells
    from D3PDMakerConfig.D3PDMakerFlags import D3PDMakerFlags
    return D3PD.VectorFillerTool (name,
                                  Prefix = prefix,
                                  Getter = getter,
                                  ObjectName = object_name,
                                  SaveMetadata = \
                                  D3PDMakerFlags.SaveObjectMetadata)



""" level of details:
0: amp, time, quality and pedestal
1: hardware id + amp, time, quality and pedestal

"""

TileRawChannelD3PDObject = D3PDObject( makeTileRawChannelD3PDObject, 'tileraw_',
                                       'TileRawChannelD3PDObject' )

TileRawChannelD3PDObject.defineBlock (0, 'RawChannel',
                                      D3PD.TileRawChannelFillerTool,
                                      SaveHardwareInfo=False,
                                      SaveRawChannel= True,
                                      )



TileRawChannelD3PDObject.defineBlock (1, 'Hardware',
                                      D3PD.TileRawChannelFillerTool,
                                      SaveHardwareInfo=True,
                                      SaveRawChannel= False,
                                      )




