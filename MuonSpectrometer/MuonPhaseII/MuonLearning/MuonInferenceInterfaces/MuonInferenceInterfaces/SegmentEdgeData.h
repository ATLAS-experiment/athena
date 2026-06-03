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

  struct SegmentEdgeGraph {
    std::vector<const xAOD::MuonSegment_v1*> segments{};
    std::vector<float> nodeFeatures{};      //!< packed [N,10]: pos_m(3), dir_u(3), bucket(4)
    std::vector<int64_t> edgeIndex{};       //!< packed edge pairs [src0,dst0,src1,dst1,...]
    std::vector<float> edgeFeatures{};      //!< packed [E,7]: dpos(3), dist, cos, same_chamber, same_sector
    std::size_t nNodes{0};
    std::size_t nEdges{0};
  };

  struct SegmentEdgeScore {
    std::size_t src{0};
    std::size_t dst{0};
    float logit{0.f};
    float probability{0.f};
  };

}
#endif
