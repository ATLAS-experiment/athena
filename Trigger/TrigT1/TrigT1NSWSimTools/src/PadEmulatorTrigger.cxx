/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PadEmulatorTrigger.h"

namespace NSWL1 {
  PadEmulatorTrigger::PadEmulatorTrigger(const char wheel, const uint32_t sector,
                                         const uint32_t bandid, const uint32_t phiid, const uint32_t relbcid,
                                         const PadPattern pattern, const uint32_t hitmask):
    m_wheel{wheel},
    m_sector{sector},
    m_pattern{pattern},
    m_hitmask{hitmask},
    m_bandid{bandid},
    m_phiid{phiid},
    m_phiid_signed{NSWL1::getSignedPhiID(m_phiid)},
    m_relbcid{relbcid}
  {
    std::string hitstr = std::bitset<32>(hitmask).to_string();
    std::reverse(hitstr.end()-9,hitstr.end());
    m_hitmasksignature = hitstr.substr(23,8);
  }
}
