#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
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

    signatureName = 'electronLRT' if 'LRT' in variant else 'electron'
    from TrigInDetConfig.utils import getFlagsForActiveConfig
    trkflags = getFlagsForActiveConfig(flags, signatureName, log)

    precisionGsfVDV = CompFactory.AthViews.ViewDataVerifier("PrecisionTrackViewDataVerifier_forGSFRefit"+tag+'VDV')

    # precision Tracking related data dependencies
    trackParticles = TrigEgammaKeys.precisionTrackingContainer
    ambimap = flags.Trigger.InDetTracking.ClusterAmbiguitiesMap
    if flags.Detector.GeometryITk:
        ambimap = flags.Trigger.ITkTracking.ClusterAmbiguitiesMap

    dataObjects = [( 'xAOD::TrackParticleContainer','StoreGateSvc+%s' % trackParticles),
                   ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing' ),
                   ( 'InDet::PixelGangedClusterAmbiguities' , 'StoreGateSvc+%s' % ambimap ),
                   ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.AveIntPerXDecor' ),
                   ]

    if flags.Trigger.useActsTracking and flags.Acts.GsfRefitActs:
        dataObjects += [('ActsTrk::GeometryContext' , 'StoreGateSvc+ActsAlignment' ),
                        ( 'xAOD::TrackParticleContainer','StoreGateSvc+%s.actsTrack' % trackParticles)]
 

    
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

    ## EMBremCollectionBuilder ##
    if flags.Acts.GsfRefitActs:
        from egammaAlgs.ActsEMBremCollectionBuilderConfig import (
            TrigActsEMBremCollectionBuilderCfg)
        acc.merge(TrigActsEMBremCollectionBuilderCfg(trkflags, name='TrigActsEMBremCollectionBuilder'+variant,
                                                     RefittedTracksLocation = TrigEgammaKeys.precisionElectronTrkCollectionGSF,
                                                     SelectedTrackParticleContainerName = trackParticles,
                                                     TrackParticlesOutKey = TrigEgammaKeys.precisionElectronTrackParticleContainerGSF))

    else:

        from TriggerMenuMT.HLT.Electron.TrigEMBremCollectionBuilder import TrigEMBremCollectionBuilderCfg
        acc.merge(TrigEMBremCollectionBuilderCfg(trkflags,
                                                 name = "TrigEMBremCollectionBuilderCfg"+variant,
                                                 TrackParticleContainerName=TrigEgammaKeys.precisionTrackingContainer,
                                                 SelectedTrackParticleContainerName=TrigEgammaKeys.precisionTrackingContainer,
                                                 OutputTrkPartContainerName=TrigEgammaKeys.precisionElectronTrackParticleContainerGSF,
                                                 OutputTrackContainerName=TrigEgammaKeys.precisionElectronTrkCollectionGSF))

    return acc
