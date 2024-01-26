# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject import D3PDObject
from D3PDMakerConfig.D3PDMakerFlags import D3PDMakerFlags
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD

def makeD3PDObject (name, prefix, object_name, getter = None,
                    sgkey = None,
                    label = None):
    if sgkey is None: sgkey = "LArHits"
    if label is None: label = prefix

    if prefix=="hitemb_" :
        sgkey = "LArHitEMB"
    elif prefix=="hitemec_" :
        sgkey = "LArHitEMEC"
    elif prefix=="hithec_" :
        sgkey = "LArHitHEC"
    elif prefix=="hitfcal_" :
        sgkey = "LArHitFCAL"

    if not getter:
        getter = D3PD.LArHitContainerGetterTool \
                 (name + '_Getter',
                  TypeName = 'LArHitContainer',
                  SGKey = sgkey,
                  Label = label)

    return D3PD.VectorFillerTool (name,
                                  Prefix = prefix,
                                  Getter = getter,
                                  ObjectName = object_name,
                                  SaveMetadata = \
                                  D3PDMakerFlags.SaveObjectMetadata)

LArHitEMBD3PDObject = D3PDObject (makeD3PDObject, 'hitemb_', 'LArHitEMBD3PDObject')

LArHitEMBD3PDObject.defineBlock (1, 'Hits',
                                 D3PD.LArHitFillerTool)


LArHitEMECD3PDObject = D3PDObject (makeD3PDObject, 'hitemec_', 'LArHitEMECD3PDObject')

LArHitEMECD3PDObject.defineBlock (1, 'Hits',
                                  D3PD.LArHitFillerTool)


LArHitHECD3PDObject = D3PDObject (makeD3PDObject, 'hithec_', 'LArHitHECD3PDObject')

LArHitHECD3PDObject.defineBlock (1, 'Hits',
                                 D3PD.LArHitFillerTool)


LArHitFCALD3PDObject = D3PDObject (makeD3PDObject, 'hitfcal_', 'LArHitFCALD3PDObject')

LArHitFCALD3PDObject.defineBlock (1, 'Hits',
                                  D3PD.LArHitFillerTool)

