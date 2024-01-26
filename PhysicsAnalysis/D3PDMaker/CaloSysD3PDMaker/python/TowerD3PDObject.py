# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject import D3PDObject
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD

def makeTowerD3PDObject (name, prefix, object_name='TowerD3PDObject', getter = None,
                           sgkey = None,
                           label = None):
    if sgkey is None: sgkey = 'CombinedTower'
    if label is None: label = prefix
    
    ContainerType='CaloTowerContainer'
    
    if sgkey== 'TopoTower': ContainerType='CaloTopoTowerContainer'
    
    print(" makeTowerD3PDObject: name = ", name)
    print(" makeTowerD3PDObject: prefix = ", prefix)
    print(" makeTowerD3PDObject: object_name = ", object_name)
    print(" makeTowerD3PDObject: sgkey = ", sgkey)

    if not getter:
        getter = D3PD.SGDataVectorGetterTool \
                 (name + '_Getter',
                  TypeName = ContainerType,
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




def getTowerD3PDObject (maker, prefix,object_name) :

    towerD3PDObject = D3PDObject (maker, prefix, object_name)

    towerD3PDObject.defineBlock (0, 'Detail0',
                                 D3PD.TowerFillerTool,
                                 SaveNCellConstituents=False
                                 )
    
    towerD3PDObject.defineBlock (1, 'Detail1',
                                 D3PD.TowerFillerTool,
                                 SaveNCellConstituents=True
                                 )
    return towerD3PDObject 



# All Tower cells
TowerD3PDObject    = getTowerD3PDObject(makeTowerD3PDObject,'tower_','TowerD3PDObject')


