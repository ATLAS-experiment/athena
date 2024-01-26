# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject import D3PDObject
from D3PDMakerCoreComps.ContainedVectorMultiAssociation import ContainedVectorMultiAssociation
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD

def makeTileCosmicMuonD3PDObject (name, prefix, object_name='TileCosmicMuonD3PDObject', getter = None,
                                  sgkey = None,
                                  label = None):
    if sgkey is None: sgkey = 'TileCosmicMuonHT'
    if label is None: label = prefix

    
    print(" makeTileCosmicMuonD3PDObject: name = ", name)
    print(" makeTileCosmicMuonD3PDObject: prefix = ", prefix)
    print(" makeTileCosmicMuonD3PDObject: object_name = ", object_name)
    print(" makeTileCosmicMuonD3PDObject: sgkey = ", sgkey)

    if not getter:
        getter = D3PD.SGDataVectorGetterTool \
                 (name + '_Getter',
                  TypeName = 'TileCosmicMuonContainer',
                  SGKey = sgkey,
                  Label = label)
        

    from D3PDMakerConfig.D3PDMakerFlags import D3PDMakerFlags
    return D3PD.VectorFillerTool (name,
                                  Prefix = prefix,
                                  Getter = getter,
                                  ObjectName = object_name,
                                  SaveMetadata = \
                                  D3PDMakerFlags.SaveObjectMetadata)



TileCosmicMuonD3PDObject=D3PDObject(makeTileCosmicMuonD3PDObject,'TileTracks_','TileCosmicMuonD3PDObject')
    
TileCosmicMuonD3PDObject.defineBlock (0, 'TileCosmicMuons',
                                      D3PD.TileCosmicMuonFillerTool)

TileCosmicMuonTileCells=ContainedVectorMultiAssociation(TileCosmicMuonD3PDObject,
                                                        D3PD.TileCosmicMuonTileCellAssociationTool,
                                                        "cell",1)

TileCosmicMuonTileCells.defineBlock (1, 'TileDetail0',
                                     D3PD.TileCellDetailsFillerTool,
                                     SaveCellDetails=True,
                                     SavePositionInfo=False,
                                     )
    
TileCosmicMuonTileCells.defineBlock (2, 'TileDetail1',
                                     D3PD.TileCellDetailsFillerTool,
                                     SaveCellDetails=False,
                                     SavePositionInfo=True,
                                     )





