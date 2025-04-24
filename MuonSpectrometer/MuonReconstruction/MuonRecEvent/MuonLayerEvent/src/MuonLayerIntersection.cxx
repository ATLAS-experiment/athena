/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonLayerEvent/MuonLayerIntersection.h"

namespace Muon {
    MuonLayerIntersection::MuonLayerIntersection(const MuonSystemExtension::Intersection& intersection_,
                                                 const std::shared_ptr<const MuonSegment>& segment_,
                                                 int quality_) :
        intersection{intersection_}, segment{segment_}, quality{quality_} {}

}  // namespace Muon
