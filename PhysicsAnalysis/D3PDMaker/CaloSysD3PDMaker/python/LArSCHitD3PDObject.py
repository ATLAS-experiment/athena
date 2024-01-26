# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject          import D3PDObject
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


def _hookForLArSCHitD3PDObject_(c, flags, acc, *args, **kw):

    basFiller = c.BlockFillers[0]
    print("getattr(c, c.name()) / Type= ", type(basFiller))
    if "CaloEtaCut" in list(kw.keys()):
        basFiller.CaloEtaCut = kw["CaloEtaCut"]
    if "CaloPhiCut" in list(kw.keys()):
        basFiller.CaloPhiCut = kw["CaloPhiCut"]
    if "CaloLayers" in list(kw.keys()):
        basFiller.CaloLayers = kw["CaloLayers"]
    if "CaloDetectors" in list(kw.keys()):
        basFiller.CaloDetectors = kw["CaloDetectors"]

    print("%s - CaloEtaCut = " % (basFiller.name), basFiller.CaloEtaCut)
    print("%s - CaloPhiCut = " % (basFiller.name), basFiller.CaloPhiCut)
    print("%s - CaloLayersCut = " % (basFiller.name), basFiller.CaloLayers)
    print("%s - CaloDetectors = " % (basFiller.name), basFiller.CaloDetectors)

    return 

def _makeLArSCHit_obj_(name, prefix, object_name,
                       getter=None,
                       sgKey=None,
                       typeName=None,
                       ):

    from D3PDMakerConfig.D3PDMakerFlags import D3PDMakerFlags
    if not typeName:
        typeName = "LArHitContainer"
    if not sgKey:
        sgKey="LArHitEMB"
    if not getter:
        getter = D3PD.SGObjGetterTool(
            name + '_Getter',
            TypeName = typeName,
            SGKey = sgKey)


    return D3PD.ObjFillerTool( name,
                               Prefix = prefix,
                               Getter = getter,
                               ObjectName = object_name,
                               SaveMetadata = \
                               D3PDMakerFlags.SaveObjectMetadata
                              )
    
def make_LArSCHitD3PDObject( typeName="LArHitContainer",
                             sgKey="LArHitEMB",
                             prefix="sch_",
                             object_name="LArSCHitD3PDObject",
                             *args, **kw ):
        
    obj = D3PDObject(_makeLArSCHit_obj_, prefix, object_name,
                     allow_args=["CaloEtaCut","CaloPhiCut",
                                 "CaloLayers","CaloDetectors"],
                     sgkey=sgKey,
                     typename=typeName)
    obj.defineBlock( 0, 'Basic', D3PD.LArSCHitFillerTool)
    obj.defineHook( _hookForLArSCHitD3PDObject_ )
    return obj

LArSCHitD3PDObject = make_LArSCHitD3PDObject( typeName="LArHitContainer",
                                             sgKey = "LArHitEMB",
                                             prefix = "hsc_",
                                             CaloEtaCut=[],
                                             CaloPhiCut=[],
                                             CaloLayers=[],
                                             CaloDetectors=[],)
