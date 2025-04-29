/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONEVENTTPCNV_TILETRACKMUFEATURE_P2_H
#define TRIGMUONEVENTTPCNV_TILETRACKMUFEATURE_P2_H

#include "DataModelAthenaPool/ElementLink_p1.h"

class TileTrackMuFeature_p2 {
  friend class TileTrackMuFeatureCnv_p2;
    
 public:
    
  // default constructor
  TileTrackMuFeature_p2() = default;
  
  // default destructor
  virtual ~TileTrackMuFeature_p2() = default;

  float m_PtTR_Trk{};
  float m_EtaTR_Trk{};
  float m_PhiTR_Trk{};
  int   m_Typ_IDTrk{};

  ElementLinkInt_p1 m_TileMu;
  ElementLinkInt_p1 m_Track;
};

#endif
