# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# menu components   
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TriggerMenuMT.HLT.CommonSequences.CaloSequences import CaloMenuDefs

# logger
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

def fastPhotonVDVCfg(name, InViewRoIs):
    acc = ComponentAccumulator()
    fastPhotonVDV = CompFactory.AthViews.ViewDataVerifier(name)
    fastPhotonVDV.DataObjects = {( 'xAOD::TrigEMClusterContainer' , 'StoreGateSvc+%s' % CaloMenuDefs.L2CaloClusters ),
                                 ( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s'%InViewRoIs  ),
                                 }
    acc.addEventAlgo(fastPhotonVDV)
    return acc


def fastPhotonRecoSequence(flags, RoIs, name = None):
    """Creates secpond step photon sequence"""
    acc = ComponentAccumulator()    
    
    acc.merge(fastPhotonVDVCfg(name+'VDV',RoIs))

    TrigEgammaKeys = getTrigEgammaKeys()

    thePhotonFex =  CompFactory.TrigEgammaFastPhotonReAlgo("EgammaFastPhotonFex_1")
    thePhotonFex.TrigEMClusterName = CaloMenuDefs.L2CaloClusters # From commom staff
    thePhotonFex.PhotonsName= TrigEgammaKeys.fastPhotonContainer

    thePhotonFex.RoIs = RoIs

    acc.addEventAlgo(thePhotonFex)

    return acc
