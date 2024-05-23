/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGFPGATrackSimOBJECTS_CONSTANTS_H
#define TRIGFPGATrackSimOBJECTS_CONSTANTS_H


#include <array>
#define MTX_TOLERANCE 3e-16

namespace fpgatracksim {

  // Default set of q/pt bins, maybe still referenced by some legacy code?
  constexpr std::array< double, 5 > QOVERPT_BINS = { -0.001, -0.0005, 0, 0.0005, 0.001};

  // --- This is the current FPGATrackSimCluster to FPGATrackSimHit scaling factor --- //
  constexpr float scaleHitFactor = 2;
  constexpr float DEG_TO_RAD = 0.017453292519943295;


  constexpr int SPACEPOINT_SECTOR_OFFSET = 1;
  constexpr int QPT_SECTOR_OFFSET = 10;
  constexpr int SUBREGION_SECTOR_OFFSET = 1000;
  constexpr int ETA_SECTOR_OFFSET = 100000;

  
  static constexpr double A = 0.0003; // for Hough Transform
  
}

#endif // TRIGFPGATrackSimOBJECTS_CONSTANTS_H
