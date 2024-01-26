# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


from D3PDMakerCoreComps.D3PDObject import D3PDObject
from D3PDMakerConfig.D3PDMakerFlags import D3PDMakerFlags
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD

def makeD3PDObject (name, prefix, object_name, getter = None,
                           sgkey = None,
                           label = None):
    if sgkey is None: sgkey = "LArCalibrationHitActive"
    if label is None: label = prefix

    if prefix=="laract_" :
        sgkey = "LArCalibrationHitActive"
    elif prefix=="larinact_" :
        sgkey = "LArCalibrationHitInactive"
    elif prefix=="lardm_" :
        sgkey = "LArCalibrationHitDeadMaterial"
    elif prefix=="tile_" :
        sgkey = "TileCalibrationCellHitCnt"
    elif prefix=="tileact_" :
        sgkey = "TileCalibHitActiveCell"
    elif prefix=="tileinact_" :
        sgkey = "TileCalibHitInactiveCell"
    elif prefix=="tiledm_" :
        sgkey = "TileCalibHitDeadMaterial"
    elif prefix=="tiledmo_" :
        sgkey = "TileCalibrationDMHitCnt"

    if not getter:
        getter = D3PD.CaloCalibrationHitContainerGetterTool \
                 (name + '_Getter',
                  TypeName = 'CaloCalibrationHitContainer',
                  SGKey = sgkey,
                  Label = label)

    return D3PD.VectorFillerTool (name,
                                  Prefix = prefix,
                                  Getter = getter,
                                  ObjectName = object_name,
                                  SaveMetadata = \
                                  D3PDMakerFlags.SaveObjectMetadata)

LArActiveCaloCalibHitD3PDObject = D3PDObject (makeD3PDObject, 'laract_', 'LArActiveCaloCalibHitD3PDObject')

LArActiveCaloCalibHitD3PDObject.defineBlock (1, 'Hits', D3PD.CaloCalibHitFillerTool)
LArActiveCaloCalibHitD3PDObject.defineBlock (2, 'FullIDHits', D3PD.CaloCalibHitFillerTool, isBasic = False, SaveFullCellID = True)
LArActiveCaloCalibHitD3PDObject.defineBlock (5, 'FullIDAllEnHits', D3PD.CaloCalibHitFillerTool, isBasic = False, SaveFullCellID = False, SaveAllEnergies = True)


LArInactiveCaloCalibHitD3PDObject = D3PDObject (makeD3PDObject, 'larinact_', 'LArInactiveCaloCalibHitD3PDObject')

LArInactiveCaloCalibHitD3PDObject.defineBlock (1, 'Hits', D3PD.CaloCalibHitFillerTool)
LArInactiveCaloCalibHitD3PDObject.defineBlock (2, 'FullIDHits', D3PD.CaloCalibHitFillerTool, isBasic = False,  SaveFullCellID = True)
LArInactiveCaloCalibHitD3PDObject.defineBlock (5, 'FullIDAllEnHits', D3PD.CaloCalibHitFillerTool, isBasic = False,  SaveFullCellID = False, SaveAllEnergies = True)


LArDMCaloCalibHitD3PDObject = D3PDObject (makeD3PDObject, 'lardm_', 'LArDMCaloCalibHitD3PDObject')

LArDMCaloCalibHitD3PDObject.defineBlock (1, 'Hits', D3PD.CaloCalibHitFillerTool,isDeadMaterial = True)
LArDMCaloCalibHitD3PDObject.defineBlock (2, 'FullIDHits', D3PD.CaloCalibHitFillerTool,isDeadMaterial = True,  isBasic = False,  SaveFullCellID = True)
LArDMCaloCalibHitD3PDObject.defineBlock (5, 'FullIDAllEnHits', D3PD.CaloCalibHitFillerTool,isDeadMaterial = True,  isBasic = False,  SaveFullCellID = False, SaveAllEnergies = True)


TileCaloCalibHitD3PDObject = D3PDObject (makeD3PDObject, 'tile_', 'TileCaloCalibHitD3PDObject')

TileCaloCalibHitD3PDObject.defineBlock (1, 'Hits', D3PD.CaloCalibHitFillerTool)
TileCaloCalibHitD3PDObject.defineBlock (2, 'FullIDHits', D3PD.CaloCalibHitFillerTool,  isBasic = False,  SaveFullCellID = True)
TileCaloCalibHitD3PDObject.defineBlock (5, 'FullIDAllEnHits', D3PD.CaloCalibHitFillerTool,  isBasic = False,  SaveFullCellID = False, SaveAllEnergies = True)


TileDMOCaloCalibHitD3PDObject = D3PDObject (makeD3PDObject, 'tiledmo_', 'TileDMOCaloCalibHitD3PDObject')

TileDMOCaloCalibHitD3PDObject.defineBlock (1, 'Hits', D3PD.CaloCalibHitFillerTool, isDeadMaterial = True)
TileDMOCaloCalibHitD3PDObject.defineBlock (2, 'FullIDHits', D3PD.CaloCalibHitFillerTool, isDeadMaterial = True,  isBasic = False,  SaveFullCellID = True )
TileDMOCaloCalibHitD3PDObject.defineBlock (5, 'FullIDAllEnHits', D3PD.CaloCalibHitFillerTool, isDeadMaterial = True,  isBasic = False,  SaveFullCellID = False, SaveAllEnergies = True )


TileActiveCaloCalibHitD3PDObject = D3PDObject (makeD3PDObject, 'tileact_', 'TileActiveCaloCalibHitD3PDObject')

TileActiveCaloCalibHitD3PDObject.defineBlock (1, 'Hits', D3PD.CaloCalibHitFillerTool)
TileActiveCaloCalibHitD3PDObject.defineBlock (2, 'FullIDHits', D3PD.CaloCalibHitFillerTool,  isBasic = False,  SaveFullCellID = True)
TileActiveCaloCalibHitD3PDObject.defineBlock (5, 'FullIDAllEnHits', D3PD.CaloCalibHitFillerTool,  isBasic = False,  SaveFullCellID = False, SaveAllEnergies = True)

TileInactiveCaloCalibHitD3PDObject = D3PDObject (makeD3PDObject, 'tileinact_', 'TileInactiveCaloCalibHitD3PDObject')

TileInactiveCaloCalibHitD3PDObject.defineBlock (1, 'Hits', D3PD.CaloCalibHitFillerTool)
TileInactiveCaloCalibHitD3PDObject.defineBlock (2, 'FullIDHits', D3PD.CaloCalibHitFillerTool,  isBasic = False,  SaveFullCellID = True)
TileInactiveCaloCalibHitD3PDObject.defineBlock (5, 'FullIDAllEnHits', D3PD.CaloCalibHitFillerTool,  isBasic = False,  SaveFullCellID = False, SaveAllEnergies = True)


TileDMCaloCalibHitD3PDObject = D3PDObject (makeD3PDObject, 'tiledm_', 'TileDMCaloCalibHitD3PDObject')

TileDMCaloCalibHitD3PDObject.defineBlock (1, 'Hits', D3PD.CaloCalibHitFillerTool, isDeadMaterial = True)
TileDMCaloCalibHitD3PDObject.defineBlock (2, 'FullIDHits', D3PD.CaloCalibHitFillerTool, isDeadMaterial = True,  isBasic = False, SaveFullCellID = True)
TileDMCaloCalibHitD3PDObject.defineBlock (5, 'FullIDAllEnHits', D3PD.CaloCalibHitFillerTool, isDeadMaterial = True,  isBasic = False, SaveFullCellID = False, SaveAllEnergies = True)


