#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
#logging
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

def precisionPhotonCaloIsoVDVCfg(flags, name, InViewRoIs, ion=False):
    acc = ComponentAccumulator()
    TrigEgammaKeys = getTrigEgammaKeys(flags, ion=ion)
    caloClusters = TrigEgammaKeys.precisionPhotonCaloClusterContainer
    dataObjects = [( 'xAOD::CaloClusterContainer' , 'StoreGateSvc+%s' % caloClusters ),
                   ( 'xAOD::CaloClusterContainer' , 'StoreGateSvc+%s' % TrigEgammaKeys.precisionTopoClusterContainer), # this is for the calo isolation tool
                   ( 'xAOD::PhotonContainer' , 'StoreGateSvc+%s' % TrigEgammaKeys.precisionPhotonContainer), # This is the Photon input container with non-isolated photons
                   ( 'CaloCellContainer' , 'StoreGateSvc+CaloCells' ),
                   ( 'CaloCellContainer' , 'StoreGateSvc+CaloCellsFS' ),
                   ( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' ),
                   ( 'xAOD::EventShape' , 'StoreGateSvc+TrigIsoEventShape' ),
                   ( 'xAOD::IParticleContainer' , 'StoreGateSvc+HLT_TopoCaloClustersFS'),
                   ( 'PseudoJetContainer' , 'StoreGateSvc+PseudoJetTrigEMTopo' )]
    if ion:
        dataObjects += [( 'CaloCellContainer' , 'StoreGateSvc+CorrectedRoICaloCells' )]

    precisionPhotonCaloIsoVDV = CompFactory.AthViews.ViewDataVerifier(name)
    precisionPhotonCaloIsoVDV.DataObjects = dataObjects
    acc.addEventAlgo(precisionPhotonCaloIsoVDV)
    return acc

def precisionPhotonCaloIsoRecoSequence(flags, RoIs,  name = None, ion=False):
    """ With this function we will setup the sequence of Calo isolation to be executed after PrecisionPhoton in  TrigEgamma 

    """
    acc= ComponentAccumulator()

    log.debug('precisionPhotonCaloIsoRecoSequence(RoIs = %s)',RoIs)

    log.debug('retrieve(precisionPhotonCaloIsoRecoSequence,None,RoIs = %s)',RoIs)

    acc.merge(precisionPhotonCaloIsoVDVCfg(flags, name+'VDV',RoIs,ion))

    # Add CaloIsolationTool
    from TriggerMenuMT.HLT.Egamma.TrigEgammaFactoriesCfg import TrigPhotonIsoBuilderCfg
    TrigPhotonIsoBuilder = TrigPhotonIsoBuilderCfg(flags,
                                                   ion =ion)

    acc.merge(TrigPhotonIsoBuilder)

    #online monitoring for xAODEgammaBuilder
    from TriggerMenuMT.HLT.Photon.TrigPhotonFactoriesCfg import PrecisionPhotonCaloIsoMonitorCfg
    PrecisionPhotonCaloIsoRecoMonAlgo = PrecisionPhotonCaloIsoMonitorCfg(flags)
    acc.merge(PrecisionPhotonCaloIsoRecoMonAlgo)

    return acc





