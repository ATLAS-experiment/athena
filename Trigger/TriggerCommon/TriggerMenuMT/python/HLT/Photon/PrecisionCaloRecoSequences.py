#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Logging    import logging
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys      import getTrigEgammaKeys
from TriggerMenuMT.HLT.Egamma.TrigEgammaFactoriesCfg import TrigEgammaRecCfg, TrigEgammaSuperClusterBuilderCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

log = logging.getLogger(__name__)


def precisionCaloPhotonVDVCfg(flags, name, InViewRoIs, ion=False):
    acc = ComponentAccumulator()
    TrigEgammaKeys = getTrigEgammaKeys(flags, ion=ion)
    dataObjects= [( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s'%InViewRoIs ),
                  ( 'CaloBCIDAverage' , 'StoreGateSvc+CaloBCIDAverage' ),
                  ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing' )]
    if ion:
        dataObjects += [( 'xAOD::HIEventShapeContainer' , 'StoreGateSvc+' + TrigEgammaKeys.egEventShape ),
                        ( 'CaloBCIDAverage' , 'StoreGateSvc+CaloBCIDAverage' ),
                        ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing' )]
    if ( not flags.Input.isMC ):
        dataObjects += [('LArDeadOTXFromSC' , 'StoreGateSvc+DeadOTXFromSC' )]

    precisionCaloPhotonVDV = CompFactory.AthViews.ViewDataVerifier(name)
    precisionCaloPhotonVDV.DataObjects = dataObjects
    acc.addEventAlgo(precisionCaloPhotonVDV)
    return acc

def precisionCaloRecoSequence(flags, RoIs, name = None, ion=False):

    acc = ComponentAccumulator()

    TrigEgammaKeys = getTrigEgammaKeys(flags, ion = ion)
    log.debug('flags = %s',flags)
    log.debug('RoIs = %s',RoIs)

    acc.merge(precisionCaloPhotonVDVCfg(flags,name+'VDV',RoIs,ion))

    from TrigCaloRec.TrigCaloRecConfig import egammaTopoClusteringCfg, hltCaloTopoClusteringHICfg
    
    if ion:
        topoCluster = hltCaloTopoClusteringHICfg(flags,
                                                 CellsName = "CaloCells",
                                                 roisKey=RoIs)
    else:
        topoCluster = egammaTopoClusteringCfg(flags, RoIs) 
    acc.merge(topoCluster)
    tag = 'HI' if ion is True else '' 
    
    copier = CompFactory.egammaTopoClusterCopier('gTrigEgammaTopoClusterCopier'+  tag + RoIs,
                                                 InputTopoCollection=TrigEgammaKeys.precisionTopoClusterContainer,
                                                 OutputTopoCollection= TrigEgammaKeys.precisionCaloTopoCollection)
    acc.addEventAlgo(copier)

    trigEgammaRec = TrigEgammaRecCfg(flags, name = 'gTrigEgammaRec'+tag + RoIs)
        
    acc.merge(trigEgammaRec)

    trigEgammaSuperClusterBuilder = TrigEgammaSuperClusterBuilderCfg(flags,
                                                                     'gTrigEgammaSuperClusterBuilder' + tag + RoIs,
                                                                     'photon', 
                                                                     TrigEgammaKeys.precisionPhotonCaloClusterContainer,
                                                                     TrigEgammaKeys.precisionPhotonSuperClusterCollection)
    acc.merge(trigEgammaSuperClusterBuilder)

    return acc



