/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-
#ifndef TRACKTRUTHTPCNV_TRACKTRUTHCOLLECTIONCNV_H
#define TRACKTRUTHTPCNV_TRACKTRUTHCOLLECTIONCNV_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"

#include "TrkTruthData/TrackTruthCollection.h"
#include "TrkTruthTPCnv/TrackTruthCollection_p3.h"
#include "TrkTruthTPCnv/TrackTruthCollection_p2.h"
#include "TrkTruthTPCnv/TrackTruthCollection_p1.h"
#include "TrkTruthTPCnv/TrackTruthCollectionCnv_p3.h"
#include "TrkTruthTPCnv/TrackTruthCollectionCnv_p2.h"
#include "TrkTruthTPCnv/TrackTruthCollectionCnv_p1.h"
#include "TrkTruthTPCnv/TrackTruthCollectionCnv_p0.h"

typedef Trk::TrackTruthCollection_p3 TrackTruthCollectionPERS;

typedef T_AthenaPoolCustomCnv<TrackTruthCollection, TrackTruthCollectionPERS> TrackTruthCollectionCnvBase;

class TrackTruthCollectionCnv : public TrackTruthCollectionCnvBase
{
  friend class CnvFactory<TrackTruthCollectionCnv>;
protected:
public:
   TrackTruthCollectionCnv(ISvcLocator* svcloc);
protected:
  virtual TrackTruthCollection* createTransient();
  virtual TrackTruthCollectionPERS* createPersistent(TrackTruthCollection*);
private:
  TrackTruthCollectionCnv_p0 m_converter_p0;
  TrackTruthCollectionCnv_p1 m_converter_p1;
  TrackTruthCollectionCnv_p2 m_converter_p2;
  TrackTruthCollectionCnv_p3 m_converter_p3;

};

#endif // TRACKTRUTHTPCNV_TRACKTRUTHCOLLECTIONCNV_H
