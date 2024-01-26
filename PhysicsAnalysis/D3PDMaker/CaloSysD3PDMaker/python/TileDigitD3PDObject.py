# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject          import D3PDObject
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD

TileDigitSGKey='TileDigitsFlt'

def makeTileDigitD3PDObject (name, prefix, object_name='TileDigitD3PDObject', getter = None,
                           sgkey = None,
                           label = None):
    if sgkey is None: sgkey = TileDigitSGKey
    if label is None: label = prefix

    if not getter:
        getter = D3PD.SGTileDigitsGetterTool \
                 (name + '_Getter',
                  TypeName = 'TileDigitsContainer',
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
0: digits
1: digit+section+side+tower

"""

TileDigitD3PDObject = D3PDObject (makeTileDigitD3PDObject, 'tiledigit_', 'TileDigitD3PDObject')

TileDigitD3PDObject.defineBlock (0, 'Digits',
                                 D3PD.TileDigitFillerTool,
                                 SaveOfflineInfo= False,
                                 SaveHardwareInfo=True,
                                 )



TileDigitD3PDObject.defineBlock (1, 'SST',
                                 D3PD.TileDigitFillerTool,
                                 SaveOfflineInfo= True,
                                 SaveHardwareInfo=False,
                                 )

