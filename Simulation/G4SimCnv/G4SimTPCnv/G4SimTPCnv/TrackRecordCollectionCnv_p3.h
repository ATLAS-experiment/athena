/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4SIMTPCNV_TRACKRECORDCOLLECTIONCNV_P3_H
#define G4SIMTPCNV_TRACKRECORDCOLLECTIONCNV_P3_H

#include "G4SimTPCnv/TrackRecordCollection_p3.h"
#include "TrackRecord/TrackRecordCollection.h"
#include "G4SimTPCnv/TrackRecordCnv_p2.h"
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

typedef T_AtlasHitsVectorCnv< TrackRecordCollection, TrackRecordCollection_p3, TrackRecordCnv_p2 >  TrackRecordCollectionCnv_p3;


#endif // not G4SIMTPCNV_TRACKRECORDCOLLECTIONCNV_P3_H
