/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// generate the T/P converter entries
#include "AthenaKernel/TPCnvFactory.h"

#include "G4SimTPCnv/TrackRecordCollectionCnv_p1.h"
#include "G4SimTPCnv/TrackRecordCollectionCnv_p2.h"
#include "G4SimTPCnv/TrackRecordCollectionCnv_p3.h"
#include "G4SimTPCnv/TrackRecord_p1.h"
#include "G4SimTPCnv/TrackRecordCollection_p1.h"
#include "G4SimTPCnv/TrackRecordCollection_p2.h"
#include "G4SimTPCnv/TrackRecordCollection_p3.h"

DECLARE_NAMED_TPCNV_FACTORY(TrackRecordCollectionCnv_p1,
                            TrackRecordCollectionCnv_p1,
                            AtlasHitsVector<TrackRecord>,
                            TrackRecordCollection_p1,
                            Athena::TPCnvVers::Old)

DECLARE_NAMED_TPCNV_FACTORY(TrackRecordCollectionCnv_p2,
                            TrackRecordCollectionCnv_p2,
                            AtlasHitsVector<TrackRecord>,
                            TrackRecordCollection_p2,
                            Athena::TPCnvVers::Old)

DECLARE_NAMED_TPCNV_FACTORY(TrackRecordCollectionCnv_p3,
                            TrackRecordCollectionCnv_p3,
                            AtlasHitsVector<TrackRecord>,
                            TrackRecordCollection_p3,
                            Athena::TPCnvVers::Current)
