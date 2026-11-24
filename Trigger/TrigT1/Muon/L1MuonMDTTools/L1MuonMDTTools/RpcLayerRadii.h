/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONMDTTOOLS_RPCLAYERRADII_H
#define L1MUONMDTTOOLS_RPCLAYERRADII_H

#include <array>

namespace L1Muon::L1MDT {

  /// Fixed radii [mm] of the barrel RPC trigger layers, measured along the sector axis, at which the
  /// sector logic gives the candidate z positions. Layer order as L1RPCCandData::l1ZPos:
  /// BI, BM doublet 1, BM doublet 2, BO.
  inline constexpr std::array<float, 4> RPC_LARGE_R{5290.0f, 6850.0f, 7525.0f, 9890.0f};
  inline constexpr std::array<float, 4> RPC_SMALL_R{4490.0f, 7860.0f, 8400.0f, 10330.0f};

  /// Large sectors are the odd ones
  inline bool isLargeSector(int sector) { return sector % 2 == 1; }

  /// Fixed radius of an RPC trigger layer (0-3) in a sector (1-16)
  inline float rpcLayerRadius(int sector, unsigned layer) {
    return isLargeSector(sector) ? RPC_LARGE_R.at(layer) : RPC_SMALL_R.at(layer);
  }

}  // namespace L1Muon::L1MDT

#endif
