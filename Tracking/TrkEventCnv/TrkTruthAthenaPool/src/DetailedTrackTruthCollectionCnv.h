/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-
#ifndef TRKTRUTHATHENAPOOL_DETAILEDTRACKTRUTHCOLLECTIONCNV_H
#define TRKTRUTHATHENAPOOL_DETAILEDTRACKTRUTHCOLLECTIONCNV_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"

#include "TrkTruthData/DetailedTrackTruthCollection.h"
#include "TrkTruthTPCnv/DetailedTrackTruthCollectionCnv_p1.h"
#include "TrkTruthTPCnv/DetailedTrackTruthCollectionCnv_p2.h"
#include "TrkTruthTPCnv/DetailedTrackTruthCollectionCnv_p3.h"
#include "TrkTruthTPCnv/DetailedTrackTruthCollectionCnv_p4.h"
#include "TrkTruthTPCnv/DetailedTrackTruthCollection_p2.h"
#include "TrkTruthTPCnv/DetailedTrackTruthCollection_p3.h"
#include "TrkTruthTPCnv/DetailedTrackTruthCollection_p4.h"

typedef Trk::DetailedTrackTruthCollection_p3 DetailedTrackTruthCollectionPERS;

typedef T_AthenaPoolCustomCnv<DetailedTrackTruthCollection, DetailedTrackTruthCollectionPERS> DetailedTrackTruthCollectionCnvBase;

class DetailedTrackTruthCollectionCnv : public DetailedTrackTruthCollectionCnvBase
{
  friend class CnvFactory<DetailedTrackTruthCollectionCnv>;
protected:
public:
   DetailedTrackTruthCollectionCnv(ISvcLocator* svcloc);
protected:
  virtual DetailedTrackTruthCollection* createTransient();
  virtual DetailedTrackTruthCollectionPERS* createPersistent(DetailedTrackTruthCollection*);
private:
  static const pool::Guid s_p0_guid;
  static const pool::Guid s_p1_guid;
  static const pool::Guid s_p2_guid;
  static const pool::Guid s_p3_guid;
  static const pool::Guid s_p4_guid;
  DetailedTrackTruthCollectionCnv_p1 m_converter_p1;
  DetailedTrackTruthCollectionCnv_p2 m_converter_p2;
  DetailedTrackTruthCollectionCnv_p3 m_converter_p3;
  DetailedTrackTruthCollectionCnv_p4 m_converter_p4;
};

#endif/*TRKTRUTHATHENAPOOL_DETAILEDTRACKTRUTHCOLLECTIONCNV_H*/
