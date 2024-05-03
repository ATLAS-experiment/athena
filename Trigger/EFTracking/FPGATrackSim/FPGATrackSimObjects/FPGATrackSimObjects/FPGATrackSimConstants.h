/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGFPGATrackSimOBJECTS_CONSTANTS_H
#define TRIGFPGATrackSimOBJECTS_CONSTANTS_H


#include <array>
#define MTX_TOLERANCE 3e-16

namespace fpgatracksim {

    // "Idealised" radius values for each of the detector barrel layers
  constexpr std::array< double, 9 > TARGET_R_1STAGE = { 291.3, 396.9, 403.2, 559.5, 565.5, 759.3, 765.3, 996.9, 1003};
  constexpr std::array< double, 13 > TARGET_R_2STAGE = { 33.9, 99.7, 160.4, 228.5, 291.3, 396.9, 403.2, 559.5, 565.5, 759.3, 765.3, 996.9, 1003};

  constexpr std::array< double, 16 > QOVERPT_BINS = { -0.001, -0.0005, 0, 0.0005, 0.001};
  
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
