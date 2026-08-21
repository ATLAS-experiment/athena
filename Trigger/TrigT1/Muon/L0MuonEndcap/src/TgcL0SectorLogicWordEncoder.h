/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L0MUONENDCAP_TGCL0SECTORLOGICWORDENCODER_H
#define L0MUONENDCAP_TGCL0SECTORLOGICWORDENCODER_H

#include <cstdint>

#include "xAODL0MuonCand/TGCCandData.h"

namespace L0Muon {

/** @brief MuCTPI candidate words produced from one TGC candidate. */
struct TgcL0SectorLogicWords {
  std::uint32_t candWord{};
  std::uint32_t candExtraWord{};
};

/** @brief Encode the TGC-owned fields of the Run-4 Sector Logic format. */
class TgcL0SectorLogicWordEncoder final {
 public:
  /** @brief Encode one TGC candidate without MDT or overlap information. */
  static TgcL0SectorLogicWords encode(const xAOD::TGCCandData& candidate);
};

}  // namespace L0Muon

#endif  // L0MUONENDCAP_TGCL0SECTORLOGICWORDENCODER_H
