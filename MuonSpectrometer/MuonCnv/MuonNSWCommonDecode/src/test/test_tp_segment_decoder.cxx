/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <sstream>
#include <iostream>
#include "MuonNSWCommonDecode/NSWSTGTPDecodeBitmaps.h"

#define TEST_BACKFORTH(in_word, place) \
   do { \
     uint32_t out_word{0}; \
     encodeSegmentProperty(place,in_word, out_word);\
     uint32_t test_word = getSegmentProperty(out_word, place); \
     if (test_word != in_word) { \
        std::cerr<<"Failed to validate "<<#place<<". Started with "<<in_word<<" ended up with "<<test_word<<std::endl; \
        return EXIT_FAILURE; \
     }  \
     std::cout<<"Back-forth conversion at "<<#place << " was successful "<<in_word<<std::endl; \
   } while (false)

int main (int , char **) {
    using namespace Muon::nsw::STGTPSegments;

    
    const uint32_t monitor_word {1};  
    const uint32_t spare_word {2};
    const uint32_t lowres_word {1};
    const uint32_t phires_word{1};
    const uint32_t dTheta_word{7};
    const uint32_t phiID_word{23};
    const uint32_t rIndex_word{52};
    /// Test first the backforth conversion
    try {
      TEST_BACKFORTH(monitor_word, MergedSegmentProperty::Monitor);
      TEST_BACKFORTH(spare_word, MergedSegmentProperty::Spare);
      TEST_BACKFORTH(lowres_word, MergedSegmentProperty::lowRes);
      TEST_BACKFORTH(phires_word, MergedSegmentProperty::phiRes);
      TEST_BACKFORTH(dTheta_word, MergedSegmentProperty::dTheta);
      TEST_BACKFORTH(phiID_word, MergedSegmentProperty::phiID);
      TEST_BACKFORTH(rIndex_word, MergedSegmentProperty::rIndex);
    }
    catch (const std::exception& e) {
      std::cerr << e.what() << "\n";
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

