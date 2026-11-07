/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIG_INDET_DICT_H
#define TRIG_INDET_DICT_H

#include "TrigInDetEvent/TrigInDetTrack.h"
#include "TrigInDetEvent/TrigInDetTrackCollection.h"
#include "TrigInDetEvent/TrigVertex.h"
#include "TrigInDetEvent/TrigVertexCollection.h"
#include "TrigInDetEvent/TrigHisto.h"
#include "TrigInDetEvent/TrigHisto1D.h"
#include "TrigInDetEvent/TrigHisto1DContainer.h"
#include "TrigInDetEvent/TrigHisto2D.h"
#include "TrigInDetEvent/TrigHisto2DContainer.h"
#include "TrigInDetEvent/TrigTrackCounts.h"
#include "TrigInDetEvent/TrigTrackCountsCollection.h"
#include "TrigInDetEvent/TrigVertexCounts.h"
#include "TrigInDetEvent/TrigVertexCountsCollection.h"

//namespace HLT::TrigInDetEventDict {
void dummyTriggerForTVCollTypedef(TrigVertexCollection a, 
				  DataVector<TrigInDetTrackCollection> b,
				  DataVector<TrigInDetTrackFitPar> c,
				  DataVector<TrigVertexCollection> d,
				  DataVector<TrigTrackCounts> g,
				  DataVector<TrigTrackCountsCollection> h,
                                  DataVector<TrigHisto1D> m,
                                  DataVector<TrigHisto1DContainer> n,
                                  DataVector<TrigHisto2D> o,
                                  DataVector<TrigHisto2DContainer> p,
                                  DataVector<TrigVertexCounts> q,
                                  DataVector<TrigVertexCountsCollection> r) {
  TrigVertexCollection aa = a;
  DataVector<TrigInDetTrackCollection> bb = b; 
  DataVector<TrigInDetTrackFitPar> cc = c;
  DataVector<TrigVertexCollection> dd = d;
  DataVector<TrigTrackCounts> gg = g;
  DataVector<TrigTrackCountsCollection> hh = h;
  DataVector<TrigHisto1D> mm = m; 
  DataVector<TrigHisto1DContainer> nn = n;
  DataVector<TrigHisto2D> oo = o;                                          
  DataVector<TrigHisto2DContainer> pp = p;
  DataVector<TrigVertexCounts> qq = q;
  DataVector<TrigVertexCountsCollection> rr = r;
}

#endif
