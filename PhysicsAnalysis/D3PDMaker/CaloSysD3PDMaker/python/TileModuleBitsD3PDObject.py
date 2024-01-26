# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject import D3PDObject
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


def makeTileModuleBitsD3PDObject (name, prefix, object_name='TileModuleBitsD3PDObject', getter = None,
                           sgkey = None,
                           label = None):
    if sgkey is None: sgkey = "TileRawChannelFlt"
    if label is None: label = prefix

    if not getter:
        getter = D3PD.SGTileModuleBitsGetterTool \
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



TileModuleBitsD3PDObject = D3PDObject (makeTileModuleBitsD3PDObject, 'tilemodule_', 'TileModuleBitsD3PDObject')

TileModuleBitsD3PDObject.defineBlock (0, 'Bits',
                                      D3PD.TileModuleBitsFillerTool
                                      )

