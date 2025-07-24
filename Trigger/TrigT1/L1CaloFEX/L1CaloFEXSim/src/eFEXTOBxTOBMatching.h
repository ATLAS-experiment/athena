/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXTOBMatching -  description
//                           ------------------------------
//     Small utility function to identify xTOBs that correspond to TOBs
//     begin                : 12 08 2022
//     email                : Alan.Watson@CERN.CH
//  ***************************************************************************/

#include <set>

namespace LVL1 {


  template <typename T> void matchTOBs(T& TOBs, T& xTOBs) {

    // Use a std::set of TOB coordinate fields to match TOBs and xTOBs
    std::set<uint32_t> tobCoords;

    // Fill maps with coordinates of TOBs
    for (auto t : *TOBs) {
       uint32_t coord = t->word0()>>16;
       tobCoords.insert(coord);
    }

    // Set isTOB flag for xTOBs whose coordinate fields match a TOB
    for (auto t : *xTOBs) {
      uint32_t coord = t->word0()>>16;
      if (tobCoords.find(coord) != tobCoords.end()) t->setIsTOB(1);
    }
  
  }  

} // end of namespace bracket

