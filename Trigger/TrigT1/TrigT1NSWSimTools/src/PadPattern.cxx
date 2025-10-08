/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PadPattern.h"

namespace NSWL1 {
  PadPattern::PadPattern(const uint32_t bandid, const uint32_t phiid, const std::array<uint32_t,8>& pfebs, const std::array<uint32_t,8>& padchans, const bool isLarge):
    m_bandid{bandid},
    m_phiid{phiid},
    m_phiid_flip{flipPhiIdSign()},
    m_pfebs{pfebs},
    m_padchans{padchans},
    m_isLarge{isLarge}
  {
  }

  uint32_t PadPattern::flipPhiIdSign() const {
    constexpr uint32_t sign_mask{0b100000};
    return sign_mask xor m_phiid;
  }
}
