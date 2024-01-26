# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject          import make_SG_D3PDObject
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


TileHitInfoD3PDObject = make_SG_D3PDObject( "AtlasHitsVector<TileHit>", "TileHitVec", "Tile_",
                                            "TileHitInfoInfoD3PDObject" )

TileHitInfoD3PDObject.defineBlock( 0, 'TileHitInfo',
                                   D3PD.TileHitInfoFillerTool,
                                   TimeMin = -12.5, TimeMax = 12.5, TimeOut = 99990. )


