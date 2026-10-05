/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTrigL1Muon/L1RPCCandData.h"

#include "CxxUtils/trapping_fp.h"
#include <cmath>
#include <algorithm>

namespace xAOD {

  void L1RPCCandData_v1::setL1ZPos(std::array<float, 4>& zPos) {
    std::array<uint16_t, 4> zPosBins;
    for (size_t i = 0; i < zPos.size(); ++i) {
      // Avoid FPE with clang.
      CXXUTILS_TRAPPING_FP;
        uint16_t zPosBin = static_cast<uint16_t>(std::round((zPos[i] + s_zPosRange) / (2.0f * s_zPosRange) * static_cast<float>(s_zPosBitRange)));
        zPosBins[i] = zPosBin;
    }
    static const SG::Accessor<std::array<uint16_t, 4>> acc("l1ZPos");
    acc(*this) = zPosBins;
  }

  std::array<uint16_t, 4> L1RPCCandData_v1::l1ZPos() const {
    static const SG::ConstAccessor<std::array<uint16_t, 4>> acc("l1ZPos");
    return acc(*this);
  }

} // namespace xAOD
