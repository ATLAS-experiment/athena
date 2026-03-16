/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AccMap.h"
#include <iostream>
#include <format>
#ifndef LARG4_STAND_ALONE
#include "PathResolver/PathResolver.h"
#endif

AccMap::AccMap() {
    // Fold ranges for the 10 electronic regions
    static constexpr std::array<int, 10> i1 = {0, 0, 3, 2, 9, 12, 10, 9, 0, 2};
    static constexpr std::array<int, 10> i2 = {2, 1, 12, 12, 13, 13, 13, 13, 1, 4};
    static constexpr double xnorm = 14.1591;

#ifndef LARG4_STAND_ALONE
    const std::string larLocation = PathResolver::find_directory("LArG4Barrel", "ATLASCALDATA");
#endif

    for (int iregion = 0; iregion < MAX_REGIONS; ++iregion) {
        
        // 1. Process Accordion Folds
        for (int ifold = i1[iregion]; ifold <= i2[iregion]; ++ifold) {
            // Using std::format for cleaner filename generation
            std::string filename = std::format("fold{}_region{}.map", ifold, iregion);
            
            std::string fileLocation = 
#ifdef LARG4_STAND_ALONE
                std::format("{}/{}", m_directory, filename);
#else
                std::format("{}/{}", larLocation, filename);
#endif

            auto cm = std::make_unique<CurrMap>(fileLocation, xnorm);
            
            // Add rounding safety for primary folds
            if (ifold < N_MAX_VEC) {
                m_xmin[ifold] = cm->GetXmin() + 0.1f;
                m_xmax[ifold] = cm->GetXmax() - 0.1f;
                m_ymin[ifold] = cm->GetYmin() + 0.1f;
                m_ymax[ifold] = cm->GetYmax() - 0.1f;
            }
            m_fastMap[ifold][iregion] = std::move(cm);
        }

        // 2. Process Straight Sections
        for (int istr = 1; istr <= 2; ++istr) {
            int ifold = 20 + istr; // Mapping istr 1,2 to index 21,22
            
            std::string filename = std::format("straight{}_region{}.map", istr, iregion);
            
            std::string fileLocation = 
#ifdef LARG4_STAND_ALONE
                std::format("{}/{}", m_directory, filename);
#else
                std::format("{}/{}", larLocation, filename);
#endif
            m_fastMap[ifold][iregion] = std::make_unique<CurrMap>(fileLocation, xnorm);
        }
    }
}

const AccMap* AccMap::GetAccMap()
{
  static const AccMap instance;
  return &instance;
}

const CurrMap* AccMap::GetMap(int ifold, int region, int sampling, int eta) const noexcept
{
  return this->GetMap(ifold,this->Region(region,sampling,eta));
}

const CurrMap* AccMap::GetMap(int ifold, int ielecregion) const noexcept {
    // Direct O(1) lookup with bounds safety
    if (ifold >= 0 && ifold < MAX_FOLDS && ielecregion >= 0 && ielecregion < MAX_REGIONS) {
        return m_fastMap[ifold][ielecregion].get();
    }
    std::cout << "Fold " << ifold << " Region " << ielecregion << " out of bounds." << std::endl;
    return nullptr;
}

int AccMap::Region(int region, int sampling, int eta) const noexcept
{
  int elecregion=0;
  // logic to compute region vs eta and sampling...
  if (region==0) {
    if (sampling==1) {
      if (eta<256) elecregion=0;
      else         elecregion=1;
    }
    else if (sampling==2) {
      if (eta<32) elecregion=2;
      else        elecregion=3;
    }
    else {
      if (eta<9 || eta==26) elecregion=4;
      if ((eta>8 && eta<13) || (eta>15 && eta<19)) elecregion=5;
      if ((eta>12 && eta < 16) || (eta>18 && eta<21)) elecregion=6;
      if ((eta>20 && eta < 26)) elecregion=7;
    }
  }
  else {
    if (sampling==1) elecregion=8;
    else             elecregion=9;
  }
  return elecregion;
}
