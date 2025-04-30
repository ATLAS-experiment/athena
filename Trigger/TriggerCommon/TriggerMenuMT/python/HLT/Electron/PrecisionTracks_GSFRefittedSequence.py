#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
#logging
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

def precisionTracks_GSFRefitted(flags, RoIs, ion=False, variant=''):
    """
    Takes precision Tracks as input and applies GSF refits on top
    """
    acc = ComponentAccumulator()

    log.debug('precisionTracks_GSFRefitted(RoIs = %s, variant = %s)',RoIs,variant)

    tag = '_ion' if ion else ''
    tag+=variant

    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import  getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys(flags, variant, ion=ion)

    precisionGsfVDV = CompFactory.AthViews.ViewDataVerifier("PrecisionTrackViewDataVerifier_forGSFRefit"+tag+'VDV')

    # precision Tracking related data dependencies
    trackParticles = TrigEgammaKeys.precisionTrackingContainer

    dataObjects = [( 'xAOD::TrackParticleContainer','StoreGateSvc+%s' % trackParticles),
                   # verifier object needed by GSF
                   ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing' ),
                   ( 'InDet::PixelGangedClusterAmbiguities' , f'StoreGateSvc+{flags.Trigger.InDetTracking.PixClustersAmbiMap}'),
                   ( 'InDet::TRT_DriftCircleContainer' , f'StoreGateSvc+{flags.Trigger.InDetTracking.TRT_DriftCircleKey}' ),
                   ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.AveIntPerXDecor' ),
                   ]

    # These objects must be loaded from SGIL if not from CondInputLoader

    if not flags.Input.isMC:
        dataObjects.append(( 'IDCInDetBSErrContainer' , 'StoreGateSvc+PixelByteStreamErrs' ))
        dataObjects.append(( 'TRT_RDO_Cache' , 
                             f'StoreGateSvc+{flags.Trigger.InDetTracking.TRTRDOCacheKey}'  ))
    else:
        dataObjects.append(( 'TRT_RDO_Container' , 'StoreGateSvc+TRT_RDOs' ))
        dataObjects.append(( 'InDet::TRT_DriftCircleContainerCache' , 
                             f'StoreGateSvc+{flags.Trigger.InDetTracking.TRT_DriftCircleCacheKey}' ))

    precisionGsfVDV.DataObjects =  dataObjects

    acc.addEventAlgo(precisionGsfVDV)

    from TriggerMenuMT.HLT.Electron.TrigEMBremCollectionBuilder import TrigEMBremCollectionBuilderCfg

    ## TrigEMBremCollectionBuilder ##

    acc.merge(TrigEMBremCollectionBuilderCfg(flags,
                                             name = "TrigEMBremCollectionBuilderCfg"+variant,
                                             TrackParticleContainerName=TrigEgammaKeys.precisionTrackingContainer,
                                             SelectedTrackParticleContainerName=TrigEgammaKeys.precisionTrackingContainer,
                                             OutputTrkPartContainerName=TrigEgammaKeys.precisionElectronTrackParticleContainerGSF,
                                             OutputTrackContainerName=TrigEgammaKeys.precisionElectronTrkCollectionGSF))

    return acc
