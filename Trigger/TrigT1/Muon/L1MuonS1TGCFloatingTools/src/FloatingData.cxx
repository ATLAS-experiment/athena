/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FloatingData.h"

namespace L1Muon::L1TGC {

FloatingHitGroupKey::Tuple FloatingHitGroupKey::tie() const {
  return std::make_tuple(subDetectorId, triggerSector, bcTag);
}

bool FloatingHitGroupKey::operator<(const FloatingHitGroupKey& other) const {
  return tie() < other.tie();
}

}  // namespace L1Muon::L1TGC
