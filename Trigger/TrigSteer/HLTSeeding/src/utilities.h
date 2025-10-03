/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef HLTSeeding_utilities_h
#define HLTSeeding_utilities_h
#include <cstdint>

namespace HLTSeedingNs{
  // User-defined literal for uint64_t
  constexpr uint64_t operator"" _u64(unsigned long long v) {
    return static_cast<uint64_t>(v);
  }

}

#endif
