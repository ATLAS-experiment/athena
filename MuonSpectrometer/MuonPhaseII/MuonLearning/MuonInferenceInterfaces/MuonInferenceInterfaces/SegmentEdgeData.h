/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCEINTERFACES_SEGMENTEDGEDATA_H
#define MUONINFERENCEINTERFACES_SEGMENTEDGEDATA_H

#include "xAODMuon/MuonSegment.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace MuonML {

  /// Gate that stopped (or did not stop) a pair of input segments from reaching
  /// ONNX. Diagnostics only; listed in the order the tool applies them.
  enum class PairGate : std::uint8_t {
    Scored = 0,     //!< the pair was sent to ONNX
    BucketCap,      //!< an endpoint was dropped by MaxSegmentsPerBucket
    SectorWindow,   //!< |delta sector| > MaxDeltaSector
    SameChamber,    //!< same-chamber pair dropped by design
    AngleWindow,    //!< cos(opening angle) below MaxDeltaThetaDeg
    EdgeCaps        //!< candidate evicted by the pre-ONNX per-node / per-target-chamber caps
  };

  /// One pair of input segments that share a truth particle (input-container indices).
  struct PairFate {
    std::uint32_t first{0};
    std::uint32_t second{0};
    PairGate gate{PairGate::Scored};
    std::uint8_t sectorDelta{0};
  };

  /// Values of SegmentEdgeGraph::inputNodeIndex for input segments without a node.
  inline constexpr std::int32_t kDroppedByBucketCap = -1;
  inline constexpr std::int32_t kDroppedAsIsolated = -2;

  struct SegmentEdgeGraph {
    std::vector<const xAOD::MuonSegment_v1*> segments{};
    std::vector<float> nodeFeatures{};      //!< packed [N,10]: pos_m(3), dir_u(3), bucket(4)
    std::vector<int64_t> edgeIndex{};       //!< packed edge pairs [src0,dst0,src1,dst1,...]
    std::vector<float> edgeFeatures{};      //!< packed [E,7]: dpos(3), dist, cos, same_chamber, same_sector
    std::size_t nNodes{0};
    std::size_t nEdges{0};

    // Truth diagnostics: filled by the tool only when its EnableTruthDiagnostics
    // is set and DEBUG output is on; empty (never allocated) otherwise.
    std::vector<std::int32_t> inputNodeIndex{};  //!< per input segment: node index, or kDropped*
    std::vector<PairFate> truthPairFates{};      //!< every pair of input segments sharing a truth particle
  };

  struct SegmentEdgeScore {
    std::size_t src{0};
    std::size_t dst{0};
    float logit{0.f};
    float probability{0.f};
  };

}
#endif
