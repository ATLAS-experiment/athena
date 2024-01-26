# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


from D3PDMakerCoreComps.D3PDObject            import D3PDObject
from CaloD3PDMaker.TileCellFilterAlgConfig    import TileCellFilterAlgCfg
from D3PDMakerCoreComps.IndexMultiAssociation import IndexMultiAssociation
from AthenaConfiguration.ComponentFactory     import CompFactory

D3PD = CompFactory.D3PD


BaseSGKey='holder'

def makeTileD3PDObject (name, prefix, object_name='TileDetailsD3PDObject', getter = None,
                           sgkey = None,
                           label = None):
    global BaseSGKey
    BaseSGKey=sgkey
    sgkey = prefix
    if label is None: label = prefix

    
    print(" makeTileD3PDObject: name = ", name)
    print(" makeTileD3PDObject: prefix = ", prefix)
    print(" makeTileD3PDObject: object_name = ", object_name)
    print(" makeTileD3PDObject: sgkey = ", sgkey)

    if not getter:
        getter = D3PD.SGDataVectorGetterTool \
                 (name + '_Getter',
                  TypeName = 'CaloCellContainer',
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



# function to create the CaloCellContainer for selected
def hookForTileCellFilterAlg(c, flags, acc, *args, **kw):

    cellSigmaCut=-1.
    global BaseSGKey
    if BaseSGKey is None or BaseSGKey=='holder' : BaseSGKey='AllCalo'
    print ('aaa', args, kw)
    sgkey = kw['prefix']

    print(" in makeTileD3PDObject, sgkey, cellSigmaCut = ",  sgkey, cellSigmaCut)

    acc.merge (TileCellFilterAlgCfg (flags,
                                     OutputCellsName=sgkey, 
                                     CellSigmaCut=cellSigmaCut, 
                                     CellsName = BaseSGKey))
    return

def TileCellRawAssoc(parent,prefix='',target='',level=0,blockname=None,*args,**kw):

    if blockname is None:
        blockname=prefix+'TileCellRawAssociation'

    return IndexMultiAssociation(parent,
                                 D3PD.TileCellRawAssociationTool,
                                 target,
                                 prefix,
                                 level,
                                 blockname,
                                 nrowName='')

def TileCellDigitAssoc(parent,prefix='',target='',level=0,blockname=None,*args,**kw):
    if blockname is None:
        blockname=prefix+'TileCellDigitAssociation'

    return IndexMultiAssociation(parent,
                                 D3PD.TileCellDigitAssociationTool,
                                 target,
                                 prefix,
                                 level,
                                 blockname,
                                 nrowName='')


def makeTileCellD3PDObject (maker, prefix,object_name) :
    
    cellD3PDObject = D3PDObject (maker, prefix, object_name)
    cellD3PDObject.defineHook(hookForTileCellFilterAlg)

    cellD3PDObject.defineBlock (0, 'Kinematics',
                                D3PD.FourMomFillerTool,
                                WriteE=True,
                                WritePt=True,
                                WriteEtaPhi=True,
                                WriteM=False,
                                )
    
    cellD3PDObject.defineBlock (1, 'TileDetails',
                                D3PD.TileCellDetailsFillerTool,
                                SaveCellDetails=True,
                                SavePositionInfo=False,
                                )

    return cellD3PDObject



# All Tile cells
TileDetailsD3PDObject    = makeTileCellD3PDObject(makeTileD3PDObject,'tilecell_',
                                                  'TileDetailsD3PDObject')
