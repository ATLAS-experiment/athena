/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*
 * FPGATrackSimLogicalEventOutputHeader_test.cxx: Unit tests for FPGATrackSimLogicalEventOutputHeader
 */

#include <cstdio>
#include <iostream>
#include <cassert>
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventOutputHeader.h"

int main(int, char**)
{
  // some random values for checking things, just put them all up here
  const int roadid(8675309);
  const double qoverpt(-0.123456), chi2(3.41159);

  FPGATrackSimLogicalEventOutputHeader header;

  FPGATrackSimTrack track;
  track.setQOverPt(qoverpt);
  track.setChi2(chi2);
  std::vector<FPGATrackSimTrack> trackvec;
  trackvec.push_back(track);
  header.addFPGATrackSimTracks_1st(trackvec);

  FPGATrackSimRoad road;
  road.setRoadID(roadid);
  std::vector<FPGATrackSimRoad> roadvec;
  roadvec.push_back(road);

  header.addFPGATrackSimTracks_1st(trackvec);
  header.addFPGATrackSimRoads_1st(roadvec);
  std::cout << "q/pt = " << header.getFPGATrackSimTracks_1st()[0].getQOverPt()  << " and chi2 = " << 
    header.getFPGATrackSimTracks_1st()[0].getChi2() << std::endl;

  const auto& roads_1st = header.getFPGATrackSimRoads_1st();
  std::cout << "Road ID = " << roads_1st[0].getRoadID() << std::endl;

  return 0;

}

