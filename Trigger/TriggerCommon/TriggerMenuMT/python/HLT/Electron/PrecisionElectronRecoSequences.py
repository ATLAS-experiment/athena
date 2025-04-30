#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

#logging
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

def precisionElectronRecoSequence(flags, RoIs, ion=False, doGSF=True, doLRT=False):

    acc = ComponentAccumulator()
    """ With this function we will setup the sequence of offline EgammaAlgorithms so to make a electron for TrigEgamma 

    Sequence of algorithms is the following:
      - egammaRecBuilder/TrigEgammaRecElectron creates egammaObjects out of clusters and tracks. 
      - electronSuperClusterBuilder algorithm will create superclusters out of the topoclusters and tracks in egammaRec under the electron hypothesis
          https://gitlab.cern.ch/atlas/athena/blob/master/Reconstruction/egamma/egammaAlgs/python/egammaSuperClusterBuilder.py#L26 
      - TopoEgammBuilder will create photons and electrons out of trakcs and SuperClusters. Here at HLT electrons the aim is to ignore photons.
          https://gitlab.cern.ch/atlas/athena/blob/master/Reconstruction/egamma/egammaAlgs/src/xAODEgammaBuilder.cxx
    """

    log.debug('precisionElectronRecoSequence(RoIs = %s, ion = %s, doGSF = %s, doLRT = %s)',RoIs,ion,doGSF,doLRT)

    tag = '_ion' if ion is True else ''

    # create a Variant string out of the options above
    variant = '_'
    if doLRT:
        variant+='LRT'
    if doGSF:
        variant+='GSF'

    if not doLRT and not doGSF:
        variant+='noGSF'

    tag += variant
       
    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import  getTrigEgammaKeys

    # makes datakeys based on LRT, GSF, noGSF, LRTGSF            
    TrigEgammaKeys = getTrigEgammaKeys(flags, variant, ion=ion)
    
    # taking care of VDV before GSF related data comes in
    if doLRT:
        TrigEgammaKeys_noGSF = getTrigEgammaKeys(flags, '_LRT')
    else:
        TrigEgammaKeys_noGSF = getTrigEgammaKeys(flags) 
    
    # following reduces if-else checking for later implementations
    if doGSF:
        useBremAssoc = True
        trackParticles = TrigEgammaKeys.precisionElectronTrackParticleContainerGSF
    else:
        useBremAssoc = False
        trackParticles = TrigEgammaKeys_noGSF.precisionTrackingContainer 

    # Arranging required data keys
    caloClusters = TrigEgammaKeys.precisionElectronCaloClusterContainer
    egammaRecContainer = TrigEgammaKeys.precisionEgammaRecCollection
    superElectronRecCollectionName = TrigEgammaKeys.precisionElectronSuperClusterCollection
    superPhotonRecCollectionName = TrigEgammaKeys.precisionPhotonSuperClusterCollection
    photonOutputName = TrigEgammaKeys.precisionPhotonContainer
    trackParticles_noGSF = TrigEgammaKeys_noGSF.precisionTrackingContainer
    TrackParticleLocation = TrigEgammaKeys.precisionTrackingContainer
    electronOutputName = TrigEgammaKeys.precisionElectronContainer
    #electronCollectionContainerName_noGSF = TrigEgammaKeys_noGSF.precisionElectronContainer
    OutputClusterContainerName = TrigEgammaKeys.precisionElectronEMClusterContainer
    #useBremAssoc = True

    cellsName = "CaloCells" if not ion else "CorrectedRoICaloCells"
    dataObjects = [( 'CaloCellContainer' , 'StoreGateSvc+%s' % cellsName ),
                   ( 'xAOD::CaloClusterContainer' , 'StoreGateSvc+%s' % caloClusters ),
                   ( 'xAOD::TrackParticleContainer','StoreGateSvc+%s' % trackParticles_noGSF)]
    if doGSF:
        dataObjects += [
                         # verifier object needed by GSF
                         ( 'xAOD::TrackParticleContainer','StoreGateSvc+%s' % trackParticles),
                         ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing' ),
                         ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.AveIntPerXDecor' )]

        if flags.Detector.GeometryTRT:
           dataObjects +=  [( 'InDet::TRT_DriftCircleContainer' , 'StoreGateSvc+%s' % "TRT_TrigDriftCircles" )]
           if flags.Input.isMC:
               dataObjects += [( 'TRT_RDO_Container' , 'StoreGateSvc+TRT_RDOs' )]
           else:
               dataObjects += [( 'TRT_RDO_Cache' , f'StoreGateSvc+{flags.Trigger.InDetTracking.TRTRDOCacheKey}' )]

        ambimap = flags.Trigger.InDetTracking.ClusterAmbiguitiesMap
        if flags.Detector.GeometryITk:
            ambimap = flags.Trigger.ITkTracking.ClusterAmbiguitiesMap

        dataObjects +=  [('InDet::PixelGangedClusterAmbiguities' , 'StoreGateSvc+%s' % ambimap )]

        if not flags.Input.isMC:
            dataObjects += [( 'IDCInDetBSErrContainer' , 'StoreGateSvc+PixelByteStreamErrs' )]

    precisionElectronVDV = CompFactory.AthViews.ViewDataVerifier("precisionElectron"+tag+"VDV")
    precisionElectronVDV.DataObjects = dataObjects
    acc.addEventAlgo(precisionElectronVDV)

    """ Retrieve the factories now """
    from TriggerMenuMT.HLT.Electron.TrigElectronFactoriesCfg import TrigEgammaRecElectronCfg, TrigElectronSuperClusterBuilderCfg, TrigTopoEgammaElectronCfg, TrigElectronIsoBuilderCfg
   
    # Create the sequence of steps:
    #  - TrigEgammaRecElectron, TrigElectronSuperClusterBuilder, TrigTopoEgammaElectron
    #The sequence of these algorithms
    
    ## TrigEgammaRecElectron ##
    TrigEgammaRecAlgo = TrigEgammaRecElectronCfg(flags, tag, trackParticles, caloClusters, egammaRecContainer)
    acc.merge(TrigEgammaRecAlgo)

    ## TrigElectronSuperClusterBuilder ##
    TrigSuperElectronAlgo = TrigElectronSuperClusterBuilderCfg(flags, tag, egammaRecContainer, superElectronRecCollectionName,trackParticles)
    acc.merge(TrigSuperElectronAlgo)    
    
    ## TrigTopoEgammaElectronCfg ##
    TrigTopoEgammaAlgo = TrigTopoEgammaElectronCfg(flags, tag, variant, cellsName, superElectronRecCollectionName, superPhotonRecCollectionName, electronOutputName, photonOutputName,OutputClusterContainerName)

    acc.merge(TrigTopoEgammaAlgo)
    
    collectionOut = electronOutputName
    
    ## TrigElectronIsoBuilderCfg ##
    isoBuilder = TrigElectronIsoBuilderCfg(flags, tag, TrackParticleLocation, electronOutputName, useBremAssoc)
    acc.merge(isoBuilder)

    isoVarKeys = [ '%s.ptcone20' % collectionOut,
                   '%s.ptvarcone20' % collectionOut,
                   '%s.ptcone30' % collectionOut,
                   '%s.ptvarcone30' % collectionOut ]

    #online monitoring for topoEgammaBuilder_GSF
    from TriggerMenuMT.HLT.Electron.TrigElectronFactoriesCfg import PrecisionElectronTopoMonitorCfg
    PrecisionElectronRecoMonAlgo = PrecisionElectronTopoMonitorCfg(flags, tag, collectionOut, isoVarKeys)

    acc.merge(PrecisionElectronRecoMonAlgo)

    #online monitoring for TrigElectronSuperClusterBuilder
    from TriggerMenuMT.HLT.Electron.TrigElectronFactoriesCfg import PrecisionElectronSuperClusterMonitorCfg
    PrecisionElectronSuperClusterMonAlgo = PrecisionElectronSuperClusterMonitorCfg(flags, tag, superElectronRecCollectionName)
 
    acc.merge(PrecisionElectronSuperClusterMonAlgo)

    return acc


