/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODL0MuonCand/RPCCandData.h"

#include "xAODMuonPrepData/versions/AccessorMacros.h"
#include "CxxUtils/trapping_fp.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace {
   static const std::string preFixStr{"L0Mu_"};
}

namespace xAOD {
 
  void RPCCandData_v1::setZPos(std::array<float, 4>& zPos) {
    std::array<uint16_t, 4> zPosBins;
    for (size_t i = 0; i < zPos.size(); ++i) {
      // Avoid FPE with clang.
      CXXUTILS_TRAPPING_FP;
        uint16_t zPosBin = static_cast<uint16_t>(std::round((zPos[i] + s_zPosRange) / (2.0f * s_zPosRange) * static_cast<float>(s_zPosBitRange)));
        zPosBins[i] = zPosBin;
    }
    static const SG::Accessor<std::array<uint16_t, 4>> acc(preFixStr + "zPos");
    acc(*this) = zPosBins;
  }
  
  std::array<uint16_t, 4> RPCCandData_v1::zPos() const {
    static const SG::ConstAccessor<std::array<uint16_t, 4>> acc(preFixStr + "zPos");
    return acc(*this);
  }

} // namespace xAOD



