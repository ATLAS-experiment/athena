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
    ambimap = flags.Trigger.InDetTracking.ClusterAmbiguitiesMap
    if flags.Detector.GeometryITk:
        ambimap = flags.Trigger.ITkTracking.ClusterAmbiguitiesMap

    dataObjects = [( 'xAOD::TrackParticleContainer','StoreGateSvc+%s' % trackParticles),
                   # verifier object needed by GSF
                   ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing' ),
                   ( 'InDet::PixelGangedClusterAmbiguities' , 'StoreGateSvc+%s' % ambimap ),
                   ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.AveIntPerXDecor' ),
                   ]

    if flags.Detector.GeometryTRT:
        dataObjects +=  [( 'InDet::TRT_DriftCircleContainer' , 'StoreGateSvc+%s' % "TRT_TrigDriftCircles" )]
        if flags.Input.isMC:
            dataObjects += [( 'TRT_RDO_Container' , 'StoreGateSvc+TRT_RDOs' ),
                            ( 'InDet::TRT_DriftCircleContainerCache' , 
                            f'StoreGateSvc+{flags.Trigger.InDetTracking.TRT_DriftCircleCacheKey}' )]
        else:
            dataObjects += [( 'TRT_RDO_Cache' , f'StoreGateSvc+{flags.Trigger.InDetTracking.TRTRDOCacheKey}' )]

    # These objects must be loaded from SGIL if not from CondInputLoader

    if not flags.Input.isMC:
        dataObjects.append(( 'IDCInDetBSErrContainer' , 'StoreGateSvc+PixelByteStreamErrs' ))

    from TrigInDetConfig.TrigInDetConfig import InDetExtraDataObjectsFromDataPrep
    InDetExtraDataObjectsFromDataPrep(flags,dataObjects)

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
