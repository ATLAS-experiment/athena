/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimObjects/FPGATrackSimTowerInputHeader.h"
#include <iostream>


FPGATrackSimTowerInputHeader::FPGATrackSimTowerInputHeader(int id, double e, double p, double de, double dp) :
  m_id(id),
  m_Eta(e), m_Phi(p),
  m_DeltaEta(de), m_DeltaPhi(dp)
{
  m_Hits.clear();
}

void FPGATrackSimTowerInputHeader::reset()
{
  m_id = 0;
  m_Eta = 0.0;
  m_Phi = 0.0;
  m_DeltaEta = 0.0;
  m_DeltaPhi = 0.0;
  m_Hits.clear();
}


std::ostream& operator<<(std::ostream& s, const FPGATrackSimTowerInputHeader& r)
{

  s << "Tower: id=" << r.id() << "\t eta=" << r.eta() << "\t phi=" << r.phi()
    << "\t Nhits=" << r.nHits();

  return s;
}

