/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimObjects/FPGATrackSimEventInputHeader.h"
#include <iostream>


FPGATrackSimEventInputHeader::~FPGATrackSimEventInputHeader()
{
  reset();
}

void FPGATrackSimEventInputHeader::reset()
{
  m_optional.reset();
  m_Hits.clear();
}


std::ostream& operator<<(std::ostream& s, const FPGATrackSimEventInputHeader& h) {
  s << "Event: " << h.event() << "\t"
    << "Optional: " << h.optional() << "\t"
    << "Nhits=" << h.nHits();

  s << "\n";

  const std::vector<FPGATrackSimHit>& hits = h.hits();
  for (int j = 0; j < h.nHits(); j++) {
    s << " " << j << "  " << hits[j] << "\n";
    //    if ( (j+1)%5==0 ) s << "\n";
  }
  //s <<"\n";


  return s;
}

