# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigEventAthenaPool )

# Component(s) in the package:
atlas_add_poolcnv_library( TrigEventAthenaPoolPoolCnv
   src/*.h src/*.cxx
   FILES TrigDecisionEvent/TrigDecision.h
         TrigInDetEvent/TrigInDetTrackCollection.h
         TrigInDetEvent/TrigVertexCollection.h
         TrigInDetEvent/TrigHisto1D.h TrigInDetEvent/TrigHisto2D.h
         TrigInDetEvent/TrigSpacePointCounts.h
         TrigInDetEvent/TrigSpacePointCountsCollection.h
         TrigInDetEvent/TrigTrtHitCounts.h
         TrigInDetEvent/TrigTrtHitCountsCollection.h
         TrigInDetEvent/TrigTrackCounts.h
         TrigInDetEvent/TrigTrackCountsCollection.h
         TrigInDetEvent/TrigVertexCounts.h
         TrigInDetEvent/TrigVertexCountsCollection.h
         TrigCaloEvent/TrigEMCluster.h
         TrigCaloEvent/TrigEMClusterContainer.h TrigCaloEvent/TrigTauCluster.h
         TrigCaloEvent/TrigTauClusterContainer.h
         TrigCaloEvent/TrigTauClusterDetails.h
         TrigCaloEvent/TrigTauClusterDetailsContainer.h
         TrigCaloEvent/TrigT2MbtsBits.h TrigCaloEvent/TrigT2MbtsBitsContainer.h
         TrigCaloEvent/TrigT2ZdcSignals.h
         TrigCaloEvent/TrigT2ZdcSignalsContainer.h
         TrigCaloEvent/RingerRings.h
         TrigCaloEvent/RingerRingsContainer.h TrigCaloEvent/TrigRNNOutput.h
         TrigCaloEvent/TrigRNNOutputContainer.h TrigCaloEvent/TrigCaloCluster.h
         TrigCaloEvent/TrigCaloClusterContainer.h
   TYPES_WITH_NAMESPACE TrigDec::TrigDecision
   LINK_LIBRARIES AthenaPoolUtilities GaudiKernel AthenaPoolCnvSvcLib
                  RootConversions TrigCaloEvent
                  TrigCaloEventTPCnv
                  TrigDecisionEvent TrigDecisionEventTPCnv TrigInDetEvent
                  TrigInDetEventTPCnv )
