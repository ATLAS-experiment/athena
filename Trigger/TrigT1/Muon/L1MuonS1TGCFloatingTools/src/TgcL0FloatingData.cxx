/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0FloatingData.h"

namespace L1Muon {
namespace TgcL0Floating {

HitGroupKey::Tuple HitGroupKey::tie() const {
  return std::make_tuple(subDetectorId, triggerSector, bcTag);
}

bool HitGroupKey::operator<(const HitGroupKey& other) const {
  return tie() < other.tie();
}

}  // namespace TgcL0Floating
}  // namespace L1Muon
