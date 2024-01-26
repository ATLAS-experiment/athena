# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject          import D3PDObject
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


def makeTileMuD3PDObject (name, prefix, object_name='TileMuD3PDObject', getter = None,
                           sgkey = None,
                           label = None):
    if sgkey is None: sgkey = 'TileMuObj'
    if label is None: label = prefix

    
    print(" makeTileMuD3PDObject: name = ", name)
    print(" makeTileMuD3PDObject: prefix = ", prefix)
    print(" makeTileMuD3PDObject: object_name = ", object_name)
    print(" makeTileMuD3PDObject: sgkey = ", sgkey)

    if not getter:
        getter = D3PD.SGDataVectorGetterTool \
                 (name + '_Getter',
                  TypeName = 'TileMuContainer',
                  SGKey = sgkey,
                  Label = label)
        

    from D3PDMakerConfig.D3PDMakerFlags import D3PDMakerFlags
    return D3PD.VectorFillerTool (name,
                                  Prefix = prefix,
                                  Getter = getter,
                                  ObjectName = object_name,
                                  SaveMetadata = \
                                  D3PDMakerFlags.SaveObjectMetadata)



TileMuD3PDObject=D3PDObject(makeTileMuD3PDObject,'TileMuId_','TileMuD3PDObject')
    
TileMuD3PDObject.defineBlock (0, 'TileMus',
                              D3PD.TileMuFillerTool)
