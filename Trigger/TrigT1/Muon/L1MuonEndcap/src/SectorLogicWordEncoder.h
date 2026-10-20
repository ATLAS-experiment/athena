/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1MUONENDCAP_SECTORLOGICWORDENCODER_H
#define L1MUONENDCAP_SECTORLOGICWORDENCODER_H

#include <cstdint>

#include "xAODTrigL1Muon/L1TGCCandData.h"

namespace L1Muon::L1TGC {

/** @brief MuCTPI candidate words produced from one TGC candidate. */
struct SectorLogicWords {
  std::uint32_t candWord{};
  std::uint32_t candExtraWord{};
};

/** @brief Encode the TGC-owned fields of the Run-4 Sector Logic format. */
class SectorLogicWordEncoder final {
 public:
  /** @brief Encode one TGC candidate without MDT or overlap information. */
  static SectorLogicWords encode(const xAOD::L1TGCCandData& candidate);
};

}  // namespace L1Muon::L1TGC

#endif  // L1MUONENDCAP_SECTORLOGICWORDENCODER_H
