/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONMDTTOOLS_CANDIDATEINFO_H
#define L1MUONMDTTOOLS_CANDIDATEINFO_H

#include "AthenaKernel/CLASS_DEF.h"
#include "Identifier/Identifier.h"
#include "L1MuonMDTTools/IPtEstimationTool.h"
#include "L1MuonMDTTools/Segment.h"
#include "xAODMuonPrepData/MdtDriftCircleFwd.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include <array>
#include <cstddef>
#include <optional>
#include <vector>

namespace L1Muon::L1MDT {

constexpr std::size_t nStationSlots = Muon::MuonStationIndex::toInt(Muon::MuonStationIndex::LayerIndex::LayerIndexMax);

/**
 * @struct CandidateInfo
 * @brief Transient snapshot of what the MDT trigger simulation did with one sector logic candidate.
 *
 * It is filled by L1Muon::MDTSimulation when its CandidateInfoKey is set, and is meant for
 * validation and for writing test vectors for the L0MDT firmware. The pointers refer to objects
 * in StoreGate and are only valid within the event.
 */
struct CandidateInfo {
  /// Index of the seeding candidate in its input container (L1RPCCandData)
  std::size_t seedIndex{0};
  /// Decoded eta / phi [rad] of the seed
  float eta{0.f};
  float phi{0.f};
  /// Straight line R = m * z + b, fitted to the seed, used to select the MDT hits
  float m{0.f};
  float b{0.f};
  /// Hit position (z, R) of each RPC layer used in the fit (BI, BM doublet 1, BM doublet 2, BO); 0 if absent
  std::array<float, 4> rpcZ{};
  std::array<float, 4> rpcR{};
  /// Chambers (module identifiers) returned by the region selector for the seed
  std::vector<Identifier> regSelChambers{};
  /// Chambers (module identifiers) of each station slot used for the hit extraction: the chamber
  /// crossed by the candidate line first, then its neighbours along eta; empty if none
  std::array<std::vector<Identifier>, nStationSlots> stationChambers{};
  /// Hit extraction window: tubes on each side of the candidate line, in every tube layer
  int heHalfWidth{0};
  /// True if the candidate has at least two stations with enough hits and went through the segment finders
  bool processed{false};
  /// Selected hits, CSF and Legendre segments per station slot
  std::array<std::vector<const xAOD::MdtDriftCircle*>, nStationSlots> hits{};
  std::array<std::vector<Segment>, nStationSlots> csfSegments{};
  std::array<std::vector<Segment>, nStationSlots> legendreSegments{};
  /// pT estimate from the CSF segments, if available
  std::optional<PtEstimate> ptEstimate{};
};

using CandidateInfoContainer = std::vector<CandidateInfo>;

}  // namespace L1Muon::L1MDT

CLASS_DEF(L1Muon::L1MDT::CandidateInfoContainer, 1104677403, 1)

#endif
