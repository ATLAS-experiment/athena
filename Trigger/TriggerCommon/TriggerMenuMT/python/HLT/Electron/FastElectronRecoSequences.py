#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

#logging
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def fastElectronRecoSequence(flags, name, RoIs, variant=''):
    
    acc = ComponentAccumulator()

    TrigEgammaKeys = getTrigEgammaKeys(flags, variant)
    trackParticlesName = TrigEgammaKeys.fastTrackParticleContainer  

    # A simple algorithm to confirm that data has been inherited from parent view
    # Required to satisfy data dependencies

    fastElectronVDV = CompFactory.AthViews.ViewDataVerifier(name+variant+'VDV')
    from TriggerMenuMT.HLT.CommonSequences.CaloSequences import CaloMenuDefs

    dataObjects = [( 'xAOD::TrigEMClusterContainer' , 'StoreGateSvc+%s' % CaloMenuDefs.L2CaloClusters ),
                   ( 'xAOD::TrackParticleContainer' , 'StoreGateSvc+%s' % trackParticlesName ),
                   ( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % RoIs ),
                   ]

    fastElectronVDV.DataObjects = dataObjects                                 
    acc.addEventAlgo(fastElectronVDV)

    from TrigEgammaRec.TrigEgammaFastElectronConfig import fastElectronFexAlgCfg

    trigEMClusterName = CaloMenuDefs.L2CaloClusters
    trackParticlesName = trackParticlesName
    electronsName = TrigEgammaKeys.fastElectronContainer

    theElectronFex = fastElectronFexAlgCfg(flags, trigEMClusterName, trackParticlesName, electronsName, name="EgammaFastElectronFex_Clean_gen"+variant, rois=RoIs)
    
    acc.merge(theElectronFex)
    return acc
