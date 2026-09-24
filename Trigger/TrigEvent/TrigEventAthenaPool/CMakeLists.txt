# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigEventAthenaPool )

# Component(s) in the package:
atlas_add_poolcnv_library( TrigEventAthenaPoolPoolCnv
   src/*.h src/*.cxx
   FILES TrigParticle/TrigL2BphysContainer.h
         TrigParticle/TrigElectronContainer.h TrigParticle/TrigTauContainer.h
         TrigParticle/TrigTau.h TrigParticle/TrigL2BjetContainer.h
         TrigParticle/TrigEFBjetContainer.h TrigParticle/TrigPhotonContainer.h
         TrigParticle/TrigEFBphysContainer.h TrigDecisionEvent/TrigDecision.h
         TrigInDetEvent/TrigInDetTrackCollection.h
         TrigInDetEvent/TrigVertexCollection.h
         TrigInDetEvent/TrigTauTracksInfo.h
         TrigInDetEvent/TrigTauTracksInfoCollection.h
         TrigInDetEvent/TrigHisto1D.h TrigInDetEvent/TrigHisto2D.h
         TrigInDetEvent/TrigSpacePointCounts.h
         TrigInDetEvent/TrigSpacePointCountsCollection.h
         TrigInDetEvent/TrigTrtHitCounts.h
         TrigInDetEvent/TrigTrtHitCountsCollection.h
         TrigInDetEvent/TrigTrackCounts.h
         TrigInDetEvent/TrigTrackCountsCollection.h
         TrigInDetEvent/TrigVertexCounts.h
         TrigInDetEvent/TrigVertexCountsCollection.h
         TrigMuonEvent/MuonFeature.h
         TrigMuonEvent/MuonFeatureContainer.h TrigMuonEvent/MuonFeatureDetails.h
         TrigMuonEvent/MuonFeatureDetailsContainer.h
         TrigMuonEvent/CombinedMuonFeature.h
         TrigMuonEvent/CombinedMuonFeatureContainer.h
         TrigMuonEvent/IsoMuonFeature.h TrigMuonEvent/IsoMuonFeatureContainer.h
         TrigMuonEvent/TrigMuonClusterFeature.h
         TrigMuonEvent/TrigMuonClusterFeatureContainer.h
         TrigMuonEvent/TileMuFeature.h TrigMuonEvent/TileMuFeatureContainer.h
         TrigMuonEvent/TileTrackMuFeature.h
         TrigMuonEvent/TileTrackMuFeatureContainer.h
         TrigMuonEvent/TrigMuonEFContainer.h
         TrigMuonEvent/TrigMuonEFInfoContainer.h
         TrigMuonEvent/TrigMuonEFIsolationContainer.h
         TrigMuonEvent/TrigCombDiMuonContainer.h
         TrigCaloEvent/TrigEMCluster.h
         TrigCaloEvent/TrigEMClusterContainer.h TrigCaloEvent/TrigTauCluster.h
         TrigCaloEvent/TrigTauClusterContainer.h
         TrigCaloEvent/TrigTauClusterDetails.h
         TrigCaloEvent/TrigTauClusterDetailsContainer.h
         TrigCaloEvent/TrigT2MbtsBits.h TrigCaloEvent/TrigT2MbtsBitsContainer.h
         TrigCaloEvent/TrigT2ZdcSignals.h
         TrigCaloEvent/TrigT2ZdcSignalsContainer.h TrigCaloEvent/TrigT2Jet.h
         TrigCaloEvent/TrigT2JetContainer.h
         TrigCaloEvent/RingerRings.h
         TrigCaloEvent/RingerRingsContainer.h TrigCaloEvent/TrigRNNOutput.h
         TrigCaloEvent/TrigRNNOutputContainer.h TrigCaloEvent/TrigCaloCluster.h
         TrigCaloEvent/TrigCaloClusterContainer.h
         TrigTopoEvent/ElectronMuonTopoInfo.h
         TrigTopoEvent/ElectronMuonTopoInfoContainer.h
         TrigMonitoringEvent/TrigMonEvent.h
         TrigMonitoringEvent/TrigMonEventCollection.h
         TrigMonitoringEvent/TrigMonConfig.h
         TrigMonitoringEvent/TrigMonConfigCollection.h 
   TYPES_WITH_NAMESPACE TrigDec::TrigDecision
   LINK_LIBRARIES AthenaPoolUtilities GaudiKernel AthenaPoolCnvSvcLib
                  RootConversions TrigCaloEvent
                  TrigCaloEventTPCnv
                  TrigDecisionEvent TrigDecisionEventTPCnv TrigInDetEvent
                  TrigInDetEventTPCnv
                  TrigMonitoringEvent TrigMonitoringEventTPCnv TrigMuonEvent
                  TrigMuonEventTPCnv TrigParticle TrigParticleTPCnv
                  TrigTopoEvent TrigTopoEventTPCnv )
