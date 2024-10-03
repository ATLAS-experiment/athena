/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimObjects/FPGATrackSimLogicalEventOutputHeader.h"


FPGATrackSimLogicalEventOutputHeader::~FPGATrackSimLogicalEventOutputHeader() {
  reset();
}

void FPGATrackSimLogicalEventOutputHeader::reset() {
  m_FPGATrackSimRoads_1st.clear();
  m_FPGATrackSimRoads_2nd.clear();
  m_FPGATrackSimTracks_1st.clear();
  m_FPGATrackSimTracks_2nd.clear();
  m_dataflowInfo.reset();
}

std::ostream& operator<<(std::ostream& s, FPGATrackSimLogicalEventOutputHeader const& h) {

  s << "NFPGATrackSimRoads_1st = " << h.nFPGATrackSimRoads_1st() << ", "
    << "NFPGATrackSimRoads_2nd = " << h.nFPGATrackSimRoads_2nd() << ", "
    << "NFPGATrackSimTracks_1st = " << h.nFPGATrackSimTracks_1st() << ", "
    << "NFPGATrackSimTracks_2nd = " << h.nFPGATrackSimTracks_2nd() << std::endl;

  return s;
}
