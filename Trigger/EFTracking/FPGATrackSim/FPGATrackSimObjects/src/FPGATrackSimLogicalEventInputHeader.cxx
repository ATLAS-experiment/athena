/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"




void FPGATrackSimLogicalEventInputHeader::reset() const
{
  m_optional.reset();
  m_towers.clear();
}


std::ostream& operator<<(std::ostream& s, const FPGATrackSimLogicalEventInputHeader& h)
{
  s << "Event: " << h.event() << "\t"
    << "Optional: " << h.optional() << "\t"
    << "NTowers: " << h.nTowers() << "\n";

  const std::vector<FPGATrackSimTowerInputHeader>& towers = h.towers();
  for (int j = 0; j < h.nTowers(); j++)
  {
    s << " " << j << "  " << towers[j] << "\n";
  }
  s << std::endl;

  return s;
}

