# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject import D3PDObject
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD

def makeTileTriggerD3PDObject (name, prefix, object_name='TileTriggerD3PDObject', getter = None,
                           sgkey = None,
                           label = None):
    if sgkey is None: sgkey = 'TileTriggerCnt'
    if label is None: label = prefix

    
    print(" makeTileTriggerD3PDObject: name = ", name)
    print(" makeTileTriggerD3PDObject: prefix = ", prefix)
    print(" makeTileTriggerD3PDObject: object_name = ", object_name)
    print(" makeTileTriggerD3PDObject: sgkey = ", sgkey)

    if not getter:
        getter = D3PD.SGDataVectorGetterTool \
                 (name + '_Getter',
                  TypeName = 'TileTriggerContainer',
                  SGKey = sgkey,
                  Label = label)
        

    from D3PDMakerConfig.D3PDMakerFlags import D3PDMakerFlags
    return D3PD.VectorFillerTool (name,
                                  Prefix = prefix,
                                  Getter = getter,
                                  ObjectName = object_name,
                                  SaveMetadata = \
                                  D3PDMakerFlags.SaveObjectMetadata)



TileTriggerD3PDObject=D3PDObject(makeTileTriggerD3PDObject,'TileTrigger_','TileTriggerD3PDObject')
    
TileTriggerD3PDObject.defineBlock (0, 
                                   'TileTrigger',
                                   D3PD.TileTriggerFillerTool)
