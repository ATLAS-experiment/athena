/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TSU_HELPERS_H
#define TSU_HELPERS_H

#include "L1TopoEvent/TOBArray.h"

namespace TSU {
  bool isAmbiguousAt(TCS::TOBArray const* tobs, size_t pos, unsigned minEt = 0); 
  bool isAmbiguousTruncation(TCS::TOBArray const* tobs, size_t pos, unsigned minEt = 0); 
  bool isAmbiguousAnywhere(TCS::TOBArray const* tobs, size_t pos, unsigned minEt = 0); 
}
#endif
