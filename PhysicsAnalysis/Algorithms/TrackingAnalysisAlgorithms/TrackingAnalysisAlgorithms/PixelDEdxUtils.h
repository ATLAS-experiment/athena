//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

#ifndef TRACKINGANALYSISALGORITHMS_PIXELDEDXUTILS_H
#define TRACKINGANALYSISALGORITHMS_PIXELDEDXUTILS_H

#include <map>
#include <vector>
#include <cmath>

namespace PixelDEdx {

  /// For charge -> dE/dx calc.
  constexpr float energyPair = 3.68e-6; // Energy in MeV to create an electron-hole pair in silicon
  constexpr float sidensity = 2.329; // silicon density in g cm^-3
  constexpr double conversionfactor=energyPair/sidensity; // use double to match old tool, trying to match precision when checking for differences. 

  constexpr float Pixel_sensorthickness=.025; // 250 microns Pixel Planars
  constexpr float IBL_3D_sensorthickness=.023; // 230 microns IBL 3D
  constexpr float IBL_PLANAR_sensorthickness=.020; // 200 microns IBL Planars
    
  struct PixelClusterStruct {  // Struct representing a pixel cluster to abstract away the two EDMs
    double locx = -99.9;
    double locy = -99.9;
    int bec = -99;
    int layer = -99;
    int eta_module = -99;
    float cosalpha = -99.9;
    float charge = -99.9;
    float dEdx = -99.9;
    float dEdxEq = -99.9;
    bool isIBL = false;
    bool iblOverflow = 0;
    bool passdEdxCutsLoose = false;
    bool passdEdxCutsTight = false;
  };

  void getClusterdEdx( PixelClusterStruct& cluster,
                        unsigned char& nIBLOverflowHits,
                        bool tightClusterCleaning = false);
  
  void getdEdxMetrics(const std::vector<PixelClusterStruct>& clusters,
                         float& averagedEdx, 
                         float& sigmadEdx, 
                         unsigned char& nUsedHits,
                         bool equalize = false);

} // namespace PixelDEdx

#endif  // PIXELDEDXUTILS_H
