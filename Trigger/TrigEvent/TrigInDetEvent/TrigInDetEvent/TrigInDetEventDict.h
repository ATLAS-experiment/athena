/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIG_INDET_DICT_H
#define TRIG_INDET_DICT_H

#include "TrigInDetEvent/TrigInDetTrack.h"
#include "TrigInDetEvent/TrigInDetTrackCollection.h"
#include "TrigInDetEvent/TrigVertex.h"
#include "TrigInDetEvent/TrigVertexCollection.h"
#include "TrigInDetEvent/TrigVertexCounts.h"
#include "TrigInDetEvent/TrigVertexCountsCollection.h"

//namespace HLT::TrigInDetEventDict {
void dummyTriggerForTVCollTypedef(TrigVertexCollection a, 
				  DataVector<TrigInDetTrackCollection> b,
				  DataVector<TrigInDetTrackFitPar> c,
				  DataVector<TrigVertexCollection> d,
                                  DataVector<TrigVertexCounts> q,
                                  DataVector<TrigVertexCountsCollection> r) {
  TrigVertexCollection aa = a;
  DataVector<TrigInDetTrackCollection> bb = b; 
  DataVector<TrigInDetTrackFitPar> cc = c;
  DataVector<TrigVertexCollection> dd = d;
  DataVector<TrigVertexCounts> qq = q;
  DataVector<TrigVertexCountsCollection> rr = r;
}

#endif
