# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject          import D3PDObject
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD

TileHitSGKey='TileHitVec'

def makeTileHitD3PDObject (name, prefix, object_name='TileHitD3PDObject', getter = None,
                           sgkey = None,
                           label = None):
    if sgkey is None: sgkey = TileHitSGKey
    if label is None: label = prefix

    if not getter:
        getter = D3PD.SGTileHitGetterTool \
                 (name + '_Getter',
                  TypeName = 'TileHitVector',
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




TileHitD3PDObject = D3PDObject (makeTileHitD3PDObject, 'TileHit_', 'TileHitD3PDObject')

TileHitD3PDObject.defineBlock (0, 'TileHitDetails',
                               D3PD.TileHitFillerTool)
MBTSHitD3PDObject = D3PDObject (makeTileHitD3PDObject, 'MBTSHit_', 'MBTSHitD3PDObject', None, 'MBTSHits')
MBTSHitD3PDObject.defineBlock (0, 'MBTSHitDetails',D3PD.TileHitFillerTool)
