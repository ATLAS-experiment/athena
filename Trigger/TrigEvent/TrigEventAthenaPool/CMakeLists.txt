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
         TrigInDetEvent/TrigTrackCounts.h
         TrigInDetEvent/TrigTrackCountsCollection.h
         TrigInDetEvent/TrigVertexCounts.h
         TrigInDetEvent/TrigVertexCountsCollection.h
   TYPES_WITH_NAMESPACE TrigDec::TrigDecision
   LINK_LIBRARIES AthenaPoolUtilities GaudiKernel AthenaPoolCnvSvcLib
                  RootConversions
                  TrigDecisionEvent TrigDecisionEventTPCnv TrigInDetEvent
                  TrigInDetEventTPCnv )
