/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// generate the T/P converter entries
#include "AthenaKernel/TPCnvFactory.h"

#include "TrigInDetEventTPCnv/TrigInDetTrackCollection_tlp2.h"
#include "TrigInDetEventTPCnv/TrigInDetTrackCollectionCnv_tlp2.h"

#include "TrigInDetEventTPCnv/TrigInDetTrackCollection_tlp3.h"
#include "TrigInDetEventTPCnv/TrigInDetTrackCollectionCnv_tlp3.h"

#include "TrigInDetEventTPCnv/TrigInDetTrackCollection_tlp4.h"
#include "TrigInDetEventTPCnv/TrigInDetTrackCollectionCnv_tlp4.h"

#include "TrigInDetEventTPCnv/TrigInDetTrackCollection_tlp1.h"
#include "TrigInDetEventTPCnv/TrigInDetTrackCollectionCnv_tlp1.h"
#include "TrigInDetEventTPCnv/TrigInDetTrackCollection_p1.h"
#include "TrigInDetEventTPCnv/TrigInDetTrack_p1.h"
#include "TrigInDetEventTPCnv/TrigInDetTrack_p2.h"
#include "TrigInDetEventTPCnv/TrigInDetTrack_p3.h"
#include "TrigInDetEventTPCnv/TrigInDetTrackFitPar_p1.h"
#include "TrigInDetEventTPCnv/TrigInDetTrackFitPar_p2.h"
#include "TrigInDetEventTPCnv/TrigInDetTrackFitPar_p3.h"

#include "TrigInDetEventTPCnv/TrigVertexCounts_p1.h"
#include "TrigInDetEventTPCnv/TrigVertexCountsCollection_p1.h"
#include "TrigInDetEventTPCnv/TrigVertexCountsCollection_tlp1.h"
#include "TrigInDetEventTPCnv/TrigVertexCountsCollectionCnv_tlp1.h"
#include "TrigInDetEventTPCnv/TrigVertexCountsCnv_p1.h"

#include "TrigInDetEventTPCnv/TrigVertexCollection_tlp1.h"
#include "TrigInDetEventTPCnv/TrigVertexCollectionCnv_tlp1.h"
#include "TrigInDetEventTPCnv/TrigVertexCollection_p1.h"
#include "TrigInDetEventTPCnv/TrigVertex_p1.h"
#include "TrigInDetEventTPCnv/TrigVertexCollection_tlp2.h"
#include "TrigInDetEventTPCnv/TrigVertexCollectionCnv_tlp2.h"
#include "TrigInDetEventTPCnv/TrigVertex_p2.h"

DECLARE_TPCNV_FACTORY(TrigInDetTrackCollectionCnv_tlp1,
                      TrigInDetTrackCollection,
                      TrigInDetTrackCollection_tlp1,
                      Athena::TPCnvVers::Old)
                      
DECLARE_TPCNV_FACTORY(TrigInDetTrackCollectionCnv_tlp2,
                      TrigInDetTrackCollection,
                      TrigInDetTrackCollection_tlp2,
                      Athena::TPCnvVers::Old)

DECLARE_TPCNV_FACTORY(TrigInDetTrackCollectionCnv_tlp3,
                      TrigInDetTrackCollection,
                      TrigInDetTrackCollection_tlp3,
                      Athena::TPCnvVers::Old) 
                      
DECLARE_TPCNV_FACTORY(TrigInDetTrackCollectionCnv_tlp4,
                      TrigInDetTrackCollection,
                      TrigInDetTrackCollection_tlp4,
                      Athena::TPCnvVers::Current) 
                      
DECLARE_TPCNV_FACTORY(TrigVertexCountsCnv_p1,
                      TrigVertexCounts,
                      TrigVertexCounts_p1,
                      Athena::TPCnvVers::Current)

DECLARE_TPCNV_FACTORY(TrigVertexCountsCollectionCnv_tlp1,
                      TrigVertexCountsCollection,
                      TrigVertexCountsCollection_tlp1,
                      Athena::TPCnvVers::Current)
 
DECLARE_TPCNV_FACTORY(TrigVertexCollectionCnv_tlp1,
                      TrigVertexCollection,
                      TrigVertexCollection_tlp1,
                      Athena::TPCnvVers::Old)

DECLARE_TPCNV_FACTORY(TrigVertexCollectionCnv_tlp2,
                      TrigVertexCollection,
                      TrigVertexCollection_tlp2,
                      Athena::TPCnvVers::Current)
